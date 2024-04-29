#pragma once
#include "../zombie_struct.hpp"

class NormalZombie : public Zombie
{
public:
    ~NormalZombie();
    void init() override;
    void make_credit() override;
};