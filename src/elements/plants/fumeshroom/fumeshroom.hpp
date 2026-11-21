#pragma once
#include "../plants.hpp"
#include "elements/bullets/fume/fume.hpp"

class FumeShroom : public Plants
{
public:
    FumeShroom(const int &_row, const int &_col);
    ~FumeShroom();
    bool is_night() const;
    void fire_fume(vector<Bullet *> &bullets);
    void next_frame(const PlantAssets &assets) override;
    void determine_appearance(bool check_zombie_in_attack_range);
    void display(const int &_row) override;
    void action() override;
};
