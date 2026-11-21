#pragma once
#include "../plants.hpp"
#include "elements/bullets/pea/pea.hpp"

class Repeater : public Plants
{
public:
    Repeater(const int &_row, const int &_col);
    ~Repeater();
    void determine_appearance(bool check_zombie_in_attack_range);
    void fire_pea(vector<Bullet *> &bullets);
    void next_frame(const PlantAssets &assets) override;
    void display(const int &_row) override;
    void action() override;
};
