#pragma once
#include "../plants.hpp"

class SpikeWeed : public Plants
{
private:
    int hit_clock; // ticks between two stabs (~0.7 s == GOTY's attack cadence)

public:
    SpikeWeed(const int &_row, const int &_col);
    ~SpikeWeed();
    void display(const int &_row) override;
    void action() override;
};
