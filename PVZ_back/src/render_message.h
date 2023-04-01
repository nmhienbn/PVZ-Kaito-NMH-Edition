#pragma once
#include "rsdl.hpp"
#include "render_elements.h"

void display_ready_set_plant(window &win, Player &player, int image_num, Level &level);
void display_game_announce(window &win, Level &level);
void display_winning_message(window &win, Elements &elements, Map &cells, Level &level);
void display_losing_message(window &win, Elements &elements, Map &cells, Level &level);
void display_unlock_plant(window &win, const int &new_plant_dir);