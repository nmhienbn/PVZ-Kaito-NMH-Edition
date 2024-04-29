#include "door_zombie.hpp"
#include "draw/rsdl.hpp"

DoorZombie::~DoorZombie()
{
}

void DoorZombie::init()
{
    dir_width = 196;
    dir_height = 197;
    health = *ZOMBIE_HEALTH_LIMIT[DOOR_TYPE].rbegin();
    directory_num = DOOR_ZOMBIE_WALK_1_DIRECTORY;
}

void DoorZombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - dir_height);
    directory_num = rand(DOOR_ZOMBIE_CREDIT1_DIRECTORY, DOOR_ZOMBIE_CREDIT2_DIRECTORY);
}

void DoorZombie::armor_drop(vector<ZombiePart> &zombie_parts)
{
    if (health == *ZOMBIE_HEALTH_LIMIT[NORMAL_TYPE].rbegin())
    {
        type = NORMAL_TYPE;
        dir_width = 166;
        dir_height = 144;
    }
}
