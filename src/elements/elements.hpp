/*
This file include game elements: Plants, Zombies, Player, Level, Icons (Plant seeds)
*/
#pragma once
#include "elements/plants/plant_type.hpp"
#include "elements/zombies/zombie.hpp"

/*--------------------------------------------------------------------
Player and others
--------------------------------------------------------------------*/

/*
Contain vector of all plants and zombies:
@param sun
@param zombie
@param peashooter
@param pea
@param sunflower
@param peashooter
@param walnut
*/
struct Elements
{
    vector<Plants *> plants;

    vector<Zombie *> zombies;
    vector<ZombiePart> zombie_parts;

    vector<Sun> suns;
    vector<Pea> peas;

    void reset();
};

struct Player
{
    string name;
    int sun_count;
    int level;
    bool is_choosing_a_plant, is_shoveling;
    int unlocked_level = 0;

    int sun_count_change_color_time = 0;
    int sun_count_change_color_times = 0;
};

struct Icons
{
    int chosen_plant; // = PLANT_COUNT if no plant is chosen
    int plant_remaining_time[PLANT_COUNT];

    Icons();
};

/*
Information about level:
@param level_num;
@param wave_count;
@param cur_wave;
@param cur_sec;
@param waves_finished;

@param zombie_distr_for_wave(vector<vector<int>>): contains all zombie in all waves.
@param wave_zombie_count(vector<int>): number of zombies in each wave
@param wave_duration(vector<int>): duration of each wave of zombies (in seconds);
*/
struct Level
{
    int level_num; // level num
    int map_type;  // type of map: day, night
    bool is_night; // true if map is night

    int wave_count;      // number of wave
    int cur_wave;        // current wave
    int cur_sec;         // current second
    bool waves_finished; // true if waves has finished

    bool zombie_has_coming;     // true if any zombie has appeared
    int zombie_count;           // total level zombies
    int used_zombie_count;      // appeared zombies
    int last_clk_zombie_appear; // last clk new zombies appeared

    int background_directory = BACKGROUND_DIRECTORY;
    int announce_directory = NULL_DIRECTORY;

    vector<vector<int>> zombie_distr_for_wave[COUNT_ZOMBIE_TYPE];
    vector<int> wave_zombie_count[COUNT_ZOMBIE_TYPE];
    vector<int> wave_duration;

    bool is_huge_wave();
    void reset();
};

Map create_a_collection_of_blocks();
