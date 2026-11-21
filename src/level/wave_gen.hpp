#pragma once
#include "game_param.hpp"
#include "nlohmann/json.hpp"
#include <ostream>
#include <random>
#include <vector>

using namespace nlohmann;
using namespace std;

/*
Port of Board::PickZombieWaves: zombies of a level are generated from a point
budget per wave instead of static wave lists.
    - wave w (0-based) starts with w/3 + 1 points (w*2/5 + 1 once the
      adventure has been finished -- the replay budget, Board.cpp:642-645;
      level 1-5 keeps the first-run budget).
    - flag waves (every waves_per_flag-th wave) force a parade of
      min(points, 8) normal zombies plus one flag zombie, then get
      a x2.5 point bonus.
    - a per-level point multiplier applies after the flag bonus
      (level 5 x4, level 10 x3).
    - intro_force types are forced once before the random fill: digger and
      balloon arrive on wave 7 (or the final wave), other types on the middle
      wave and the final wave (repvz Board.cpp:736-759).
    - the final wave guarantees at least one zombie of every allowed type.
    - remaining points are spent on weighted-random roster picks.
*/

struct ZombiePick
{
    int type;
    int cost;
    int weight;
    int first_wave; // 1-based wave index where this type may be picked randomly
};

struct WaveGenSpec
{
    int total_waves = 0;
    vector<ZombiePick> roster;
    int intro_force_type = -1; // type forced at its intro wave(s) (digger/balloon:
                               // wave 7 + final; others: middle wave + final)
    int point_multiplier = 1;  // applied to wave points after the flag bonus
    bool flag_waves = true;    // false: no flag parade even on the last wave (level 1)
    // First playthrough budget w/3+1 vs the replay budget w*2/5+1 (the
    // adventure has been finished: IsAdventureMode && HasFinishedAdventure
    // && mLevel != 5, Board.cpp:642-645 -- replaying ramps waves faster).
    bool replay_budget = false;
};

struct WaveSpawn
{
    int type;
    int count;
};

struct WavePlan
{
    bool has_flag = false;
    int points = 0; // point budget of the wave (after bonuses, before fill)
    vector<WaveSpawn> spawns;
};

// zombie type name <-> enum helpers shared by the loader and the dump tool
int zombie_type_from_name(const string &name);
string zombie_type_name(int type);

// Parse the "zombies" object of the level JSON.
WaveGenSpec wave_spec_from_json(const json &jzombies);

// Generate the full wave plan of a level. Pure: no game state needed.
vector<WavePlan> generate_wave_plans(const WaveGenSpec &spec, mt19937 &rng);

// Human-readable per-wave dump (PVZ_DUMP_WAVES / standalone tool).
void dump_wave_plans(ostream &out, int level_num, const string &name, const vector<WavePlan> &plans);
