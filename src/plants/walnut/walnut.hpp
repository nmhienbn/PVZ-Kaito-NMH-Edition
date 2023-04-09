#pragma once
#include "draw/rsdl.hpp"
#include "game_stats.hpp"
#include "elements/Map/Map.hpp"

struct Walnut
{
    int row, col;
    int bite;
    int directory_num = WALNUT_1_DIRECTORY;
    int frame = 0;
    int is_attacked = 0;
};

void determine_walnut_appearance(Walnut &walnut);
void display_walnuts(vector<Walnut> &walnuts);