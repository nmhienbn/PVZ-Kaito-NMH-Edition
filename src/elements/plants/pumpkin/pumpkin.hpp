#pragma once
#include "../plants.hpp"

// Pumpkin: armored shell planted ON TOP of another plant (player_click.cpp
// allows the stacking). High health (plants.json 72); zombies chew the shell
// first (ZombieVsPlant.cpp bites pumpkins before the plant inside).
class Pumpkin : public Plants
{
public:
    Pumpkin(const int &_row, const int &_col);
    ~Pumpkin();
    void display(const int &_row) override;
    void action() override {} // shell only; blocks bites by standing there
};
