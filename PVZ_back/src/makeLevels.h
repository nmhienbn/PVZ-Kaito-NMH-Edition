#pragma once
#include "render_elements.h"
#include "player_name.h"
#include "music.h"

Map create_a_collection_of_blocks();
void read_savedata(Player &player, Level &level);
void update_unlocked_level(Player &player, Level &level);
void reset_unlocked_level(Player &player);

void read_level(Level &level);
void convert_wave_cnt_str_into_int(Level &level, const string &wave_cnt);
void convert_wave_dur_str_into_int_vect(Level &level, const string &wave_dur);
void convert_zombie_seq_str_into_int_vect(Level &level, const string &zombie_seq, const int &typ);

void init_game(window &win, Level &level, Player &player, Map &cells);
void decide_zombie_cnt_for_each_sec(Level &level);
bool has_player_lost(Elements &elements);
bool has_player_won(Level &level, Elements &elements);

void display_starting_screen(window &win);
void display_choosing_level_screen(window &win, Level &level, Player &player, bool &level_chosen, bool &quit);
void load_level(Player &player, Level &level);
void reset_level(Elements &elements, Map &cells, Icons &icons);