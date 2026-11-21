#include "squash.hpp"
#include "draw/world_scale.hpp"
#include "elements/Map/Map.hpp"
#include "elements/elements.hpp"
#include "reanim/one_shot_fx.hpp"
#include "support.hpp"
#include "elements/Level/Level.hpp" // roof_tile_dy
#include <algorithm>

// State pacing at 60 tps (GOTY ran at ~100 fps: look 80 + prelaunch 30 +
// rising 50 + falling 10; scaled down here). Assets carry the ticks per
// reanim frame so the segments play out within their countdowns.
#define SQUASH_LOOK_TICKS 45
#define SQUASH_JUMP_TICKS 45
#define SQUASH_FALL_DAMAGE_TICK 12
#define SQUASH_FALL_TICKS 20
#define SQUASH_LAND_TICKS 35
#define SQUASH_LAND_SQUAT_TICK 110 // jumpdown snap-to frame: the flattened pose
// Crush kill zone around the locked landing spot (repvz Plant.cpp:1497-1520
// anchors mTargetX on the locked zombie's body-rect center minus half width;
// the slam then runs the attack rect GetPlantAttackRect(SQUASH) = mX+20 ..
// mX+65 over it -- net band: [lock_x + 37, lock_x + 82] vs the zombie rect).
#define SQUASH_CRUSH_X1 37
#define SQUASH_CRUSH_X2 82

static PlantAssets SQUASH_IDLE_ASSET(5, 85, 81, 85, 81, 5, 5);
static PlantAssets SQUASH_LOOK_ASSET(3, 85, 81, 85, 81, 5, 5);
static PlantAssets SQUASH_JUMP_ASSET(3, 85, 81, 85, 81, 5, 5);
static PlantAssets SQUASH_FALL_ASSET(2, 85, 81, 85, 81, 5, 5);

extern int game_state;
extern Map cells;
extern Window win;
extern Elements game_characters;

/*
Squash constructor
*/
Squash::Squash(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = SQUASH_TYPE;
    health = PLANT_HEALTH_LIMIT[SQUASH_TYPE];
    sec_for_prepare = 1;
    directory_num = SQUASH_SHEET_DIRECTORY;
    attack_range = NO_ATTACK; // targeting is custom (about a tile either way)
    state = SQUASH_READY;
    target_x = -1;
    target_zombie = nullptr;
}

Squash::~Squash()
{
}

/*
Nearest uncharmed zombie in squash's reach (GOTY: the plant's tile plus a
~70-110 px margin -- about a tile either way). Ranges sit on the zombie's
mouth point (x_location + 95, the repo's reached-block probe) so a zombie
about to chew counts too. Phase filter (repvz FindSquashTarget /
EffectedByDamage(13) -- ground-only flags, Plant.cpp:1426-1482 + 605-606):
tunneling diggers, flying balloons, mid-vault polevaulters (off the ground),
riding dolphin-bobsled mounts never get locked.
*/
Zombie *Squash::find_target()
{
    int left_limit = cells[row][col].x1 - 75;
    int right_limit = cells[row][col].x2 + 95;
    Zombie *best = nullptr;
    for (const auto &zombie : game_characters.zombies)
    {
        if (zombie->row != row || zombie->is_charmed)
            continue;
        if (zombie->is_underground() || zombie->is_flying() ||
            zombie->is_vaulting() || zombie->is_riding_vehicle())
            continue;
        int zx = zombie->x_location + 95;
        if (!is_in(left_limit, zx, right_limit))
            continue;
        if (best == nullptr || zombie->x_location < best->x_location)
            best = zombie;
    }
    return best;
}

/*
The locked zombie, if it is still standing in the row (GOTY FindSquashTarget
returns the mTargetZombieID zombie ahead of any closer candidate).
*/
Zombie *Squash::locked_target()
{
    if (target_zombie == nullptr)
        return nullptr;
    for (const auto &zombie : game_characters.zombies)
        if (zombie == target_zombie && !zombie->is_charmed && zombie->row == row &&
            !zombie->is_underground() && !zombie->is_flying() &&
            !zombie->is_vaulting() && !zombie->is_riding_vehicle())
            return zombie;
    return nullptr;
}

/*
Slam down: everything still standing in the landing zone of the row takes the
crush hit (GOTY DoSquashDamage, Plant.cpp:1406-1423: 1800 raw damage = 90
clone hp, attack rect [mX+20, mX+65] over the slid-to target spot -- football
tolerates a 20-px gap). Durables that tank it (gargantuar 150 hp, a
vulnerable boss) stay standing; targets that left the ground between the lock
and the slam (balloon liftoff winds, a vault that started, a surfacing
digger) slip the blade.
*/
void Squash::do_squash_damage()
{
    auto &zombies = game_characters.zombies;
    for (int j = 0; j < (int)zombies.size(); j++)
    {
        Zombie *z = zombies[j];
        if (z->row != row || z->is_charmed)
            continue;
        if (z->is_underground() || z->is_flying() ||
            z->is_vaulting() || z->is_riding_vehicle())
            continue; // repvz PHASE_* exclusions, same as find_target
        int overlap = std::min(z->zrect_right(), target_x + SQUASH_CRUSH_X2) -
                      std::max(z->zrect_left(), target_x + SQUASH_CRUSH_X1);
        if (overlap <= (z->type == FOOTBALL_TYPE ? -20 : 0)) // repvz line 1418
            continue;
        if (z->take_hit(90, game_characters.zombie_parts))
        {
            // repvz: squash kills carry DOESNT_LEAVE_BODY, so the dying zombie
            // runs DieNoLoot's early-out -- no falling body, no head pop, it
            // just vanishes under the squash (Zombie.cpp:9039-9047).
            z->add_zombie_crushed(game_characters.zombie_parts);
            delete z;
            zombies.erase(zombies.begin() + j);
            j--;
        }
    }
}

