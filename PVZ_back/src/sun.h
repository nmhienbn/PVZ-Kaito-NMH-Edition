#pragma once
#include "elements.h"
#include "rsdl.hpp"

void gen_random_sun_from_sky(Elements &elements);
void remove_suns(Elements &elements);
void move_suns(vector<Sun> &suns, Map &cells);
void display_suns(window &win, vector<Sun> suns, Map &cells);