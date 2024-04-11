#pragma once
#include "../plants.hpp"

class CherryBomb : public Plants
{
public:
    CherryBomb(const int &_row, const int &_col);
    ~CherryBomb();
    void display(const int &_row) override;
    void action() override;
    bool is_blow();
    void disappear();
    void next_frame(const PlantAssets &assets);
};