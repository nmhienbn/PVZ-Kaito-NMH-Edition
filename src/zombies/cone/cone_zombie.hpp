#pragma once
#include "../zombie_struct.hpp"

class ConeZombie : public Zombie
{
public:
    ~ConeZombie();
    void init() override;
    void make_credit() override;
    void armor_drop(vector<ZombiePart> &zombie_parts) override;
    using Zombie::Zombie;
};