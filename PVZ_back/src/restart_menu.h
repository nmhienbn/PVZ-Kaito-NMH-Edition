#pragma once
#include "rsdl.hpp"
#include "render_elements.h"
#include "makeLevels.h"

void display_game_restart(window &win, Elements &elements, Map &cells, Level &level);
void restart_game(Player &player, Elements &elements, Level &level, Icons &icons,
                  Map &cells, int &clk, bool &is_paused, bool &is_game_started);

void handle_restart_menu_click(Player &player, Elements &elements, Level &level, Icons &icons, Map &cells,
                               const int &mouse_x, const int &mouse_y, int &clk,
                               bool &is_paused, bool &is_game_started, bool &is_level_chosen);