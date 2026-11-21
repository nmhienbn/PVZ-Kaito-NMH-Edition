#pragma once
#include "../plants.hpp"
#include "elements/bullets/pea/pea.hpp"

class Zombie;

// Split Pea: front head shoots 1 pea right, rear head shoots 2 peas left
// (digger counter). The reanim renders the two heads + leaves as three
// overlay passes (reanim_plant.cpp).
class SplitPea : public Plants
{
public:
    SplitPea(const int &_row, const int &_col);
    ~SplitPea();
    void determine_appearance(bool front_target, bool back_target);
    void fire_peas(vector<Bullet *> &bullets);
    void display(const int &_row) override;
    void action() override;

protected:
    void next_frame(const PlantAssets &assets) override;

private:
    bool zombie_on_left(vector<Zombie *> &zombies) const;
};
