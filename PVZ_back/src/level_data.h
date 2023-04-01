#pragma once
#include "elements.h"

void read_level(Level &level);
void convert_wave_cnt_str_into_int(Level &level, const string &wave_cnt);
void convert_wave_dur_str_into_int_vect(Level &level, const string &wave_dur);
void convert_zombie_seq_str_into_int_vect(Level &level, const string &zombie_seq, const int &typ);
void load_level(Player &player, Level &level);
void decide_zombie_cnt_for_each_sec(Level &level);