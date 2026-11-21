#include "snorkel_zombie.hpp"
#include "draw/rsdl.hpp"
#include "elements/Map/Map.hpp"
#include "music/music.hpp"
#include "reanim/one_shot_fx.hpp"
#include "reanim/reanim_zombie.hpp"

extern Map cells;

// Leap into the pool (repvz PlayZombieReanim "anim_jumpinpool",
// REANIM_PLAY_ONCE_AND_HOLD, blend 20, 16.0 fps; Zombie.cpp:1954-1978): the
// 2Pak snorkle take is 19 frames -> 1.19 s. mVelX drops to 0.2 px/cs for the
// leap (10.9 px/s at HEAD pace); the splash fires at the 0.83 timed event,
// then the zombie surfaces into "anim_swim".
const int SNORKEL_INTO_POOL_TICKS = 72;      // ceil(19 / 16 * 60) = 71.25
const float SNORKEL_INTO_POOL_FRAMES = 19.0f; // anim_jumpinpool (79..97)
const int SNORKEL_INTO_POOL_SPLASH_TICK = 59; // 0.83 of the take
const float SNORKEL_INTO_POOL_PX = CS_VEL(0.2f) * HEAD_SPEED_SCALE;

SnorkelZombie::~SnorkelZombie()
{
}

void SnorkelZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = *ZOMBIE_HEALTH_LIMIT[SNORKEL_TYPE].rbegin(); // 10 (zombies.json)
    directory_num = SNORKEL_WALK_DIRECTORY;               // maps to anim_swim while moving
}

void SnorkelZombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - dir_height);
    directory_num = SNORKEL_WALK_DIRECTORY;
}

bool SnorkelZombie::on_move_tick()
{
    if (state == WALK_SHORE)
    {
        // Shore strip: plain walk; the original leaps in at mX in (700, 720]
        // (Zombie.cpp:1954), i.e. mid-column-8 here (board offset +220 px).
        if (x_location <= cells[row][8].x1 + 30)
        {
            state = INTO_POOL;
            armor_dir = reanimz::SNORKEL_INTO_POOL_STATE;
            into_pool_ticks = 0;
            frame = 0;
            reanim_time = 0.0f;
            reanim_rate = SNORKEL_INTO_POOL_FRAMES / SNORKEL_INTO_POOL_TICKS;
        }
        return false;
    }
    if (state == INTO_POOL)
    {
        into_pool_ticks++;
        // 0.83 timed event: splash + pool-entry foley (Zombie.cpp:1965-1973
        // -- the original adds 1.2 x 0.8 scale, this effect has no scale
        // hook).
        if (into_pool_ticks == SNORKEL_INTO_POOL_SPLASH_TICK)
        {
            play_sound_effect(ZOMBIE_ENTERING_WATER_MUSIC_DIRECTORY);
            reanimfx::spawn(reanimfx::FX_SPLASH, x_location - 47,
                            cells[row][0].y1 + 55, row);
        }
        if (into_pool_ticks >= SNORKEL_INTO_POOL_TICKS)
        {
            // Take done: in the pool for real (mInPool), swimming loop.
            state = IN_POOL;
            armor_dir = -1;
            frame = 0;
            reanim_time = 0.0f;
            reanim_rate = 0.2f;
        }
        return false;
    }
    return false;
}

/*Mid-leap it does not chew (repvz gates eating on PHASE_SNORKEL_WALKING/
WALKING_IN_POOL); on the shore strip and in the water it eats normally.*/
bool SnorkelZombie::can_eat_plant() const
{
    return state != INTO_POOL;
}

/*Submerged while it swims (moving); surfaced while it eats (is_moving
false, parked at a plant). The shore walk-in and the leap-in play in the
open air. Bullet collision checks this hook.*/
bool SnorkelZombie::is_submerged() const
{
    return state == IN_POOL && is_moving;
}

/*Damage shows only as the attack flash (no torn-arm art in the snorkel
doc); the arm pops at critical health like the plain zombie's.*/
void SnorkelZombie::determine_appearance(vector<ZombiePart> &zombie_parts)
{
    if (!armless && health <= 5 * health_scale)
    {
        armless = true;
        if (reanimz::ready())
            zombie_parts.push_back(ZombiePart(reanimz::DROP_ARM_IMG, 1, row, x_location,
                                              ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                              reanimz::SKIN_SNORKEL));
    }
}

void SnorkelZombie::add_zombie_die(vector<ZombiePart> &zombie_parts)
{
    if (reanimz::ready())
    {
        zombie_parts.push_back(ZombiePart(reanimz::DEATH_PART_IMG, 1, row, x_location,
                                          ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                          reanimz::SKIN_SNORKEL));
        return;
    }
    Zombie::add_zombie_die(zombie_parts);
}

/*Brisk everywhere but the leap: shore walk AND swim run at the original's
snorkel roll (RandRangeFloat(0.66, 0.68) px/cs, repvz PickRandomSpeed
ZOMBIE_SNORKEL type branch, Zombie.cpp:1122-1125 -- the type roll applies to
the spawn walk-in too since the constructor runs it; mid-band 0.67 here)
scaled by HEAD_SPEED_SCALE -> 36.5 px/s. The leap-in crawls at mVelX 0.2 px/cs
(Zombie.cpp:1956).*/
void SnorkelZombie::pick_speed()
{
    Zombie::pick_speed();
    // The shore sprint / swim loop rates are pinned pace-coupled in
    // reanim_zombie.cpp's kPlayRateRules (fast class); the leap take sets its
    // own tick-synced rate. Flat 0.2 feed lets the pins drive.
    reanim_rate = 0.2f;
}

float SnorkelZombie::speed_px_per_tick() const
{
    if (state == INTO_POOL)
        return SNORKEL_INTO_POOL_PX;
    /*Swim: the original drops to a fixed 0.3 px/cs while submerged
    (PickRandomSpeed, Zombie.cpp:1167-1170) -- slower than its 0.67 px/cs
    land pace; it walks the far shore at the fast class again (WALK_SHORE).*/
    if (state == IN_POOL)
        return CS_VEL(0.3f) * HEAD_SPEED_SCALE;
    return state == WALK_SHORE
               ? CS_VEL(0.67f) * HEAD_SPEED_SCALE
               : Zombie::speed_px_per_tick();
}
