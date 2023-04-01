#pragma once
#include "makeLevels.h"
#include "render_elements.h"
#include "elements_actions.h"
#include "player_click.h"
#include "music.h"

void display_game_pause(window &win, Elements &elements, Map &cells, Level &level);

void handle_menu_click(Player &player, Elements &elements, Level &level, Icons &icons, Map &cells,
                       const int &mouse_x, const int &mouse_y, int &clk,
                       bool &is_paused, bool &is_game_started, bool &is_level_chosen);

void handle_menu_icon_click(const int &mouse_x, const int &mouse_y, bool &is_paused);

void unpause_game(bool &is_paused);

void leave_game(Elements &elements, Icons &icons, Map &cells,
                bool &is_paused, bool &is_game_started, bool &is_level_chosen);