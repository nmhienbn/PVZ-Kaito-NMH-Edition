#pragma once
#include "music.h"
#include "rsdl.hpp"
#include "zombie_struct.h"

struct Pea
{
    int type;
    int row;
    int x_location;
    int directory_num;
    int explode;

    Pea();
    Pea(int _type, int _row, int _x);
};

bool has_pea_reached_zombie(Pea &pea, Zombie &zombie);
void handle_pea_zombie_encounter(vector<Pea> &peas, vector<Zombie> &zombies, vector<DeadZombie> &dead_zombies);
bool apply_pea_hitting_zombie(vector<Zombie> &zombies, vector<DeadZombie> &dead_zombies, Pea &pea, const int &z_ind);
void move_peas(vector<Pea> &peas, vector<Zombie> &zombies);
bool can_pea_move(Pea &pea, vector<Zombie> &zombies);
void display_peas(vector<Pea> &peas);