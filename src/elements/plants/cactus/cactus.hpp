#pragma once
#include "../plants.hpp"
#include "elements/bullets/spike/spike.hpp"

class Zombie;

// Cactus: pea-strength spike shooter. Stretches up (anim_rise/idlehigh) when
// a balloon zombie is in its lane so its spikes can reach the flying target;
// otherwise peppers from ground height.
class Cactus : public Plants
{
public:
    Cactus(const int &_row, const int &_col);
    ~Cactus();
    void determine_appearance(bool ground_target, bool balloon_target);
    void fire_spike(vector<Bullet *> &bullets);
    void display(const int &_row) override;
    void action() override;

private:
    bool balloon_in_row(vector<Zombie *> &zombies) const;
};
