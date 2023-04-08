#pragma once
#include "draw/rsdl.hpp"
#include "elements/sun/sun.hpp"

struct Sunflower
{
    int row, col;
    int bite;
    int sec_for_another_sun = SUN_GEN_SUNFLOWER_CLK_COUNT;
    int directory_num = SUNFLOWER_SHEET_DIRECTORY;
    int frame = 0;
    int is_attacked = 0;
};

void gen_sun_from_all_sunflowers(vector<Sunflower> &sunflowers, vector<Sun> &suns);
void gen_sun_from_a_sunflower(Sunflower &sunflower, vector<Sun> &suns);
void display_sunflowers(vector<Sunflower> &sunflowers);