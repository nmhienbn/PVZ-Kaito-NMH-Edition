#pragma once
#include "makeLevels.h"
#include "render_elements.h"
#include "render_message.h"
#include "elements_actions.h"
#include "player_click.h"
#include "music.h"
#include "game_pause.h"

void start_level_3(window &win, Player &player, Icons &icons, Map &cells,
                   Level &level, Elements &game_characters, int &clk, bool &quit,
                   bool &is_game_started, bool &is_level_chosen, bool &is_paused);