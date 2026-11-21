#pragma once
#include "../plants.hpp"

class LilyPad : public Plants
{
public:
    LilyPad(const int &_row, const int &_col);
    ~LilyPad();
    void display(const int &_row) override;
    void action() override;
};
