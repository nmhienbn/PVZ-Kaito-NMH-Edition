#pragma once
#include "elements.h"
#include "rsdl.hpp"
#include "music.h"

void display_cherrybombs(window &win, vector<CherryBomb> &cherrybombs, Map &cells, bool is_pause = false);
bool is_cherrybomb_hit_zombie(CherryBomb &cherrybomb, Zombie &zombie, Map &cells);
void handle_cherrybomb_zombie_encounter(Elements &elements, Map &cells);
bool apply_cherrybomb_hitting_zombie(Elements &elements, CherryBomb &cherrybomb, int z_ind, Map &cells);