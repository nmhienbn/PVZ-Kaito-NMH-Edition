#include "one_shot_fx.hpp"

#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "game_param.hpp"
#include "reanim/reanim.hpp"
#include "reanim/reanim_sdl.hpp"
#include "reanim/reanim_zombie.hpp"

#include "elements/Level/Level.hpp"  // level, roof_tile_dy / roof_slope_offset
#include "elements/Map/Map.hpp"      // cells
#include "elements/zombies/zombie_struct.hpp"

#include <cmath>
#include <string>
#include <vector>

extern Window win;
extern int game_state;
extern Map cells;
extern Level level;

namespace reanimfx {
namespace {

struct FxDef
{
    const char *file;
    float fps;   // doc frames per second
    float scale; // pvz1 doc px -> screen px
};

// splash.reanim: 2Pak XFL conversion (21 droplet tracks). fps from the doc
// header is 12. The grave docs are hand-authored ports of the original
// compiled particle defs (2Pak compiled/particles/*.xml.compiled):
// ZombieRise (arm + dirt splash + big clumps), WhackAZombieRise (dirt splash
// + big clumps, no arm), GraveStoneRise (dirt splash), GraveBuster /
// GraveBusterDie (pebble spray, Gravebuster_dirt burst -- the retail
// GraveBusterDie emitter list is just DoomBase/GRAVEBUSTER_DIRT, no dust).
// The newer impact docs play the 2Pak particle strips as one frame per
// keyframe (the melon burst, the planting dirt ring) or ride a hand-authored
// grow/fade over the single butter splat / dust puff frames.
const FxDef FX_DEFS[] = {
    /* FX_SPLASH */ {"splash.reanim", 12.0f, 1.0f},
    /* FX_ZOMBIE_GRAVE_RISE */ {"zombie_grave_rise.reanim", 12.0f, 1.0f},
    /* FX_WHACK_ZOMBIE_RISE */ {"whack_zombie_rise.reanim", 12.0f, 1.0f},
    /* FX_GRAVE_SPROUT */ {"grave_sprout.reanim", 12.0f, 1.0f},
    /* FX_GRAVEBUSTER_CHOMP */ {"grave_buster_chomp.reanim", 12.0f, 1.0f},
    /* FX_GRAVEBUSTER_DIE */ {"grave_buster_die.reanim", 12.0f, 1.0f},
    /* FX_BUTTER_SPLAT */ {"butter_splat.reanim", 27.0f, 1.0f},
    /* FX_MELON_IMPACT */ {"melon_impact.reanim", 27.0f, 1.0f},
    /* FX_PLANT_DIRT */ {"plant_dirt.reanim", 36.0f, 1.0f},
    /* FX_DUST_PUFF */ {"dust_puffs.reanim", 15.0f, 1.0f},
};

// Dust puff cadence, game ticks at 60 tps (~12 while a mower rolls, ~15
// along the rolling sod strip's front edge).
const int MOWER_DUST_EVERY = 12;
const int SOD_DUST_EVERY = 15;

struct Fx
{
    FxId id;
    float t = 0.0f; // doc-local frame
    int x, y, row;
};

reanim::SdlReanimRenderer *g_renderer = nullptr;
reanim::ReanimDoc g_docs[sizeof(FX_DEFS) / sizeof(FX_DEFS[0])];
bool g_ok[sizeof(FX_DEFS) / sizeof(FX_DEFS[0])] = {};
bool g_tried = false;
std::vector<Fx> g_live;

bool fx_ready()
{
    if (g_tried)
        return true;
    g_tried = true;
    static reanim::SdlReanimRenderer renderer(win.sprite_renderer());
    g_renderer = &renderer;
    if (!renderer.BindDirectory("./resources/reanim"))
        return false;
    for (int i = 0; i < (int)(sizeof(FX_DEFS) / sizeof(FX_DEFS[0])); i++)
    {
        std::string err;
        g_ok[i] = reanim::LoadReanimXml(std::string("./resources/reanim/") +
                                            FX_DEFS[i].file,
                                        g_docs[i], err);
        if (!g_ok[i])
            SDL_Log("reanimfx: %s", err.c_str());
    }
    return true;
}

} // namespace

void spawn(FxId id, int x, int y, int row)
{
    // Skip when the doc is missing so nothing ghosts in the list forever
    // (the life-cycle below only ages effects that actually render).
    if (fx_ready() && g_ok[id])
        g_live.push_back({id, 0.0f, x, y, row});
}

void spawn_butter_splat(const Zombie *z)
{
    // Fold the layout offsets / roof slope in exactly like the pat draw in
    // Zombie::display does (zombie_struct.cpp).
    const int bx = z->x_location + z->layout_dx;
    const int by = z->y_location + z->layout_dy + roof_slope_offset(bx);
    if (!reanimz::ready())
        reanimz::init(win.sprite_renderer());
    if (reanimz::ready())
    {
        float head_cx, head_top, art_mul;
        if (reanimz::headAnchor(z->cur_dir(), -1, z->reanim_time,
                                z->cold_time > 0, bx, by,
                                head_cx, head_top, art_mul))
        {
            // DrawButter centers the pat on (head_cx - 2*mul, head_top).
            spawn(FX_BUTTER_SPLAT, (int)lroundf(head_cx - 2.0f * art_mul),
                  (int)lroundf(head_top), z->row);
            return;
        }
    }
    // Sheet fallback: the legacy hand-tuned crown spot (same point the pat
    // draw uses, rect center).
    spawn(FX_BUTTER_SPLAT, bx - 4, by + 19, z->row);
}

void mower_dust(int x, int y, int row)
{
    static int clk = 0; // shared cadence: one cleaner rolls at a time in practice
    if (++clk >= MOWER_DUST_EVERY)
    {
        clk = 0;
        spawn(FX_DUST_PUFF, x, y, row);
    }
}

void spawn_planted_dirt(int row, int col)
{
    if (row < 0 || row >= (int)cells.size() || col < 0 || col >= HORIZ_BLOCK_COUNT)
        return;
    const auto &t = cells[row][col];
    if (t.is_water)
        // pool tile: the squash-into-water splash anchor (squash.cpp)
        spawn(FX_SPLASH, (t.x1 + t.x2) / 2, t.y1 + roof_tile_dy(col) + 55, row);
    else
        spawn(FX_PLANT_DIRT, (t.x1 + t.x2) / 2, t.y2 + roof_tile_dy(col) + 4, row);
}

void sod_dust(int front_x)
{
    if (level.playable_rows.empty())
        return;
    static int clk = 0; // per-intro-frame cadence (either intro loop feeds 1/frame)
    if (++clk >= SOD_DUST_EVERY)
    {
        clk = 0;
        const int r = level.playable_rows[level.playable_rows.size() / 2];
        spawn(FX_DUST_PUFF, front_x, cells[r][0].y2 - 4, r);
    }
    for (int row : level.playable_rows)
        display(row);
}

void display(int row)
{
    if (g_live.empty() || !fx_ready())
        return;
    for (int i = 0; i < (int)g_live.size();)
    {
        Fx &f = g_live[i];
        const reanim::ReanimDoc &doc = g_docs[f.id];
        if (f.row == row && g_ok[f.id])
        {
            g_renderer->Draw(doc, f.t, (float)f.x, (float)f.y,
                             FX_DEFS[f.id].scale * WORLD_SCALE, nullptr);
            // Doc frames per tick: authored fps / FPS = 0.2 for the 12-fps
            // docs at 60 tps (same feed as the plant/zombie reanim clocks).
            // The old /100 rate treated the game as 100 tps and ran every
            // effect at ~1/5 speed.
            // anim advances once per logic tick, not per render (auto-FPS)
            if (!check_status(game_state, IS_PAUSED))
                f.t += FX_DEFS[f.id].fps / (float)FPS * logic_ticked;
        }
        if (f.t >= (float)(doc.frameCount + 2)) // past the last f=-1 rest frame
            g_live.erase(g_live.begin() + i);
        else
            i++;
    }
}

void clear()
{
    g_live.clear();
}

int active_count()
{
    return (int)g_live.size();
}

int active_count_of(FxId id)
{
    int n = 0;
    for (const auto &f : g_live)
        n += f.id == id;
    return n;
}

} // namespace reanimfx
