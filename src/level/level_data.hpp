#pragma once
#include "elements/elements.hpp"
#include "elements/mower/mower.hpp"
#define LEVELS_DIRECTORY "./levels/"

void read_level();
void reset_level();
void load_level();
void decide_zombie_each_wave(vector<Wave> &waves, json &jwaves);