#pragma once
#include "elements/zombies/wave.hpp"
#include "nlohmann/json.hpp"
#include <string>
#include <vector>
using namespace nlohmann;
using namespace std;

#define LEVELS_DIRECTORY "./resources/levels/"
#define LEVEL_COUNT 50

// Seed packets the player can carry into a level (seed chooser slot count).
const int SEED_SLOT_COUNT = 6;

// Path of a level's JSON file (resources/levels/level<N>.json).
string level_file_path(int lv_id);
json get_lv_json(int lv_id);
/*Map index (offset after BACKGROUND_LV1_DIRECTORY) of a level JSON
"background" name; -1 for an unknown name.*/
int background_index_of(const string &bg_name);
bool is_belt_mode(const string &mode);
/*Zombie types declared in the level JSON roster (normal/cone/... ).
Used by the seed chooser's credit-style roster preview.*/
vector<int> get_level_roster_types();
/*Strongest zombie declared in a level's JSON roster by "cost" (ties -> first
declared, ZOMBOSS outranking a tie), for the level-selector cell fallback
when nothing debuts there; -1 when the roster declares nothing.*/
int level_roster_strongest_zombie(int level_num);
/*Schema gate of read_level: collects every problem of the level JSON,
logs them in one file-named message via SDL_Log and throws runtime_error
carrying the full list. Also usable offline to sweep resources/levels/.*/
void validate_level(const json &j, const string &path);
void read_level();
void reset_level();
void load_level();

/*All plant types the player has unlocked so far (keyed off
player.unlocked_level so replays of old levels still offer every earned
plant, as in the original).*/
vector<int> unlocked_plant_types();

/*Apply the seed chooser's pick: remember it so a restart of the same level
keeps these seeds, and rebuild the packet bar to exactly them.*/
void set_level_seeds(const vector<int> &seeds);
