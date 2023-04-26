#pragma once
#include "../zombie_struct.hpp"

class FlagZombie : public Zombie
{
public:
    ~FlagZombie();
    void init() override;
    void make_credit() override;
    using Zombie::Zombie;
};