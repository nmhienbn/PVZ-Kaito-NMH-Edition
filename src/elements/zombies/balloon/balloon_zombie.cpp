#include "balloon_zombie.hpp"
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/elements.hpp"
#include "music/music.hpp"
#include "reanim/one_shot_fx.hpp"
#include "reanim/reanim_zombie.hpp"

extern Level level;
extern Map cells;

// The body floats this far above its ground box while the balloon holds.
const int BALLOON_ALTITUDE = 45;
// Pop drop: play anim_pop once over this many ticks while sinking to ground.
const int BALLOON_POP_TICKS = 35;

BalloonZombie::~BalloonZombie()
{
}

void BalloonZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = *ZOMBIE_HEALTH_LIMIT[BALLOON_TYPE].rbegin();
    directory_num = BALLOON_WALK_DIRECTORY;
}

void BalloonZombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - dir_height);
    directory_num = BALLOON_WALK_DIRECTORY;
    state = WALKING;
}

/*px above the ground box: full altitude while flying, sinking during the pop.*/
int BalloonZombie::fly_offset() const
{
    if (state == FLYING)
        return BALLOON_ALTITUDE;
    if (state == POPPING)
        return BALLOON_ALTITUDE * pop_ticks / BALLOON_POP_TICKS;
    return 0;
}

/*repvz Zombie.cpp:630 + 8391: balloon keeps mZombieRect (36, 30, 42, 115)
and GetZombieRect slides it up by the flight altitude (GetDrawPos mBodyY =
-mAltitude). Grounded (fly_offset 0) it lands on the authored box as-is.*/
void BalloonZombie::zrect_off(int &x, int &y, int &w, int &h) const
{
    Zombie::zrect_off(x, y, w, h);
    y -= fly_offset();
}

bool BalloonZombie::on_move_tick()
{
    if (state == FLYING)
    {
        if (!inflate_played && x_location < WINDOW_WIDTH - 10)
        {
            inflate_played = true;
            play_sound_effect(BALLOON_INFLATE_MUSIC_DIRECTORY);
        }
        /*The balloon floats along at the same pace as a plain walker: in
        the original, PickRandomSpeed runs before the balloon's per-type
        init, so every balloon gets the default 0.23-0.32 px/cs roll
        (repvz Zombie.cpp:180 + the default branch at :1215-1226). The
        chill factor mirrors move_zombies.]*/
        float v = speed_px_per_tick();
        if (cold_time)
            v *= CHILLED_SPEED_FACTOR;
        walk_carry += v;
        int step = (int)walk_carry;
        walk_carry -= step;
        x_location -= step;
        return true;
    }
    if (state == POPPING)
    {
        if (--pop_ticks <= 0)
        {
            state = WALKING;
            frame = 0;
            reanim_time = 0.0f;
            reanim_rate = 0.2f;
        }
        return true;
    }
    return false; // walking: base rules
}

/*Flying/popping balloons glide over plants instead of stopping to eat.*/
bool BalloonZombie::on_reach_plant(const int &col)
{
    return state != WALKING;
}

bool BalloonZombie::can_eat_plant() const
{
    return state == WALKING;
}

bool BalloonZombie::is_flying() const
{
    return state == FLYING;
}

/*Cactus spike / Blover: pop the balloon. Over a water lane the zombie sinks
(drowns) the moment the balloon is gone; on soil it falls and keeps walking.*/
void BalloonZombie::pop_balloon(vector<ZombiePart> &zombie_parts)
{
    if (state != FLYING)
        return;
    play_sound_effect(BALLOON_POP_MUSIC_DIRECTORY);
    if (level.row_has_water(row) && cells[row][0].is_water)
    {
        play_sound_effect(ZOMBIE_FALLING_MUSIC_DIRECTORY);
        reanimfx::spawn(reanimfx::FX_SPLASH, x_location + 85,
                        cells[row][0].y1 + 55, row);
        add_zombie_die(zombie_parts);
        landed_in_water = true; // the sweeper removes the sunken zombie
        return;
    }
    state = POPPING;
    pop_ticks = BALLOON_POP_TICKS;
    frame = 0;
    reanim_time = 0.0f;
    float len = reanimz::skinSegmentLength(reanimz::SKIN_BALLOON, "anim_pop");
    reanim_rate = len > 0 ? len / BALLOON_POP_TICKS : 0.2f;
}

/*Plain damage flash on degrade (the skin wires no armless variant).*/
void BalloonZombie::determine_appearance(vector<ZombiePart> &zombie_parts)
{
}

void BalloonZombie::add_zombie_die(vector<ZombiePart> &zombie_parts)
{
    if (reanimz::ready())
    {
        zombie_parts.push_back(ZombiePart(reanimz::DEATH_PART_IMG, 1, row, x_location,
                                          ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                          reanimz::SKIN_BALLOON));
        return;
    }
    Zombie::add_zombie_die(zombie_parts);
}

void BalloonZombie::render_zombie()
{
    if (state == FLYING)
    {
        if (reanimz::drawSkinSegment(reanimz::SKIN_BALLOON, "anim_idle", reanim_time,
                                     x_location, y_location - fly_offset(),
                                     cold_time > 0, attacked_time > 0,
                                     0, 0, WORLD_SCALE))
            return;
    }
    else if (state == POPPING)
    {
        if (reanimz::drawSkinSegment(reanimz::SKIN_BALLOON, "anim_pop", reanim_time,
                                     x_location, y_location - fly_offset(),
                                     cold_time > 0, attacked_time > 0,
                                     0, 0, WORLD_SCALE, false))
            return;
    }
    int save_y = y_location;
    y_location -= fly_offset(); // sprite-sheet fallback keeps the altitude
    Zombie::render_zombie();
    y_location = save_y;
}
