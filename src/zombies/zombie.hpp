#pragma once
#include "game_stats.hpp"
#include "music/music.hpp"
#include "draw/rsdl.hpp"
#include "zombies/zombie_type.hpp"
#include "plants/plant_type.hpp"

bool has_zombie_reached_element(const Zombie &zombie, const int &row, const int &col);
bool has_zombie_reached_plant(Zombie &zombie);
bool has_zombie_reached_any_plant(Zombie &zombie);
bool can_zombie_move(Zombie &zombie);
void update_moving_status_for_zombies(vector<Zombie *> &zombies);
void move_zombies(vector<Zombie *> &zombies);
void display_zombies(vector<Zombie *> &zombies, const int &_row);
void display_zombie_parts(vector<ZombiePart> &zombie_parts, const int &_row);

//------------------------------Zombie bite--------------------------------------
void update_zombie_next_bite(vector<Zombie *> &zombies);
bool apply_zombie_bite_on_plant(Zombie &zombie, vector<Plants *> &plants, int &p_ind);
void handle_zombie_plant_encounter(vector<Zombie *> &zombies, vector<Plants *> &plants);

//------------------------------Zombie all status--------------------------------------
void update_zombies_status(vector<Zombie *> &zombies, vector<Plants *> &plants);