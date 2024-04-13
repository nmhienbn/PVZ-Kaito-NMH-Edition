#pragma once
#include "../plants.hpp"

class WallNut : public Plants
{
public:
    WallNut(const int &_row, const int &_col);
    ~WallNut();
    void determine_appearance();
    void display(const int &_row) override;
    void action() override;
};
