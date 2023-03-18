#pragma once
#include "rsdl.hpp"
#include "gameInfo.h"
#include "elements.h"

Map create_a_collection_of_blocks();
void read_savedata(Player &player, Level &level);
void read_level(Level &level);
void save_wave_cnt_str_as_int(Level &level, string wave_cnt);
void save_zombie_seq_str_as_int_vect(Level &level, string zombie_seq);
void save_wave_dur_str_as_int_vect(Level &level, string wave_dur);
void init_game(window &win, Level &level, Player &player, Map &map);
void decide_zombie_cnt_for_each_sec(Level &level);
void display_starting_screen(window &win);
void display_choosing_level_screen(window &win);
bool user_clicked_on_start(int mouse_x, int mouse_y);
bool has_player_lost(Elements &elements);
bool has_player_won(Level &level, Elements &elements);
