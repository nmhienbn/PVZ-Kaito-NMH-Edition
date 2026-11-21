#pragma once
#include "../plants.hpp"

class GraveBuster : public Plants
{
public:
    GraveBuster(const int &_row, const int &_col);
    ~GraveBuster();
    void display(const int &_row) override;
    void action() override;

private:
    int landing_left; // ticks of the land segment before chewing starts
    int eat_left;     // 240 ticks of chewing remove the grave
};
