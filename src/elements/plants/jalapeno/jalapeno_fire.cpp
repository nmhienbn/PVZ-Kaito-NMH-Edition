#include "jalapeno_fire.hpp"

#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/zombies/zombie_struct.hpp" // rand_float
#include "game_param.hpp"
#include "reanim/reanim.hpp"
#include "reanim/reanim_sdl.hpp"

#include <cmath>
#include <vector>

extern Window win;
extern int game_state;
extern Map cells;

namespace jalapeno_fx {
namespace {

// repvz DoFwoosh: twelve stations at x = 10 + 750*i/11 on the original
// 800-wide board; this board runs the same layout shifted +220 px, and the
// flame origin rides the row's ground line 10 px up (GetPosYBasedOnRow
// (aPosX + 10, row) - 10), following the roof slope per column.
const int FIRE_STATIONS = 12;
const int BOARD_OFFSET_X = 220;
// repvz mFwooshCountDown = 100 cs (1 s here): stations hold "anim_flame" for
// the first 50 cs; UpdateFwoosh's linear 12 -> 0 ramp then kills one station
// every ~4.2 cs, each finishing with the 10-frame "anim_done" take at 15 fps.
const int FIRE_HOLD_TICKS = 30; // 50 cs
const int FIRE_RAMP_TICKS = 30; // 50 cs over 12 stations
const int FIRE_DONE_TICKS = 40; // 10 frames @ 15 fps (60 tps)

struct LaneFire
{
    int row;
    int station;  // 0..11 left to right = the repvz mFwooshID slot order
    int x, y;     // reanim origin of the station (like AddReanimation's pos)
    int tick = 0; // ticks since spawn
    float rate;   // anim_flame advance per tick, Rand(0.7, 1.3) x doc rate
    float scale;  // Rand(0.9, 1.1) in the original (flip skipped: the SDL
                  // reanim renderer has no mirror option)
};

reanim::SdlReanimRenderer *g_renderer = nullptr;
reanim::ReanimDoc g_doc;
int g_first_flame = 0, g_last_flame = -1;
int g_first_done = 3, g_last_done = 12;
bool g_tried = false, g_ok = false;
std::vector<LaneFire> g_fires;

bool fx_ready()
{
    if (g_tried)
        return g_ok;
    g_tried = true;
    static reanim::SdlReanimRenderer renderer(win.sprite_renderer());
    g_renderer = &renderer;
    std::string err;
    g_ok = renderer.BindDirectory("./resources/reanim") &&
           reanim::LoadReanimXml("./resources/reanim/fire.reanim", g_doc, err);
    if (!g_ok)
    {
        SDL_Log("jalapeno_fx: %s", err.c_str());
        return false;
    }
    auto fl = g_doc.segments.find("anim_flame"), dn = g_doc.segments.find("anim_done");
    if (fl == g_doc.segments.end() || dn == g_doc.segments.end())
    {
        g_ok = false;
        SDL_Log("jalapeno_fx: fire.reanim lacks anim_flame/anim_done");
        return false;
    }
    g_first_flame = fl->second.first;
    g_last_flame = fl->second.last;
    g_first_done = dn->second.first;
    g_last_done = dn->second.last;
    return true;
}

// Tick the station's burn schedule: UpdateFwoosh starts dropping stations
// once 50 cs remain; station i (left to right) switches to "anim_done" at
// tick HOLD + RAMP * i / 12 and is erased FIRE_DONE_TICKS later.
int done_tick_of(int station)
{
    return FIRE_HOLD_TICKS + (station * FIRE_RAMP_TICKS + FIRE_STATIONS - 1) /
                                 FIRE_STATIONS;
}

} // namespace

void spawn_lane_fire(int row)
{
    if (!fx_ready())
        return;
    for (int i = 0; i < FIRE_STATIONS; i++)
    {
        LaneFire f;
        f.row = row;
        f.station = i;
        f.x = BOARD_OFFSET_X + (int)(750.0f * i / (FIRE_STATIONS - 1.0f)) + 10;
        int col = (f.x + 10 - X_UPPER_LEFT) / BLOCK_WIDTH;
        if (col < 0)
            col = 0;
        if (col > HORIZ_BLOCK_COUNT - 1)
            col = HORIZ_BLOCK_COUNT - 1;
        f.y = cells[row][col].y2 + roof_tile_dy(col) - 10; // orig's -10 lift
        f.rate = 0.2f * rand_float(0.7f, 1.3f); // doc 12 fps at 60 tps
        f.scale = rand_float(0.9f, 1.1f);
        g_fires.push_back(f);
    }
}

void display_row(int row)
{
    if (g_fires.empty() || !fx_ready())
        return;
    const bool paused = check_status(game_state, IS_PAUSED);
    for (int i = 0; i < (int)g_fires.size();)
    {
        LaneFire &f = g_fires[i];
        if (f.row != row)
        {
            i++;
            continue;
        }
        const int done_tick = done_tick_of(f.station);
        // Auto-FPS: the burn LIFETIME (expiry/erase below) must track LOGIC
        // ticks, not the uncapped render rate -- the fire would vanish
        // several times too fast at high monitor rates otherwise.
        for (int _lt = logic_ticked; _lt > 0 && !paused; _lt--)
            f.tick++;
        // Timeline: [0, done_tick) loops anim_flame at the jittered rate, then
        // anim_done plays once at 15 fps (0.25 frames/tick).
        float absFrame;
        bool expired = false;
        if (f.tick < done_tick)
        {
            float span = (float)(g_last_flame - g_first_flame + 1);
            absFrame = g_first_flame + fmodf(f.tick * f.rate, span);
        }
        else
        {
            absFrame =
                g_first_done + (float)(f.tick - done_tick) * (15.0f / 60.0f);
            expired = f.tick >= done_tick + FIRE_DONE_TICKS;
        }
        if (!expired)
            g_renderer->Draw(g_doc, absFrame, (float)f.x, (float)f.y,
                             f.scale * WORLD_SCALE, nullptr);
        if (expired)
            g_fires.erase(g_fires.begin() + i);
        else
            i++;
    }
}

void clear()
{
    g_fires.clear();
}

int active_count(int row)
{
    int n = 0;
    for (const auto &f : g_fires)
        n += f.row == row;
    return n;
}

} // namespace jalapeno_fx
