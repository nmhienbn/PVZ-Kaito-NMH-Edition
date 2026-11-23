#include "imp_zombie.hpp"
#include "elements/Level/Level.hpp" // roof_slope_offset
#include "../roof/roof_shared.hpp"
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "game_param.hpp"
#include "music/music.hpp"
#include "reanim/reanim_zombie.hpp"

extern int game_state;

// Flight from the gargantuar's back to the landing tile ~0.8 s; landing hop
// ~0.3 s. On the ground the imp walks at the plain per-zombie roll like the
// original (adventure imps take PickRandomSpeed's default branch; the 0.9
// px/cs figure is IZombie-only, repvz Zombie.cpp:1184-1187).
const int IMP_FLIGHT_TICKS = 50;
const int IMP_LAND_TICKS = 18;

ImpZombie::~ImpZombie()
{
}

void ImpZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = *ZOMBIE_HEALTH_LIMIT[IMP_TYPE].rbegin();
    directory_num = IMP_WALK_DIRECTORY;
}

void ImpZombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - dir_height);
    directory_num = IMP_WALK_DIRECTORY;
    state = WALKING;
}

void ImpZombie::spawn_thrown(const int &landing_x)
{
    state = THROWN;
    fly_total = IMP_FLIGHT_TICKS;
    fly_ticks = fly_total;
    fly_from_x = x_location;
    fly_to_x = landing_x;
    anim_t = 0.0f;
    is_moving = true;
}

bool ImpZombie::on_move_tick()
{
    if (state == THROWN)
    {
        if (--fly_ticks <= 0)
        {
            x_location = fly_to_x;
            state = LANDING;
            fly_ticks = IMP_LAND_TICKS;
            anim_t = 0.0f;
        }
        else
        {
            int t = fly_total - fly_ticks;
            x_location = fly_from_x + (int)((long)(fly_to_x - fly_from_x) * t / fly_total);
        }
        return true;
    }
    if (state == LANDING)
    {
        if (--fly_ticks <= 0)
        {
            state = WALKING;
            anim_t = 0.0f;
        }
        return true;
    }
    return false;
}

bool ImpZombie::on_reach_plant(const int &)
{
    // Mid-flight the imp sails over plants; on the ground it eats normally.
    return state != WALKING;
}

bool ImpZombie::can_eat_plant() const
{
    return state == WALKING;
}

void ImpZombie::determine_appearance(vector<ZombiePart> &)
{
    // No degrade visuals: the attack flash is the damage cue.
}

/*repvz Zombie.cpp:1424: HEIGHT_FALLING imps (the thrown flight + landing hop)
hit with a lowered box (36, 17, 42, 115); once walking, the default body box.*/
void ImpZombie::zrect_off(int &x, int &y, int &w, int &h) const
{
    Zombie::zrect_off(x, y, w, h);
    if (state != WALKING)
    {
        x = 36, y = 17, w = 42, h = 115;
    }
}

void ImpZombie::add_zombie_die(vector<ZombiePart> &zombie_parts)
{
    if (reanimz::ready())
    {
        zombie_parts.push_back(ZombiePart(reanimz::DEATH_PART_IMG, 1, row, x_location,
                                          ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                          reanimz::SKIN_IMP, false, head_dropped));
        return;
    }
    Zombie::add_zombie_die(zombie_parts);
}

void ImpZombie::render_zombie()
{
    if (state == WALKING)
    {
        Zombie::render_zombie(); // dir-state table: IMP_WALK/IMP_EAT -> skin imp
        return;
    }
    const char *seg = state == THROWN ? "anim_thrown" : "anim_land";
    bool loop = state == THROWN;
    // parabola height for the flight frames
    int ddy = 0;
    if (state == THROWN)
    {
        float f = (float)(fly_total - fly_ticks) / (float)fly_total;
        ddy = -(int)(140.0f * 4.0f * f * (1.0f - f)) - 30;
    }
    if (reanimz::drawSkinSegment(reanimz::SKIN_IMP, seg, anim_t,
                                 x_location, y_location + roof_slope_offset(x_location),
                                 cold_time > 0, attacked_time > 0,
                                 0, ddy, WORLD_SCALE, loop))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && !check_status(game_state, IS_PAUSED); _lt--)
            anim_t += 0.2f;
    }
}
