#pragma once
#include "draw/rsdl.hpp"
#include "elements/plants/plants.hpp"
#include "elements/zombies/zombie.hpp"
#include "game_param.hpp"
#include "music/music.hpp"
//------------------------------Zombie bite--------------------------------------
void update_zombie_next_bite(vector<Zombie *> &zombies);
bool apply_zombie_bite_on_plant(Zombie &zombie, vector<Plants *> &plants, int &p_ind);
void handle_zombie_plant_encounter(vector<Zombie *> &zombies, vector<Plants *> &plants);

//------------------------------Zombie all status--------------------------------------
void update_zombies_status(vector<Zombie *> &zombies, vector<Plants *> &plants);