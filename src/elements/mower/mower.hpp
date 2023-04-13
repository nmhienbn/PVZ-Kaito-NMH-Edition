#pragma once
#include "draw/rsdl.hpp"
#include "elements/Map/Map.hpp"
#include "zombies/zombie_struct.hpp"

enum Mower_status
{
    MOWER_INACTIVE,
    MOWER_ACTIVE,
};

struct Mower
{
    int x, row;
    int frame;
    Mower_status status;

    Mower();
    Mower(int _x, int _row, int _frame, Mower_status _status);
};

void init_mower(int l_row, int r_row);
void display_mower(const int &_row);
void handle_mower_zombie_encounter(vector<Zombie> &zombies,
                                   vector<DeadZombie> &dead_zombies);
bool apply_mower_hitting_zombie(vector<Zombie> &zombies, const int &z_ind,
                                Mower &mower,
                                vector<DeadZombie> &dead_zombies);
bool is_mower_hit_zombie(Mower &mower, Zombie &zombie);
bool active_mower(int row);