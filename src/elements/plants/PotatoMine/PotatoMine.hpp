#pragma once
#include "../plants.hpp"

enum PotatoMineState
{
    PLANTED,
    READY_TO_ARM,
    ARMED,
    ATTACK,
    EXPLOSION
};

class PotatoMine : public Plants
{
private:
    PotatoMineState state;
    int arm_count_down;

public:
    PotatoMine(const int &_row, const int &_col);
    ~PotatoMine();
    void display(const int &_row) override;
    void action() override;
    bool is_blow();
    void disappear();
    void next_frame(const PlantAssets &assets);
    bool is_armed();
};