#pragma once
#include "../plants.hpp"

class TallNut : public Plants
{
public:
    TallNut(const int &_row, const int &_col);
    ~TallNut();
    void display(const int &_row) override;
    void action() override;
};
