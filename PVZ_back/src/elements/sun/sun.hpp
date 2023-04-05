#pragma once
#include "draw/rsdl.hpp"
#include "game_stats.hpp"
#include <elements\Map\Map.hpp>

struct Sun
{
    int final_col, final_row;
    int x_location, y_location;
    int wait_seconds;
    bool is_clicked = false;
};

void gen_random_sun_from_sky(vector<Sun> &suns);
void remove_suns(vector<Sun> &suns);
void move_suns(vector<Sun> &suns);
void display_suns(vector<Sun> &suns);