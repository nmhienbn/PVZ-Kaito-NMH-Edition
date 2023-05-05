#include "normal_zombie.hpp"

NormalZombie::~NormalZombie()
{
}

void NormalZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = *ZOMBIE_HEALTH_LIMIT[NORMAL_TYPE].rbegin();
    directory_num = ZOMBIE_WALK_DIRECTORY;
}

void NormalZombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - dir_height);
    directory_num = rand(ZOMBIE_CREDIT1_DIRECTORY, ZOMBIE_CREDIT2_DIRECTORY);
}
