#include "wave.hpp"

void Wave::add_zombie(Zombie *zombie)
{
    zombies.push_back(zombie);
}

size_t Wave::zombie_count()
{
    return zombies.size();
}