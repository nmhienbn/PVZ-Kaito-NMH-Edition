#pragma once
#include "elements.h"
#include "rsdl.hpp"
#include "music.h"

void fire_peas(Elements &elements, Map &cells);
bool are_there_zombies_in_peashooter_row(Peashooter &peashooter, Elements &elements, Map &cells);
void display_peashooters(window &win, vector<Peashooter> &peashooters, Map &cells, bool is_pause = false);