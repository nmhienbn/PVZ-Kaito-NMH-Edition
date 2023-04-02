#pragma once
#include "player_data.h"
#include "music.h"

Map create_a_collection_of_blocks();

void init_game();
bool has_player_lost();
bool has_player_won();

void display_starting_screen();