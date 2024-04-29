#pragma once
#include "elements/zombies/wave.hpp"
#include "nlohmann/json.hpp"
#include <vector>
using namespace nlohmann;
using namespace std;

#define LEVELS_DIRECTORY "./resources/levels/"
#define LEVEL_COUNT 12

json get_lv_json(int lv_id);
void read_level();
void reset_level();
void load_level();
void decide_zombie_each_wave(vector<Wave> &waves, json &jwaves);