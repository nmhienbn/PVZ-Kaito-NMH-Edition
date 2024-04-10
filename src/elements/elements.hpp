/*
This file include game elements: Plants, Zombies, Player, Level, Icons (Plant seeds)
*/
#pragma once
#include "elements/announcer/announcer.hpp"
#include "elements/plants/plant_type.hpp"
#include "elements/zombies/wave.hpp"

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
    vector<Bullet *> bullets;

    vector<Zombie *> zombies;
    vector<ZombiePart> zombie_parts;

    vector<Sun> suns;

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

enum ZOMBIE_DELAY_STATE
{
    NO_DELAY = 1,
    HUGE_WAVE = 2,
    FINAL_WAVE = 4,
    AFTER_ANNOUNCE = 8
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
    int level_num;         // level num
    int map_type;          // type of map: day, night
    bool has_sun_from_sky; // true if has sun from the sky

    int wave_count;      // number of wave
    int cur_wave;        // current wave
    int cur_sec;         // current second
    int final_wave;      // final wave
    bool waves_finished; // true if waves has finished

    bool zombie_has_coming; // true if any zombie has appeared
    int zombie_count;       // total level zombies
    int used_zombie_count;  // appeared zombies

    int next_wave_clk; // next wave clk

    int background_directory = BACKGROUND_DIRECTORY;

    Announcer announcer = Announcer(NULL_DIRECTORY, NULL_MUSIC_DIRECTORY);

    ZOMBIE_DELAY_STATE zombie_delay_state;

    vector<Wave> waves;
    vector<int> count_zombie_type;

    void reset();
};

Map create_a_collection_of_blocks();
