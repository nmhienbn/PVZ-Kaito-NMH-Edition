#include "flag_zombie.hpp"
#include "draw/rsdl.hpp"

FlagZombie::~FlagZombie()
{
}

void FlagZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = *ZOMBIE_HEALTH_LIMIT[FLAG_TYPE].rbegin();
    directory_num = FLAG_ZOMBIE_WALK_1_DIRECTORY;
}

void FlagZombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - dir_height);
    directory_num = rand(ZOMBIE_CREDIT1_DIRECTORY, ZOMBIE_CREDIT2_DIRECTORY);
}
