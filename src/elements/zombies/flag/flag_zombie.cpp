#include "flag_zombie.hpp"
#include "draw/rsdl.hpp"

FlagZombie::~FlagZombie()
{
}

// Flag pace: 0.45 px/cs fixed in the original (repvz Zombie.cpp:1196-1200
// PickRandomSpeed) -> 24.5 px/s under HEAD_SPEED_SCALE, brisk vs the plain
// 12.5-17.5 px/s.
float FlagZombie::speed_px_per_tick() const
{
    return CS_VEL(0.45f) * HEAD_SPEED_SCALE;
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
