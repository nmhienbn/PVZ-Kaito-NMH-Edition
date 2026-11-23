#include "bobsled_zombie.hpp"
#include "draw/rsdl.hpp"
#include "elements/Level/Level.hpp"
#include "reanim/reanim_zombie.hpp"
#include "../zomboni/zomboni_zombie.hpp"

BobsledZombie::~BobsledZombie()
{
}

void BobsledZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = *ZOMBIE_HEALTH_LIMIT[BOBSLED_TYPE].rbegin(); // 28.5 (zombies.json): the team LEADER's pool -- 270-pt body + the 300-pt sled helm it wears (Zombie.cpp:553-560). Follower riders are trimmed back to the bare body tier when the team is spawned (level_data.cpp).
    directory_num = BOBSLED_WALK_DIRECTORY;
}

void BobsledZombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - dir_height);
    directory_num = BOBSLED_WALK_DIRECTORY;
}

/*On the zomboni's ice trail: any part of the zombie right of the ice front
means it is still on the slick (the trail runs from the front back to the
street). A melted-out trail (ice_timer == 0) counts as no ice.*/
bool BobsledZombie::on_ice() const
{
    int front = ZomboniZombie::ice_front_x(row);
    return front >= 0 && x_location + 10 >= front;
}

/*repvz Zombie.cpp:496: sliding with the sled the whole craft is one hitbox
(mZombieRect (-50, 0, 275, 115)); BobsledCrash (d2647) reverts it to the
default body box when the ride ends.*/
void BobsledZombie::zrect_off(int &x, int &y, int &w, int &h) const
{
    if (on_ice())
    {
        x = -50, y = 0, w = 275, h = 115;
        return;
    }
    Zombie::zrect_off(x, y, w, h);
}

/*Slide on the ice at the sled's glide speed, plain walk once the ice runs
out. Original: the sliding team coasts at a fixed 0.6 px/cs
(PHASE_BOBSLED_SLIDING init, repvz Zombie.cpp:502) -> 32.7 px/s under
HEAD_SPEED_SCALE.*/
float BobsledZombie::speed_px_per_tick() const
{
    if (on_ice())
        return CS_VEL(0.6f) * HEAD_SPEED_SCALE;
    return Zombie::speed_px_per_tick();
}

void BobsledZombie::determine_appearance(vector<ZombiePart> &zombie_parts)
{
    if (!armless && health <= 5 * health_scale)
    {
        armless = true;
        if (reanimz::ready())
            zombie_parts.push_back(ZombiePart(reanimz::DROP_ARM_IMG, 1, row, x_location,
                                              ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                              reanimz::SKIN_BOBSLED));
    }
}

void BobsledZombie::add_zombie_die(vector<ZombiePart> &zombie_parts)
{
    if (reanimz::ready())
    {
        zombie_parts.push_back(ZombiePart(reanimz::DEATH_PART_IMG, 1, row, x_location,
                                          ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                          reanimz::SKIN_BOBSLED, false, head_dropped));
        return;
    }
    Zombie::add_zombie_die(zombie_parts);
}
