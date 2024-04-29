#pragma once
#include "elements/announcer/announcer.hpp"
#include "elements/zombies/wave.hpp"

enum ZOMBIE_DELAY_STATE
{
    NO_DELAY = 1,
    HUGE_WAVE = 2,
    FINAL_WAVE = 4,
    AFTER_ANNOUNCE = 8
};

/*
Information about level:
*/
struct Level
{
    int level_num;         // level num
    int map_type;          // type of map: day, night
    bool has_sun_from_sky; // true if has sun from the sky

    int wave_count;      // number of wave
    int cur_wave;        // current wave
    int final_wave;      // final wave
    bool waves_finished; // true if waves has finished

    bool zombie_has_coming; // true if any zombie has appeared
    int zombie_count;       // total level zombies
    int used_zombie_count;  // appeared zombies

    int next_wave_clk; // next wave clk

    int background_directory;

    Announcer announcer;

    ZOMBIE_DELAY_STATE zombie_delay_state;

    vector<Wave> waves;
    vector<int> count_zombie_type;

    Level();
    void reset();
};