#pragma once
#include "../plants.hpp"
#include "elements/bullets/pea/pea.hpp"

class Peashooter : public Plants
{
public:
    Peashooter(const int &_row, const int &_col);
    ~Peashooter();
    void determine_appearance(bool check_zombie_in_attack_range);
    void display(const int &_row) override;
    void action() override;
    void fire_pea(vector<Bullet *> &bullets);
};