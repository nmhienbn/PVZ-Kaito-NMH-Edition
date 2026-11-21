#pragma once
#include "../plants.hpp"
#include "elements/sun/sun.hpp"

class SunShroom : public Plants
{
public:
    SunShroom(const int &_row, const int &_col);
    ~SunShroom();
    bool is_night() const;
    void gen_sun(vector<Sun> &suns);
    void grow_step(); // growth timer + grow-up segment handoff (night only)
    void display(const int &_row) override;
    void action() override;

private:
    int grow_clock;     // ticks until the little shroom grows up (PvZ1: 120 s)
    int grow_anim_left; // countdown while the grow-up segment plays
    bool is_grown = false;
    void determine_appearance();
};
