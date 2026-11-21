#pragma once
#include "../plants.hpp"
#include "elements/sun/sun.hpp"

class Sunflower : public Plants
{
public:
    Sunflower(const int &_row, const int &_col);
    ~Sunflower();
    void display(const int &_row) override;
    void action() override;
    void determine_appearance();
    void gen_sun_from_a_sunflower(vector<Sun> &suns);
};