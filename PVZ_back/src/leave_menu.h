#pragma once
#include "rsdl.hpp"
#include "render_elements.h"
#include "makeLevels.h"

void display_game_leave(window &win, Elements &elements, Map &cells, Level &level);
void leave_game(Elements &elements, Icons &icons, Map &cells,
                bool &is_paused, bool &is_game_started, bool &is_level_chosen);

void handle_leave_menu_click(Player &player, Elements &elements, Level &level, Icons &icons, Map &cells,
                             const int &mouse_x, const int &mouse_y, int &clk,
                             bool &is_paused, bool &is_game_started, bool &is_level_chosen);