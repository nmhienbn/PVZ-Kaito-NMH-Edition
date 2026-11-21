#pragma once
#include "../plants.hpp"

class Torchwood : public Plants
{
public:
    Torchwood(const int &_row, const int &_col);
    ~Torchwood();
    void display(const int &_row) override;
    void action() override;
};
