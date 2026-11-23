#include "dolphinrider_zombie.hpp"
#include "../zombie.hpp" // plant_rect_span
#include "../roof/roof_shared.hpp" // plant_index_at
#include "draw/rsdl.hpp"
#include "music/music.hpp"
#include "reanim/one_shot_fx.hpp"
#include "reanim/reanim_zombie.hpp"
#include <algorithm>

// Leap into the pool (repvz PlayZombieReanim "anim_jumpinpool",
// REANIM_PLAY_ONCE_AND_HOLD, blend 20, 16.0 fps; Zombie.cpp:1838-1861): the
// 2Pak take is 45 frames -> 2.81 s. The splash fires at the 0.56 timed event
// (t ~= 0.56 * 169); once the take has run the body lurches 70 px into the
// water and the ride loop starts.
const int DOLPHIN_INTO_POOL_TICKS = 169;      // ceil(45 / 16 * 60)
const float DOLPHIN_INTO_POOL_FRAMES = 45.0f; // anim_jumpinpool (313..357)
const int DOLPHIN_INTO_POOL_SPLASH_TICK = 95; // 0.56 of the take
const int DOLPHIN_INTO_POOL_LURCH = 70;       // mPosX -= 70 at loop end
// Vault: repvz mVelX = 0.5 px/cs while the jump take runs at 10 fps (the 2Pak
// anim_dolphinjump take is 21 frames -> 2.1 s), zeroed at the 0.49 splash
// event (~tick 62); the landing snaps another 94 px past the plant when the
// loop completes (Zombie.cpp:1884-1933). Total travel ~= 28 + 94 = 122 px at
// HEAD pace.
const int DOLPHIN_JUMP_TICKS = 126;      // 21 / 10 * 60
const float DOLPHIN_JUMP_FRAMES = 21.0f; // anim_dolphinjump (365..385)
const int DOLPHIN_JUMP_SPLASH_TICK = 62; // 0.49 of the take
const float DOLPHIN_JUMP_PX = CS_VEL(0.5f) * HEAD_SPEED_SCALE;
const int DOLPHIN_JUMP_LAND = 94;
// Shore approach + ride speed: the original rolls the dolphin phases
// (PHASE_DOLPHIN_WALKING walk-in -- the spawn PickRandomSpeed call at
// Zombie.cpp:344-348 -- and the carried ride alike) at
// RandRangeFloat(0.89, 0.91) px/cs (PickRandomSpeed, Zombie.cpp:1210-1214;
// mid-band 0.90 here) -> 49.1 px/s under HEAD_SPEED_SCALE.
const float DOLPHIN_FAST_PX = CS_VEL(0.90f) * HEAD_SPEED_SCALE;
// The arm pops at the original's 2/3-body mark (2/3*500/20 = 16.67;
// zombies.json "dolphinrider": [16.5, 25]).
const float DOLPHIN_ARM_HEALTH = 16.5f;

DolphinRiderZombie::~DolphinRiderZombie()
{
}

void DolphinRiderZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = *ZOMBIE_HEALTH_LIMIT[DOLPHINRIDER_TYPE].rbegin(); // 25 (zombies.json)
    directory_num = DOLPHINRIDER_WALK_DIRECTORY;
    // Spawn on the shore strip carrying the dolphin ("anim_walkdolphin" via
    // DOLPHIN_CARRY_STATE); the ride skin only comes on in the water.
    state = WALK_IN;
    armor_dir = reanimz::DOLPHIN_CARRY_STATE;
}

void DolphinRiderZombie::enter_pool()
{
    // PHASE_DOLPHIN_WALKING -> PHASE_DOLPHIN_INTO_POOL at mX in (700, 720]:
    // leap in with anim_jumpinpool; the splash, the 70-px lurch and the ride
    // switch run off into_pool_ticks in on_move_tick.
    state = INTO_POOL;
    armor_dir = reanimz::DOLPHIN_INTO_POOL_STATE;
    into_pool_ticks = 0;
    frame = 0;
    reanim_time = 0.0f;
    reanim_rate = DOLPHIN_INTO_POOL_FRAMES / DOLPHIN_INTO_POOL_TICKS;
    play_sound_effect(DOLPHIN_APPEARS_MUSIC_DIRECTORY);
}

void DolphinRiderZombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - dir_height);
    state = SWIMMING;
    armor_dir = -1;
    directory_num = DOLPHINRIDER_WALK_DIRECTORY;
}

