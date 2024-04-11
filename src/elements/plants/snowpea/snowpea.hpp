#pragma once
#include "../plants.hpp"
#include "elements/bullets/snowpea/snowzpea.hpp"

class Snowpea : public Plants
{
public:
    Snowpea(const int &_row, const int &_col);
    ~Snowpea();
    void determine_appearance(bool check_zombie_in_attack_range);
    void display(const int &_row) override;
    void action() override;
    void fire_pea(vector<Bullet *> &bullets);
};