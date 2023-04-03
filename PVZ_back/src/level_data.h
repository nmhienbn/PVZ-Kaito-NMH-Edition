#pragma once
#include "elements.h"

void read_level();
void convert_wave_cnt_str_into_int(const string &wave_cnt);
void convert_wave_dur_str_into_int_vect(const string &wave_dur);
void convert_zombie_seq_str_into_int_vect(const string &zombie_seq, const int &typ);
void reset_level();
void load_level();
void decide_zombie_cnt_for_each_sec();