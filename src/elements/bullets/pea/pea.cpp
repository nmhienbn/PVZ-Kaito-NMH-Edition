#include "pea.hpp"
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/elements.hpp"
#include "elements/zombies/zombie_struct.hpp" // rand_float
#include "game_param.hpp"
#include "music/music.hpp"
#include "reanim/reanim.hpp"
#include "reanim/reanim_sdl.hpp"
#include <algorithm>
#include <cmath>

extern Map cells;
extern Window win;
extern Elements game_characters;
extern Level level;

// Straight-shot speed comes from PEA_DX (HEAD pacing, a61577b: 10 px/tick;
// DPS unchanged -- fire rate and damage untouched, only flight pacing).

// Fire pea art (repvz Projectile::Draw: the fireball has NO sprite image --
// Projectile.cpp:987-990 -- every pixel comes from the REANIM_FIRE_PEA
// reanimation attached at mPos - (-25, -25), looping at a per-shot rate of
// RandRangeFloat(50, 80) fps; ConvertToFireball, Projectile.cpp:1206-1227).
// FirePea.reanim anchors its orbit over the retail 28x28 pea box; the clone's
// box is 26x25, so the art plays at 26/28 of the doc pixels: the 38-px fire
// ball renders 35 px against the 26-px pea.
const float FIRE_PEA_SCALE = 26.0f / 28.0f;
const int FIRE_PEA_ORG_DX = -25, FIRE_PEA_ORG_DY = -25; // reanim origin offset

namespace {
reanim::SdlReanimRenderer *g_firepea_renderer = nullptr;
reanim::ReanimDoc g_firepea_doc;
bool g_firepea_tried = false, g_firepea_ok = false;

bool fire_pea_ready()
{
    if (g_firepea_tried)
        return g_firepea_ok;
    g_firepea_tried = true;
    static reanim::SdlReanimRenderer renderer(win.sprite_renderer());
    g_firepea_renderer = &renderer;
    std::string err;
    g_firepea_ok = renderer.BindDirectory("./resources/reanim") &&
                   reanim::LoadReanimXml("./resources/reanim/FirePea.reanim",
                                         g_firepea_doc, err);
    if (!g_firepea_ok)
        SDL_Log("pea: %s", err.c_str());
    return g_firepea_ok;
}
} // namespace

/*Construct new pea; dir=-1 flies left (Split Pea's rear head).*/
Pea::Pea(int _row, int _x, int _dir) : Bullet(PEA, _row, _x)
{
    width = 26;
    x_pos = (float)_x;
    height = 25;
    dir = _dir < 0 ? -1 : 1;
    angle = dir < 0 ? 180 : 0;
    directory_num = PEA_DIRECTORY;
    y_location = cells[row][0].y1 + 20 + roof_tile_dy(get_block_col(_x));
    // On a roof the shot keeps its height while the slope climbs under it:
    // peas out of the slanted columns dive into the shingles.
    bind_roof_slope();
}

/*
Torchwood: a pea flying right past a torchwood's column in its own lane
catches fire (double damage, drawn with the 2Pak FirePea sprite). Snow peas
melt through the same crossing check, wired in SnowzPea::move.
*/

/*Crossing test shared by Pea and SnowzPea: the shot is in the torchwood's
lane, flying right, and passed the trunk edge (cell x2 - 30) this tick.
(Original: Projectile::PeaAboutToHitTorchwood.)*/
bool crosses_torchwood(const int &row, const int &prev_x, const int &cur_x)
{
    for (const auto &plant : game_characters.plants)
    {
        if (plant->get_type() != TORCHWOOD_TYPE || plant->get_row() != row)
            continue;
        int edge = cells[row][plant->get_col()].x2 - 30; // ~the trunk's middle
        if (prev_x < edge && cur_x >= edge)
            return true;
    }
    return false;
}

void Pea::apply_torchwood(const int &prev_x)
{
    if (dir <= 0 || type != PEA)
        return;
    if (dmg == 1 && crosses_torchwood(row, prev_x, x_location))
    {
        dmg = 2; // fire pea
        directory_num = FIRE_PEA_DIRECTORY;
        // Roll this shot's orbit rate (repvz mAnimRate = RandRangeFloat
        // (50, 80) fps; advanced in move() at /60 per game tick).
        fire_rate = rand_float(50.0f, 80.0f);
        fire_t = 0.0f;
    }
}

/*
Return true if pea can move
    + If pea go out the playground: false
    + If pea reach any zombie: false
    + Else: true
*/
void Pea::move()
{
    if (has_exploded())
        return;
    // Fire peas age their reanim loop on the logic clock (paused game =
    // frozen flame, like every other reanim feed).
    if (directory_num == FIRE_PEA_DIRECTORY)
        fire_t += fire_rate / 60.0f;
    if (dir > 0)
    {
        if (x_location <= WINDOW_WIDTH)
        {
            int prev_x = x_location;
            x_pos += PEA_DX;
            x_location = (int)lroundf(x_pos);
            apply_torchwood(prev_x);
            if (splats_on_roof(28)) // pea/spike threshold (repvz Projectile.cpp:341)
                make_explode();
        }
    }
    else
    {
        x_pos -= PEA_DX;
        x_location = (int)lroundf(x_pos);
        if (x_location < X_UPPER_LEFT - width)
            make_explode(); // splat against the lawn's left edge
    }
}

/*
Display the peas
*/
void Pea::display()
{
    // Fire pea: the original draws no projectile sprite at all -- the looped
    // fire-pea reanimation (ball orbit + flame plume + sparks) IS the shot.
    if (directory_num == FIRE_PEA_DIRECTORY && fire_pea_ready())
    {
        float absFrame = fmodf(fire_t, (float)g_firepea_doc.frameCount);
        g_firepea_renderer->Draw(g_firepea_doc, absFrame,
                                 x_location + FIRE_PEA_ORG_DX,
                                 y_location + FIRE_PEA_ORG_DY,
                                 FIRE_PEA_SCALE * WORLD_SCALE, nullptr);
        return;
    }
    int more_px = 0;
    if (directory_num == PEA_EXPLODE_DIRECTORY)
        more_px += 25;
    else if (directory_num == FIRE_PEA_DIRECTORY)
        more_px += 55; // PNG fallback spans the retail FirePea union (~83px, §8.3)
    // World knob on the sprite only; width stays logical (collision).
    const int wtot = world_px(width + more_px);
    const int grow = wtot - width;
    win.draw_png_height_scaled(directory_num, x_location - grow / 2, y_location - grow / 2,
                               wtot, angle);
}

void Pea::crash_effect()
{
    play_sound_effect(PEA_CRASH_MUSIC_DIRECTORY);
}

bool Pea::fire_state_for_test(float &fire_frame, float &fire_fps) const
{
    fire_frame = fire_t;
    fire_fps = fire_rate;
    return directory_num == FIRE_PEA_DIRECTORY;
}

void Pea::make_explode()
{
    if (directory_num == PEA_DIRECTORY)
    {
        directory_num++; // Make the pea explode (PEA_DIRECTORY+1 is the pop art)
        explode = BULLET_EXPLODE_TIME;
        angle = rand(0, 360);
    }
    else if (directory_num == FIRE_PEA_DIRECTORY)
    {
        // A lit pea pops with the same splat art; the fire state must not
        // block the explode path or the hit keeps landing every tick.
        directory_num = PEA_EXPLODE_DIRECTORY;
        explode = BULLET_EXPLODE_TIME;
        angle = rand(0, 360);
    }
}
