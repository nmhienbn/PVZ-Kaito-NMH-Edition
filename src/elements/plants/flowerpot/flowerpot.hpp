#pragma once
#include "../plants.hpp"

/*
Flower Pot (roof): the support plant of roof levels, mirroring the Lily Pad
in the pool -- a regular plant can be planted on top of it, and zombies chew
through the rider first (see support_plant.hpp: FLOWERPOT_TYPE counts as a
support plant there).
*/
class FlowerPot : public Plants
{
public:
    FlowerPot(const int &_row, const int &_col);
    ~FlowerPot();
    void display(const int &_row) override;
    void action() override;
};
