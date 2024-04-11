#pragma once
#include "elements/mower/mower.hpp"
#include "elements/zombies/zombie_struct.hpp"

void handle_mower_zombie_encounter(vector<Mower *> &mowers, vector<Zombie *> &zombies,
                                   vector<ZombiePart> &zombie_parts);
bool apply_mower_hitting_zombie(vector<Zombie *> &zombies, const int &z_ind,
                                Mower &mower, vector<ZombiePart> &zombie_parts);
bool is_mower_hit_zombie(Mower &mower, Zombie &zombie);