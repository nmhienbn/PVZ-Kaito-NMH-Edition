#pragma once
#include "../plants.hpp"
#include "elements/bullets/puff/puff.hpp"

class PuffShroom : public Plants
{
public:
    PuffShroom(const int &_row, const int &_col);
    ~PuffShroom();
    void determine_appearance(bool check_zombie_in_attack_range);
    void fire_puff(vector<Bullet *> &bullets);
    void next_frame(const PlantAssets &assets) override;
    void display(const int &_row) override;
    void action() override;
    bool is_night() const; // night-awake checks; a coffee bean flips coffee_awake
};
