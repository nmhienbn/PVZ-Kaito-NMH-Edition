#pragma once
#include "../plants.hpp"
#include "elements/bullets/star/star.hpp"

class Zombie;

// Starfruit: volleys 5 stars per volley — backward, straight up, straight
// down, and forward-up/forward-down diagonals (no plain forward star, per the
// original's StarFruitFire). Lane-crossing handled by the Star bullet.
class Starfruit : public Plants
{
public:
    Starfruit(const int &_row, const int &_col);
    ~Starfruit();
    bool find_target(vector<Zombie *> &zombies) const;
    void fire_stars(vector<Bullet *> &bullets);
    void display(const int &_row) override;
    void action() override;
};
