#pragma once
#include "music/music.hpp"
#include "draw/rsdl.hpp"
#include "zombies/zombie_struct.hpp"

struct Pea
{
    int type;
    int row;
    int x_location;
    int directory_num;
    int explode;

    Pea();
    Pea(int _type, int _row, int _x);

    bool has_reached_zombie(Zombie &zombie);
    bool can_move(vector<Zombie> &zombies);
    bool apply_hitting_zombie(vector<Zombie> &zombies, vector<DeadZombie> &dead_zombies, const int &z_ind);
    void display();
};

void handle_pea_zombie_encounter(vector<Pea> &peas, vector<Zombie> &zombies, vector<DeadZombie> &dead_zombies);
void move_peas(vector<Pea> &peas, vector<Zombie> &zombies);