bool DolphinRiderZombie::on_move_tick()
{
    if (state == WALK_IN)
    {
        // Shore strip: sprint in at the dolphin band; the original leaps in
        // at mX in (700, 720] (Zombie.cpp:1835), i.e. mid-column-8 here
        // (board offset +220 px).
        if (x_location <= cells[row][8].x1 + 30)
            enter_pool();
        return false;
    }
    if (state == INTO_POOL)
    {
        into_pool_ticks++;
        // 0.56 timed event: splash + pool-entry foley (Zombie.cpp:1844-1850
        // -- the original adds 1.2 x 0.8 scale, this effect has no scale
        // hook).
        if (into_pool_ticks == DOLPHIN_INTO_POOL_SPLASH_TICK)
        {
            play_sound_effect(ZOMBIE_ENTERING_WATER_MUSIC_DIRECTORY);
            reanimfx::spawn(reanimfx::FX_SPLASH, x_location - 83,
                            cells[row][0].y1 + 55, row);
        }
        if (into_pool_ticks >= DOLPHIN_INTO_POOL_TICKS)
        {
            // Take done: the 70-px lurch, then the ride loop (Zombie.cpp:1854-
            // 1860:lurch, mInPool, attack rect, "anim_ride").
            state = RIDING;
            armor_dir = reanimz::DOLPHIN_RIDING_STATE;
            x_location -= DOLPHIN_INTO_POOL_LURCH;
            frame = 0;
            reanim_time = 0.0f;
            // head-pace fast-class feed: 20 fps x ride/base speed ratio
            reanim_rate = (20.0f / 60.0f) * DOLPHIN_FAST_PX / CS_VEL(0.15f);
        }
        return false; // the leap crawls at the plain roll, spanning the take
                      // (repvz drives INTO_POOL off the take's ground track)
    }
    if (state == RIDING)
    {
        /*repvz target lock for the vault: the ride jumps the first plant the
        attack box overlaps AT ALL (FindPlantTarget runs GetRectOverlap > 0,
        Zombie.cpp:6459-6476; the 20-px band is only the chewers' stop rule).
        The 70-px pool-entry lurch can land the box already past that narrow
        band, so probe for any positive overlap and vault/bonk via the same
        on_reach_plant path.*/
        int a_left, a_width, p_left, p_right;
        attack_x_span(a_left, a_width);
        const int left = x_location + a_left;
        for (int col = HORIZ_BLOCK_COUNT - 1; col >= 0; col--)
        {
            if (!cells[row][col].is_block_zombie || cells[row][col].has_ladder)
                continue;
            plant_rect_span(row, col, p_left, p_right);
            if (std::min(left + a_width, p_right) - std::max(left, p_left) > 0)
            {
                on_reach_plant(col);
                return state == JUMPING; // vault started: this tick is scripted
            }
        }
        return false; // ride on
    }
    if (state != JUMPING)
        return false;
    jump_ticks++;
    if (jump_ticks == DOLPHIN_JUMP_SPLASH_TICK)
    {
        // 0.49 timed event: splash + entering-water foley, then no more
        // forward grind (mVelX = 0) for the rest of the take
        // (Zombie.cpp:1917-1924).
        play_sound_effect(ZOMBIE_ENTERING_WATER_MUSIC_DIRECTORY);
        reanimfx::spawn(reanimfx::FX_SPLASH, x_location - 63,
                        cells[row][0].y1 + 55, row);
    }
    else if (jump_ticks < DOLPHIN_JUMP_SPLASH_TICK)
    {
        // mVelX grind until the splash event, fractional-carry over ticks.
        jump_carry += DOLPHIN_JUMP_PX;
        int step = (int)jump_carry;
        jump_carry -= step;
        x_location -= step;
    }
    if (jump_ticks >= DOLPHIN_JUMP_TICKS)
    {
        // Loop complete: snap the last 94 px past the plant and wade on
        // (Zombie.cpp:1926-1933).
        x_location -= DOLPHIN_JUMP_LAND;
        state = SWIMMING;
        armor_dir = -1; // wading segment
        frame = 0;
        reanim_time = 0.0f;
        reanim_rate = 20.0f / 60.0f; // wade = plain-roll pace: HEAD 20 fps pin
    }
    return true;
}

