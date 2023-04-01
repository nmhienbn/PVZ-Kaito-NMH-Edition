#pragma once
#include "makeLevels.h"
#include "button.h"
#include "render_elements.h"
#include "render_message.h"
#include "elements_actions.h"
#include "player_click.h"
#include "music.h"
#include "game_pause.h"
#include "restart_menu.h"
#include "leave_menu.h"

void start_level_3(window &win, Player &player, Icons &icons, Map &cells,
                   Level &level, Elements &game_characters, int &clk, bool &quit,
                   bool &is_game_started, bool &is_level_chosen, bool &is_paused);

bool display_win(window &win, Player &player, Icons &icons, Map &cells,
                 Level &level, Elements &game_characters, bool &quit, bool &is_level_chosen);
void display_lose(window &win, Player &player, Icons &icons, Map &cells,
                  Level &level, Elements &game_characters, bool &quit, bool &is_level_chosen);
void display_R_S_P(window &win, Player &player, Level &level,
                   int &clk, bool &is_game_started);

void display_all_in_game(window &win, Player &player, Icons &icons, Map &cells,
                         Level &level, Elements &game_characters, int &clk);

void unlock_plant(window &win, bool &quit, const int &_plant_type);