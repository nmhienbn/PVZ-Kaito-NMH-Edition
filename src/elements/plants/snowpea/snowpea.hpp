#pragma once
#include "../plants.hpp"
#include "elements/bullets/snowpea/snowzpea.hpp"

class SnowPea : public Plants
{
public:
    SnowPea(const int &_row, const int &_col);
    ~SnowPea();
    void determine_appearance(bool check_zombie_in_attack_range);
    void display(const int &_row) override;
    void action() override;
    void fire_pea(vector<Bullet *> &bullets);
};