/*
Squash's action: idle until a zombie walks into range, then the look / jump /
slam sequence (the body itself is rendered on its own tile -- GOTY slides it
over the target, which this reanim placement path can't do yet).
*/
void Squash::action()
{
    switch (state)
    {
    case SQUASH_READY:
        if (Zombie *zombie = find_target())
        {
            target_zombie = zombie; // lock the zombie for this squash
            target_x = zombie->x_location;
            state = SQUASH_LOOKING;
            directory_num = SQUASH_LOOK_DIRECTORY;
            frame = 0;
            play_sound_effect(SQUASH_HMM_SND_MUSIC_DIRECTORY);
        }
        break;
    case SQUASH_LOOKING:
        if (frame >= SQUASH_LOOK_TICKS)
        {
            // Re-aim once at the jump (GOTY STATE_SQUASH_PRE_LAUNCH leads the
            // *locked* zombie); only if it died/left does the squash pick a
            // new closest target. The spot is frozen from here on.
            Zombie *zombie = locked_target();
            if (zombie == nullptr)
                zombie = find_target();
            if (zombie)
                target_x = zombie->x_location;
            state = SQUASH_JUMPING;
            directory_num = SQUASH_JUMPUP_DIRECTORY;
            frame = 0;
        }
        break;
    case SQUASH_JUMPING:
        if (frame >= SQUASH_JUMP_TICKS)
        {
            state = SQUASH_FALLING;
            directory_num = SQUASH_JUMPDOWN_DIRECTORY;
            frame = 0;
        }
        break;
    case SQUASH_FALLING:
        if (frame == SQUASH_FALL_DAMAGE_TICK)
            do_squash_damage();
        if (frame >= SQUASH_FALL_TICKS)
        {
            if (cells[row][col].is_water)
            {
                // pool tile: the squash sinks with a splash right away
                play_sound_effect(ZOMBIE_SPLASH_MUSIC_DIRECTORY);
                reanimfx::spawn(reanimfx::FX_SPLASH,
                                (cells[row][col].x1 + cells[row][col].x2) / 2,
                                (cells[row][col].y1 + roof_tile_dy(col)) + 55, row);
                health = 0;
            }
            else
            {
                // slam dust at the plant's base (retail PARTICLE dirt puff)
                reanimfx::spawn(reanimfx::FX_DUST_PUFF, (cells[row][col].x1 + cells[row][col].x2) / 2, cells[row][col].y2 + roof_tile_dy(col) - 2, row);
                state = SQUASH_LANDED;
                frame = SQUASH_LAND_SQUAT_TICK; // snap to the flattened pose
            }
        }
        break;
    case SQUASH_LANDED:
        if (frame >= SQUASH_LAND_SQUAT_TICK + SQUASH_LAND_TICKS)
            health = 0;
        break;
    }
}

void Squash::next_frame(const PlantAssets &assets)
{
    if (check_status(game_state, IS_PAUSED))
        return;
    // anim advances per logic tick, not per render (auto-FPS)
    for (int _lt = logic_ticked; _lt > 0; _lt--)
    {
        ++frame;
        if (state == SQUASH_READY &&
            frame >= assets.frame * all_img[SQUASH_SHEET_DIRECTORY].n_sheet)
            frame = 0;
    }
}

/*
Display squash in row. Reanim-only art (2Pak Squash.reanim).
*/
void Squash::display(const int &_row)
{
    if (row != _row)
        return;
    const PlantAssets *assets = &SQUASH_IDLE_ASSET;
    if (state == SQUASH_LOOKING)
        assets = &SQUASH_LOOK_ASSET;
    else if (state == SQUASH_JUMPING)
        assets = &SQUASH_JUMP_ASSET;
    else if (state == SQUASH_FALLING || state == SQUASH_LANDED)
        assets = &SQUASH_FALL_ASSET;
    if (display_reanim(*assets))
    {
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        next_frame(*assets);
        return;
    }
    win.draw_png_width_scaled(SQUASH_DIRECTORY, cells[row][col].x1 + 5,
                              (cells[row][col].y2 + roof_tile_dy(col)) - 76, world_px(ELEMENT_WIDTH - 8));
    next_frame(*assets);
}
