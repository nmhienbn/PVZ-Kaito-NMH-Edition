#include "gargantuar_zombie.hpp"
#include "../dancer/dancer_zombie.hpp" // queue_spawned_zombie
#include "../imp/imp_zombie.hpp"
#include "../roof/roof_shared.hpp"
#include "../zombie_type.hpp" // init_zombie
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "game_param.hpp"
#include "music/music.hpp"
#include "reanim/reanim_zombie.hpp"

extern int game_state;
extern Level level;

namespace
{
// One slam ~1 s; the weapon lands at 2/3 of the swing.
const int GARG_SMASH_TICKS = 60;
// Imp launch anim ~0.75 s. The landing spot is the original's anchored
// formula, not a tile count: aThrowingDistance = mPosX - 360 (repvz
// Zombie.cpp:2206-2234) -- the clone's lawn grid sits +220 px (X_UPPER_LEFT
// 260 vs the original's 40), so read it as x_location - 580.
const int GARG_THROW_TICKS = 45;
/*Full landing-distance computation (repvz Zombie.cpp:2206-2234): base
aThrowingDistance = mPosX - 360; on the roof it is 180 shorter and may go
negative down to -140 (the imp plops down at the gargantuar's feet); clamped
below by 40 on other maps; when the throw rolls long (> 140) a Rand(0..100)
bit lands the imp up to 100 px deeper (x ~447..547, tiles ~2.3-3.6).*/
float imp_throwing_distance(const int &garg_x, bool on_roof)
{
    float dist = garg_x - 580.0f; // mPosX - 360, +220 board offset worked in
    float min_dist = 40.0f;
    if (on_roof)
    {
        dist -= 180.0f;
        min_dist = -140.0f;
    }
    if (dist < min_dist)
        dist = min_dist;
    else if (dist > 140.0f)
        dist -= rand(0, 100);
    return dist;
}
// The imp leaves from the gargantuar's back (gốc mPosX - 133; relative, so
// the board offset cancels).
const int IMP_LAUNCH_DX = 133;
// The original checks the throw threshold as health < max/2 (strict); the
// mini variant (big_trouble) keeps the same relative mark via health_scale.
float imp_threshold(const float &health_scale)
{
    return *ZOMBIE_HEALTH_LIMIT[GARGANTUAR_TYPE].rbegin() / 2 * health_scale;
}
} // namespace

GargantuarZombie::~GargantuarZombie()
{
}

void GargantuarZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = *ZOMBIE_HEALTH_LIMIT[GARGANTUAR_TYPE].rbegin();
    directory_num = GARGANTUAR_WALK_DIRECTORY;
}

void GargantuarZombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - dir_height);
    directory_num = GARGANTUAR_WALK_DIRECTORY;
    state = WALKING;
}

bool GargantuarZombie::on_reach_plant(const int &col)
{
    if (state == SMASHING)
        return true; // swing already in progress
    if (state == THROWING)
        return true;
    if (plant_index_at(row, col) < 0)
        return true; // nothing left on the tile: step over the crater
    // Smash the tile instead of stopping to chew.
    state = SMASHING;
    smash_ticks = GARG_SMASH_TICKS;
    smash_col = col;
    smash_hit_done = false;
    anim_t = 0.0f;
    float len = reanimz::skinSegmentLength(reanimz::SKIN_GARGANTUAR, "anim_smash");
    anim_rate = len > 0 ? len / GARG_SMASH_TICKS : 0.2f;
    return true;
}

bool GargantuarZombie::on_move_tick()
{
    // Half health without a forward check: hurl the imp -- but never when it
    // would plop past the doorstep (repvz Zombie.cpp:2280: throw only while
    // aThrowingDistance > 40; rechecked every tick, and the gate never
    // re-opens once the gargantuar walked past it).
    if (!imp_thrown && health < imp_threshold(health_scale) && state == WALKING)
    {
        if (x_location - 580.0f <= 40.0f)
            return false; // too close to the house: the imp stays on its back
        imp_thrown = true;
        state = THROWING;
        throw_ticks = GARG_THROW_TICKS;
        anim_t = 0.0f;
        float len = reanimz::skinSegmentLength(reanimz::SKIN_GARGANTUAR, "anim_throw");
        anim_rate = len > 0 ? len / GARG_THROW_TICKS : 0.2f;
        return true;
    }
    if (state == THROWING)
    {
        if (--throw_ticks <= 0)
        {
            // Imp leaves at the end of the chuck; landing spot per repvz
            // Zombie.cpp:2206-2234 (fixed ~x 447, deeper over the roof).
            ImpZombie *imp = static_cast<ImpZombie *>(init_zombie(IMP_TYPE, level.level_num));
            imp->row = row;
            imp->x_location = x_location - IMP_LAUNCH_DX;
            imp->y_location = cells[row][0].y1 - 50;
            imp->wave_id = wave_id;
            imp->spawn_thrown(imp->x_location -
                              (int)imp_throwing_distance(x_location, level.is_roof()));
            queue_spawned_zombie(imp);
            play_sound_effect(IMP_SND_DIRECTORY);
            state = WALKING;
            anim_t = 0.0f;
            anim_rate = 0.2f;
        }
        return true;
    }
    if (state == SMASHING)
    {
        smash_ticks--;
        if (!smash_hit_done && smash_ticks <= GARG_SMASH_TICKS / 3)
        {
            smash_hit_done = true;
            if (crush_plants_at(row, smash_col))
                play_sound_effect(GARGANTUAR_THUMP_MUSIC_DIRECTORY);
        }
        if (smash_ticks <= 0)
        {
            state = WALKING;
            smash_col = -1;
            anim_t = 0.0f;
            anim_rate = 0.2f;
        }
        return true;
    }
    return false;
}

bool GargantuarZombie::can_eat_plant() const
{
    return false; // it crushes, never chews
}

void GargantuarZombie::determine_appearance(vector<ZombiePart> &)
{
    // No degrade visuals: the attack flash is the damage cue.
}

void GargantuarZombie::add_zombie_die(vector<ZombiePart> &zombie_parts)
{
    play_sound_effect(GARGANTUDEATH_MUSIC_DIRECTORY);
    if (reanimz::ready())
    {
        zombie_parts.push_back(ZombiePart(reanimz::DEATH_PART_IMG, 1, row, x_location,
                                          ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                          reanimz::SKIN_GARGANTUAR));
        return;
    }
    Zombie::add_zombie_die(zombie_parts);
}

void GargantuarZombie::render_zombie()
{
    const char *seg = "anim_walk";
    bool loop = true;
    if (state == SMASHING)
    {
        seg = "anim_smash";
        loop = false;
    }
    else if (state == THROWING)
    {
        seg = "anim_throw";
        loop = false;
    }
    bool drawn = reanimz::drawSkinSegment(reanimz::SKIN_GARGANTUAR, seg, anim_t,
                                          x_location, y_location + roof_slope_offset(x_location),
                                          cold_time > 0, attacked_time > 0,
                                          0, 0, WORLD_SCALE, loop);
    if (!drawn)
    {
        Zombie::render_zombie();
        return;
    }
    // anim advances per logic tick, not per render (auto-FPS)
    for (int _lt = logic_ticked; _lt > 0 && !check_status(game_state, IS_PAUSED); _lt--)
    {
        anim_t += anim_rate;
        if (loop)
        {
            float len = reanimz::skinSegmentLength(reanimz::SKIN_GARGANTUAR, seg);
            if (len > 0 && anim_t >= len)
                anim_t -= len;
        }
    }
}
