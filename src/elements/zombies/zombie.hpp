#pragma once
#include "zombie_type.hpp"

bool has_zombie_reached_block(const Zombie &zombie, const int &row, const int &col);
bool has_zombie_reached_plant(Zombie &zombie);
bool has_zombie_reached_any_plant(Zombie &zombie);
bool can_zombie_move(Zombie &zombie);
void update_moving_status_for_zombies(vector<Zombie *> &zombies);
void move_zombies(vector<Zombie *> &zombies);
void display_zombies(vector<Zombie *> &zombies, const int &_row);
void display_zombie_parts(vector<ZombiePart> &zombie_parts, const int &_row);
