#pragma once
#include "../plants.hpp"

// Day dozers: mushrooms play anim_sleep and never act while the sun is up;
// a Coffee Bean dropped on one wakes it for good.
bool is_day_sleepy_mushroom(const int &type);

/*
Coffee Bean: planted onto a sleeping mushroom (day maps). It hovers over the
host for a beat, plays its crumble, sets the host's coffee_awake flag and
disappears -- the host then runs its night routine in daylight.
*/
class CoffeeBean : public Plants
{
public:
    CoffeeBean(const int &_row, const int &_col);
    ~CoffeeBean();
    void display(const int &_row) override;
    void action() override;

private:
    int wake_clk = 0; // ticks until the wake-up lands
};
