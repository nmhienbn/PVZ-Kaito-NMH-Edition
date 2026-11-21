#pragma once
#include "../plants.hpp"

class Zombie;

enum TangleKelpState
{
    KELP_WAITING,
    KELP_GRABBING
};

class TangleKelp : public Plants
{
private:
    TangleKelpState state;

public:
    TangleKelp(const int &_row, const int &_col);
    ~TangleKelp();
    Zombie *find_target();
    void display(const int &_row) override;
    void action() override;
};
