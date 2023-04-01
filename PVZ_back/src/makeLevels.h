#pragma once
#include "button.h"
#include "render_elements.h"
#include "render_message.h"
#include "player_name.h"
#include "level_data.h"
#include "music.h"

Map create_a_collection_of_blocks();

void init_game(window &win, Level &level, Player &player, Map &cells);
bool has_player_lost(Elements &elements);
bool has_player_won(Level &level, Elements &elements);

void display_starting_screen(window &win);
void reset_level(Elements &elements, Map &cells, Icons &icons);