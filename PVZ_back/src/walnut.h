#pragma once
#include "rsdl.hpp"
#include "gameInfo.h"
#include "Map.h"

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