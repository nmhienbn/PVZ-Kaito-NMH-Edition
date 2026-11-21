#include "zomboni_zombie.hpp"
#include "../zombie.hpp" // plant_rect_span
#include "../roof/roof_shared.hpp" // crush_plants_at
#include "draw/rsdl.hpp"
#include "elements/Level/Level.hpp"
#include "music/music.hpp"
#include "reanim/reanim_zombie.hpp"
#include <algorithm>

extern Level level;

// zamboni.ogg retrigger period while the driver is alive (~4 s loop).
const int ZAMBONI_SOUND_CLK = 240;
// Ice front behind the rear wheels (original: mPosX + 118).
const int ZOMBONI_ICE_BACK = 118;

ZomboniZombie::~ZomboniZombie()
{
}

void ZomboniZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = *ZOMBIE_HEALTH_LIMIT[ZOMBONI_TYPE].rbegin(); // 68 (zombies.json)
    directory_num = ZOMBONI_DRIVE_DIRECTORY;
}

void ZomboniZombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - dir_height);
    directory_num = ZOMBONI_DRIVE_DIRECTORY;
}

int ZomboniZombie::ice_front_x(int row)
{
    if (row < 0 || row >= (int)level.ice_min_x.size())
        return -1;
    // Melted ice is no trail: the timer is the source of truth
    // (Board::IsIceAt requires mIceTimer > 0 too).
    if (row < (int)level.ice_timer.size() && level.ice_timer[row] <= 0)
        return -1;
    return level.ice_min_x[row];
}

bool ZomboniZombie::on_move_tick()
{
    // Engine hum, replayed as long as the zomboni is around.
    if (--engine_sound_clk <= 0)
    {
        engine_sound_clk = ZAMBONI_SOUND_CLK;
        play_sound_effect(ZAMBONI_MUSIC_DIRECTORY);
    }
    // Lay the ice trail behind the wheels (per row, leftmost extent).
    if ((int)level.ice_min_x.size() < GRID_ROWS)
        level.ice_min_x.assign(GRID_ROWS, -1);
    if ((int)level.ice_timer.size() < GRID_ROWS)
        level.ice_timer.assign(GRID_ROWS, 0);
    if (is_moving && row >= 0 && row < (int)level.ice_min_x.size())
    {
        int front = x_location + ZOMBONI_ICE_BACK;
        if (level.ice_min_x[row] < 0 || front < level.ice_min_x[row])
            level.ice_min_x[row] = front;
        // The melt timer ticks back up while the trail head is on the board
        // (original UpdateZamboni: mIceTimer = 3000cs while anIceX < 800 of
        // the 800px board).
        if (front < WINDOW_WIDTH)
            level.ice_timer[row] = ICE_TRAIL_TICKS;
    }
    // Drive-over crush (repvz Zombie::CheckSquish, Zombie.cpp:6595-6611): the
    // hull's attack box (mZombieAttackRect (10,-13,133,140), :406) against
    // each same-row plant's rect (Plant::GetPlantRect, Plant.cpp:5221-5240);
    // >= 20 px of overlap flattens the whole tile (SquishAllInSquare), no
    // bite-clock delay. Spiky plants are skipped (:6605 -- the spikeweed's
    // own DoRowAreaDamage pop handles the tire, Plant.cpp:678-697) and
    // CanTargetPlant's DRIVE_OVER guard (:6406-6417) refuses the one-shot
    // plants (cherry/jalapeno/blover/squash; doom/ice only while awake).
    // on_reach_plant below only keeps the wheels rolling.
    if (is_moving && row >= 0)
    {
        int a_left, a_width;
        attack_x_span(a_left, a_width);
        const int h_left = x_location + a_left, h_right = h_left + a_width;
        for (int i = 0; i < (int)game_characters.plants.size(); i++)
        {
            Plants *p = game_characters.plants[i];
            if (p->get_row() != row)
                continue;
            const int t = p->get_type();
            if (t == SPIKEWEED_TYPE || t == CHERRYBOMB_TYPE || t == JALAPENO_TYPE ||
                t == BLOVER_TYPE || t == SQUASH_TYPE)
                continue;
            if ((t == DOOMSHROOM_TYPE || t == ICESHROOM_TYPE) &&
                (level.is_night() || p->coffee_awake))
                continue; // awake bomb-shrooms are not rolled over
            int p_left, p_right;
            plant_rect_span(row, p->get_col(), p_left, p_right);
            if (std::min(h_right, p_right) - std::max(h_left, p_left) >= 20)
            {
                crush_plants_at(row, p->get_col());
                play_sound_effect(GARGANTUAR_THUMP_MUSIC_DIRECTORY);
                break; // repvz: one SquishAllInSquare per tick
            }
        }
    }
    return false;
}

/*The zomboni never stops to chew: the drive-over crush in on_move_tick
flattens the tile the tick the nose overlaps it; this hook only keeps the
default "stop and eat" from parking the hull (the ZombieVsPlant crush branch
remains as the backstop for edge cases).*/
bool ZomboniZombie::on_reach_plant(const int &col)
{
    return true;
}

bool ZomboniZombie::can_eat_plant() const
{
    return false; // crushes instead of chewing
}

bool ZomboniZombie::crushes_plants() const
{
    return true;
}

/*Fast at the curb, eases off toward the house: repvz UpdateZamboni
(Zombie.cpp:4015) sets mVelX = TodAnimateCurveFloat(700, 300, mPosX, 0.25,
0.05, LINEAR) every tick on the ORIGINAL board coordinate (clone x - 220).
Same curve here, x HEAD_SPEED_SCALE: 13.6 px/s at clone-x 920 falling
linearly to 2.7 px/s by clone-x 520, in px/tick.*/
float ZomboniZombie::speed_px_per_tick() const
{
    float f = (float)(x_location - 220 - 300) / 400.0f;
    if (f > 1.0f)
        f = 1.0f;
    if (f < 0.0f)
        f = 0.0f;
    return CS_VEL(0.05f + (0.25f - 0.05f) * f) * HEAD_SPEED_SCALE;
}

/*Damage shows as the attack flash only (no degrade art in the zamboni
doc).*/
void ZomboniZombie::determine_appearance(vector<ZombiePart> &zombie_parts)
{
}

/*No corpse: Zombie_zamboni.reanim has no anim_death, so there is nothing to
draw (the hull vanishes with the falling-zombie sound).*/
void ZomboniZombie::add_zombie_die(vector<ZombiePart> &zombie_parts)
{
}
