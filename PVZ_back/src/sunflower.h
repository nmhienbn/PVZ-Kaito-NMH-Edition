#pragma once
#include "elements.h"
#include "rsdl.hpp"

void gen_sun_from_all_sunflowers(Elements &elements, Map &cells);
void gen_sun_from_a_sunflower(Elements &elements, Sunflower &sunflower, Map &cells);
void display_sunflowers(window &win, vector<Sunflower> &sunflowers, Map &cells, bool is_pause = false);