bool DolphinRiderZombie::on_reach_plant(const int &col)
{
    if (state == JUMPING)
        return true; // mid-air: the plant stays untouched until landing
    if (state == INTO_POOL)
        return true; // mid-leap: no chewing (can_eat_plant is false too)
    if (state == WALK_IN)
        enter_pool(); // lily pad / pot at the water line: splash down first
    if (state != RIDING)
        return false; // dolphin lost -> eat like any zombie
    // A Tall-nut is too tall to jump: the dolphin rears up against it and
    // leaves; the rider drops in front and wades on (the original starts the
    // jump and bonks at its 0.3 event, Zombie.cpp:1902-1916 -- here the bonk
    // fires on contact instead, no partial take).
    int p = plant_index_at(row, col);
    if (p >= 0 && game_characters.plants[p]->get_type() == TALLNUT_TYPE)
    {
        state = SWIMMING;
        armor_dir = -1; // wading segment
        frame = 0;
        reanim_time = 0.0f;
        reanim_rate = 20.0f / 60.0f; // wade = plain-roll pace: HEAD 20 fps pin
        play_sound_effect(BONK_MUSIC_DIRECTORY);
        return false;
    }
    state = JUMPING;
    jump_ticks = 0;
    jump_carry = 0.0f;
    armor_dir = reanimz::DOLPHIN_JUMPING_STATE;
    frame = 0;
    reanim_time = 0.0f;
    reanim_rate = DOLPHIN_JUMP_FRAMES / DOLPHIN_JUMP_TICKS;
    play_sound_effect(DOLPHIN_BEFORE_JUMPING_MUSIC_DIRECTORY);
    return true;
}

bool DolphinRiderZombie::can_eat_plant() const
{
    // Only chews once the dolphin is gone; riding triggers the jump instead.
    return state == SWIMMING;
}

/*repvz phase boxes: riding the dolphin nose leads (mZombieAttackRect
(-29, 0, 70), Zombie.cpp:1857), mid-jump GetZombieAttackRect swaps in
(-40, 0, 100) (:8406), and wading after the jump it is (30, 0, 30) (:1927).
Walking up the shore strip keeps the plain default (50, 20).*/
void DolphinRiderZombie::attack_x_span(int &left, int &width) const
{
    switch (state)
    {
    case RIDING:   left = -29, width = 70;  break;
    case JUMPING:  left = -40, width = 100; break;
    case SWIMMING: left = 30,  width = 30;  break;
    default:       Zombie::attack_x_span(left, width);
    }
}

/*The walk-in sprints at the same 0.89-0.91 px/cs roll as the ride, x
HEAD_SPEED_SCALE (repvz: ZombieInitialize runs PickRandomSpeed on
PHASE_DOLPHIN_WALKING, Zombie.cpp:344-348). The pool leap keeps the plain-roll
crawl (the original drives INTO_POOL off the jumpinpool take's ground track,
leaving mVelX untouched), and the post-jump wade picks the normal walk pace
(StartWalkAnim re-roll: PHASE_DOLPHIN_WALKING_IN_POOL = 0.3 px/cs,
Zombie.cpp:1169-1172 -- right inside the plain band).*/
void DolphinRiderZombie::pick_speed()
{
    Zombie::pick_speed();
    // The carry segment's feet track the HEAD-pace sprint (20 fps x
    // speed/base); every later phase sets its own rate (leap take, ride
    // loop, jump take, wade).
    reanim_rate = (20.0f / 60.0f) * DOLPHIN_FAST_PX / CS_VEL(0.15f);
}

float DolphinRiderZombie::speed_px_per_tick() const
{
    return state == WALK_IN || state == RIDING ? DOLPHIN_FAST_PX
                                               : Zombie::speed_px_per_tick();
}

/*Damage shows as the attack flash + the arm popping at critical health (no
armor / degrade art in the dolphin doc).*/
void DolphinRiderZombie::determine_appearance(vector<ZombiePart> &zombie_parts)
{
    if (!armless && health <= DOLPHIN_ARM_HEALTH * health_scale)
    {
        armless = true;
        if (reanimz::ready())
            zombie_parts.push_back(ZombiePart(reanimz::DROP_ARM_IMG, 1, row, x_location,
                                              ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                              reanimz::SKIN_DOLPHINRIDER));
    }
}

void DolphinRiderZombie::add_zombie_die(vector<ZombiePart> &zombie_parts)
{
    if (reanimz::ready())
    {
        zombie_parts.push_back(ZombiePart(reanimz::DEATH_PART_IMG, 1, row, x_location,
                                          ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                          reanimz::SKIN_DOLPHINRIDER, false, head_dropped));
        return;
    }
    Zombie::add_zombie_die(zombie_parts);
}
