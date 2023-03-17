#pragma once
#include "elements.h"

void gen_random_sun_from_sky(Elements &elements);
void remove_suns(Elements &elements);
void move_suns(vector<Sun> &suns, Map &map);