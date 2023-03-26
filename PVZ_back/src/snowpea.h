#pragma once
#include "elements.h"
#include "rsdl.hpp"
#include "music.h"
#include "pea.h"

void fire_snowz_peas(Elements &elements, Map &cells);
bool are_there_zombies_in_snowpea_row(Snowpea &snowpea, Elements &elements, Map &cells);
void display_snowpeas(window &win, vector<Snowpea> &snowpeas, Map &cells, bool is_pause = false);