// Game class hooks for the reanim zombie renderer. Kept separate from
// reanim_zombie.cpp so that file stays linkable without the whole game
// (used by demo/reanim_gameview).

#include "reanim_zombie.hpp"

#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/zombies/zombie_struct.hpp"
#include "game_param.hpp" // MAX_TIME_BLINK
#include <cmath>

extern int game_state;
extern Level level;
extern Map cells;
extern Window win;

// Big Trouble Little Zombie (3-5): every zombie renders at half size and
// plays its reanim at 2x -- exactly the original's anim rate
// (1/mScaleZombie with scale 0.5, repvz Zombie.cpp UpdateAnimSpeed). The
// per-level JSON "zombie_time_mul" overrides it; read into
// level.zombie_time_mul at level load. Anything outside big_trouble keeps
// the plain 1.0 factors.
static const float BIG_TROUBLE_SCALE_MUL = 0.5f;

static void apply_zombie_render_mode()
{
    const bool small = level.is_special(LevelSpecial::BigTrouble);
    reanimz::setZombieScaleMul((small ? BIG_TROUBLE_SCALE_MUL : 1.0f) * WORLD_SCALE);
    reanimz::setZombieTimeMul(small ? level.zombie_time_mul : 1.0f);
}

bool Zombie::render_reanim(const int &_minus_x)
{
    if (!reanimz::ready())
    {
        reanimz::init(win.sprite_renderer());
        if (!reanimz::ready())
            return false; // assets missing -> caller falls back to sprite sheets
    }
    apply_zombie_render_mode();
    // F2 layout editor (transient, render-only): the per-zombie debug scale
    // multiplies the mode factor on top (the renderer's bottom-center/ feet
    // anchor convention keeps the ground contact pinned); layout_dx/layout_dy
    // shift the drawn anchor. Defaults: 1.0 scale multiplier, zero offsets.
    const float dbg_scale = rsdl_debug_scale(this);
    if (dbg_scale != 1.0f)
        reanimz::setZombieScaleMul((level.is_special(LevelSpecial::BigTrouble)
                                        ? BIG_TROUBLE_SCALE_MUL
                                        : 1.0f) *
                                   dbg_scale * WORLD_SCALE);
    // Flash fades out over the blink budget (repvz mJustGotShotCounter
    // grayness ramp, Zombie.cpp:5744-5750; the clone's budget is the 10-tick
    // MAX_TIME_BLINK window).
    const float fade = attacked_time > 0 ? (float)attacked_time / MAX_TIME_BLINK : 1.0f;
    // Flag zombie on a pool lane still wears the duck tube (repvz
    // Zombie.cpp:1020-1023: pool rows reveal the duckytube/whitewater tracks).
    // wrap_in_duckytube can't swap its state slot (that would bury the flag
    // under the plain body), and the flag body shares the plain doc, so the
    // render hook force-shows the same tube tracks on top of the flag state.
    static const std::unordered_set<std::string> FLAG_DUCKY_TUBE = {
        "zombie_duckytube", "zombie_whitewater", "zombie_whitewater2"
    };
    const auto *extra_show = (type == FLAG_TYPE && level.row_has_water(row))
                                 ? &FLAG_DUCKY_TUBE : nullptr;
    // Roof slope: draw the body down the sloped columns (render-only).
    return reanimz::drawZombie(directory_num, armor_dir, reanim_time,
                               x_location + layout_dx - _minus_x,
                               y_location + layout_dy + roof_slope_offset(x_location + layout_dx),
                               cold_time > 0, attacked_time > 0,
                               nullptr, extra_show, nullptr, nullptr, fade);
}

bool ZombiePart::display_reanim_death(const int &_row)
{
    if (row != _row)
        return false;
    int y_location = cells[row][0].y1 - 50 + roof_slope_offset(x_location);
    apply_zombie_render_mode(); // big_trouble corpses stay half-size too
    // The corpse anim and toss ride the same time multiplier as the walk
    // cycle, so big_trouble deaths pop proportionally faster.
    reanimz::drawDeathPart(frame * 0.2f * reanimz::zombieTimeMul(), x_location, y_location,
                           is_cold, skin, in_water);
    // anim advances per logic tick, not per render (auto-FPS)
    for (int _lt = logic_ticked; _lt > 0 && check_status(game_state, IS_PAUSED) == false; _lt--)
        frame++;
    unsigned seed = (unsigned)x_location * 2654435761u ^ (unsigned)y_location * 40503u;
    // big_trouble ages the scaled clock, so real ticks shrink with it.
    const float tm = reanimz::zombieTimeMul() > 1.0f ? reanimz::zombieTimeMul() : 1.0f;
    return frame > (int)ceilf(reanimz::deathLingerTicks(skin, in_water, seed) / tm);
}

bool ZombiePart::display_reanim_drop(const int &_row)
{
    if (row != _row)
        return false;
    int y_location = cells[row][0].y1 - 50 + roof_slope_offset(x_location);
    apply_zombie_render_mode(); // same time multiplier as the corpse above
    reanimz::drawDropPart(img_num, frame * 0.2f * reanimz::zombieTimeMul(), x_location, y_location, is_cold, skin);
    // anim advances per logic tick, not per render (auto-FPS)
    for (int _lt = logic_ticked; _lt > 0 && check_status(game_state, IS_PAUSED) == false; _lt--)
        frame++;
    return frame > reanimz::dropPartLinger(img_num);
}
