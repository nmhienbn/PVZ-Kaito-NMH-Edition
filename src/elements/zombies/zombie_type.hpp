#pragma once
#include "zombie_struct.hpp"
// Concrete subclass headers are NOT pulled in here: the type -> class table
// lives in zombie_factory.hpp and only zombie_type.cpp needs the classes.

// First wave (0-based) on which water rows open up to every water-capable
// zombie type; before it, water rows spawn pool-only types alone (repvz
// Board::RowCanHaveZombieType's mCurrentWave < 5 gate, Board.cpp:2638-2646).
const int FIRST_OPEN_POOL_WAVE = 5;

Zombie *init_zombie(int _type, int level_num);
Zombie *init_zombie(int _type, int level_num, const int &x_bias);
// Wave-aware form: materialize_wave_plans passes the 0-based wave index so
// the early-wave pool restriction applies to the row roll.
Zombie *init_zombie(int _type, int level_num, const int &x_bias, int wave_idx);

/*Lanes a zombie type may enter on this board (water lanes restricted).
wave_idx is the 0-based wave the zombie spawns with; wave-agnostic spawn
sites (ambush, vase, boss drop, credit) use the default -- no early-wave
restriction.*/
vector<int> lanes_for_zombie(int type, int wave_idx = FIRST_OPEN_POOL_WAVE);
// Water-lane tube wrap for spawn sites that assign the row after creation.
void wrap_in_duckytube(Zombie *z);