#pragma once
#include "rsdl.hpp"
#include "music.h"
#include "pea.h"

struct Peashooter
{
    int row, col;
    int bite;
    int sec_for_another_pea = 1;
    int directory_num = PEASHOOTER_SHEET_DIRECTORY;
    int frame = 0;
    int is_attacked = 0;
};

void fire_peas(vector<Peashooter> &peashooters, vector<Zombie> &zombies, vector<Pea> &peas);
bool are_there_zombies_in_peashooter_row(Peashooter &peashooter, vector<Zombie> &zombies);
void display_peashooters(vector<Peashooter> &peashooters);