#pragma once
#include "draw/rsdl.hpp"
#include "music/music.hpp"
#include <elements\bullets\pea\pea.hpp>

struct Snowpea
{
    int row, col;
    int bite;
    int sec_for_another_pea = 1;
    int directory_num = SNOWPEA_SHEET_DIRECTORY;
    int frame = 0;
    int is_attacked = 0;
};

void fire_snowz_peas(vector<Snowpea> &snowpeas, vector<Zombie> &zombies, vector<Pea> &peas);
bool are_there_zombies_in_snowpea_row(Snowpea &snowpea, vector<Zombie> &zombies);
void display_snowpeas(vector<Snowpea> &snowpeas);