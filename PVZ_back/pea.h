#pragma once
#include "elements.h"

void handle_pea_zombie_encounter(Elements &elements, Map &map);
bool apply_pea_hitting_zombie(Elements &elements, int p_ind, int z_ind);
bool has_pea_reached_zombie(Pea pea, Zombie zombie);
void move_peas(vector<Pea> &peas, Elements &elements, Map &map);
bool can_pea_move(Pea &pea, Elements &elements, Map &map);