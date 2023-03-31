#pragma once
#include "elements.h"
#include "music.h"
#include "rsdl.hpp"

void handle_pea_zombie_encounter(Elements &elements, Map &cells);
bool apply_pea_hitting_zombie(Elements &elements, Pea &pea, int z_ind);
bool has_pea_reached_zombie(Pea &pea, Zombie &zombie);
void move_peas(vector<Pea> &peas, Elements &elements, Map &cells);
bool can_pea_move(Pea &pea, Elements &elements, Map &cells);
void display_peas(window &win, vector<Pea> &peas, Map &cells);