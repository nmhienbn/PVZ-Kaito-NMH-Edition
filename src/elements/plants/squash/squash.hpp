#pragma once
#include "../plants.hpp"

class Zombie;

// Squash states (GOTY: look at the target, jump up, slam down, vanish).
enum SquashState
{
    SQUASH_READY,
    SQUASH_LOOKING,
    SQUASH_JUMPING,
    SQUASH_FALLING,
    SQUASH_LANDED
};

class Squash : public Plants
{
private:
    SquashState state;
    int target_x;      // locked landing spot (screen px) of the chosen zombie
    Zombie *target_zombie; // the zombie locked when the look started

public:
    Squash(const int &_row, const int &_col);
    ~Squash();
    Zombie *find_target();
    /*The locked zombie, if it is still standing in the row (GOTY keeps one
    mTargetZombieID and lets the jump aim at that zombie only).*/
    Zombie *locked_target();
    void do_squash_damage();
    void display(const int &_row) override;
    void action() override;
    void next_frame(const PlantAssets &assets);
};
