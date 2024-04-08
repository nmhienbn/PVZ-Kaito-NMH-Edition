#pragma once
#include "elements/zombies/zombie.hpp"

class Wave
{
private:
public:
    vector<Zombie *> zombies;
    int delay_time;
    bool has_flag;
    void add_zombie(Zombie *zombie);
    size_t zombie_count();
};