#pragma once
#include "draw/rsdl.hpp"
#include "elements/sun/sun.hpp"
#include "../plants.hpp"

class Sunflower : public Plants
{
public:
    Sunflower(const int &_row, const int &_col);
    ~Sunflower();
    void display(const int &_row) override;
    void action() override;
    void determine_appearance();
    bool is_gen_sun();
    void gen_sun_from_a_sunflower(vector<Sun> &suns);
};