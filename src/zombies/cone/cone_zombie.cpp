#include "cone_zombie.hpp"

ConeZombie::~ConeZombie()
{
}

void ConeZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = *ZOMBIE_HEALTH_LIMIT[CONE_TYPE].rbegin();
    directory_num = CONE_ZOMBIE_WALK_1_DIRECTORY;
}

void ConeZombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - dir_height);
    directory_num = rand(CONE_ZOMBIE_CREDIT1_DIRECTORY, CONE_ZOMBIE_CREDIT2_DIRECTORY);
}

void ConeZombie::armor_drop(vector<ZombiePart> &zombie_parts)
{
    if (health == *ZOMBIE_HEALTH_LIMIT[NORMAL_TYPE].rbegin())
    {
        type = NORMAL_TYPE;
        zombie_parts.push_back(ZombiePart(CONE_DROP_DIRECTORY, HEAD_ZOMBIE_FRAME, row, x_location + 80,
                                          HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT));
    }
}
