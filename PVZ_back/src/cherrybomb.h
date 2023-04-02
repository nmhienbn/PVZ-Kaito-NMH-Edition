#pragma once
#include "rsdl.hpp"
#include "music.h"
#include "zombie_struct.h"

struct CherryBomb
{
    int row, col;
    int bite;
    int directory_num = CHERRYBOMB_SHEET_DIRECTORY;
    int frame = 0;
    int is_attacked = 0;
};

void display_cherrybombs(vector<CherryBomb> &cherrybombs);
bool is_cherrybomb_hit_zombie(CherryBomb &cherrybomb, Zombie &zombie);
void handle_cherrybomb_zombie_encounter(vector<CherryBomb> &cherrybombs,
                                        vector<Zombie> &zombies,
                                        vector<DeadZombie> &dead_zombies);
bool apply_cherrybomb_hitting_zombie(vector<Zombie> &zombies, const int &z_ind,
                                     CherryBomb &cherrybomb,
                                     vector<DeadZombie> &dead_zombies);