#pragma once
#include "../plants.hpp"

class Zombie;

enum ChomperState
{
    CHOMPER_READY,
    CHOMPER_BITING,
    CHOMPER_CHEWING,
    CHOMPER_SWALLOWING
};

class Chomper : public Plants
{
private:
    ChomperState state;
    int chew_clock; // ticks left digesting (GOTY 4000 @100fps ~= 40 s; swallow adds ~4 s)
    Zombie *find_target();

public:
    Chomper(const int &_row, const int &_col);
    ~Chomper();
    void display(const int &_row) override;
    void action() override;
    void next_frame(const PlantAssets &assets) override;
};
