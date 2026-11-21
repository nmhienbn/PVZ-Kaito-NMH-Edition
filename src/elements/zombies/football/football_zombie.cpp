#include "football_zombie.hpp"
#include "draw/rsdl.hpp"
#include "music/music.hpp"
#include "reanim/reanim_zombie.hpp"

FootballZombie::~FootballZombie()
{
}

void FootballZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = FOOTBALL_TOTAL_HEALTH;
    directory_num = FOOTBALL_RUN_DIRECTORY;
}

void FootballZombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - dir_height);
    directory_num = FOOTBALL_RUN_DIRECTORY;
}

/*Run speed: the original rolls every football zombie at
RandRangeFloat(0.66, 0.68) px/cs (repvz PickRandomSpeed,
Zombie.cpp:1201-1205) -- rolled once per spawn like the plain zombies, then
scaled to HEAD pace: 36.0-37.1 px/s.*/
void FootballZombie::pick_speed()
{
    step_speed = rand_float(CS_VEL(0.66f), CS_VEL(0.68f)) * HEAD_SPEED_SCALE;
}

float FootballZombie::speed_px_per_tick() const
{
    return step_speed;
}

/*Only the left arm drops at one-third health (threshold, not exact hit);
the one-stage helmet art never degrades.*/
void FootballZombie::determine_appearance(vector<ZombiePart> &zombie_parts)
{
    if (health <= FOOTBALL_ARM_HEALTH * health_scale && armor_dir != reanimz::FOOT_ARMLESS_STATE)
    {
        armor_dir = reanimz::FOOT_ARMLESS_STATE;
        if (reanimz::ready())
            zombie_parts.push_back(ZombiePart(reanimz::DROP_ARM_IMG, 1, row, x_location,
                                              ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                              reanimz::SKIN_FOOTBALL));
        else
            zombie_parts.push_back(ZombiePart(ZOMBIE_ARM_DIRECTORY, 1, row, x_location,
                                              ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0));
    }
}

void FootballZombie::add_zombie_die(vector<ZombiePart> &zombie_parts)
{
    if (reanimz::ready())
    {
        zombie_parts.push_back(ZombiePart(reanimz::DEATH_PART_IMG, 1, row, x_location,
                                          ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                          reanimz::SKIN_FOOTBALL));
        return;
    }
    Zombie::add_zombie_die(zombie_parts);
}
