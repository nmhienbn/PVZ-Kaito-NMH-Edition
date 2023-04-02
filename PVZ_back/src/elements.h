/*
This file include game elements
*/
#pragma once
#include "gameInfo.h"
#include "peashooter.h"
#include "sunflower.h"
#include "walnut.h"
#include "snowpea.h"
#include "cherrybomb.h"
#include "zombie.h"

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
    vector<Peashooter> peashooters;
    vector<Sunflower> sunflowers;
    vector<Walnut> walnuts;
    vector<Snowpea> snowpeas;
    vector<CherryBomb> cherrybombs;

    vector<Zombie> zombies;
    vector<DeadZombie> dead_zombies;

    vector<Sun> suns;
    vector<Pea> peas;
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

enum PlantType
{
    PEASHOOTER,
    SUNFLOWER,
    WALNUT,
    SNOWPEA,
    CHERRYBOMB,

    PLANT_COUNT
};

struct Icons
{
    bool is_plant_chosen[PLANT_COUNT];
    int plant_remaining_time[PLANT_COUNT];

    Icons();
    void reset_is_chosen();
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
    int level_num;
    int wave_count;
    int cur_wave;
    int cur_sec;
    bool waves_finished;
    int map_type;
    bool is_night;
    bool zombie_has_coming;

    int background_directory = BACKGROUND_DIRECTORY;
    int announce_directory = NULL_DIRECTORY;

    vector<vector<int>> zombie_distr_for_wave[COUNT_ZOMBIE_TYPE];
    vector<int> wave_zombie_count[COUNT_ZOMBIE_TYPE];
    vector<int> wave_duration;

    bool is_huge_wave();
};

Map create_a_collection_of_blocks();
