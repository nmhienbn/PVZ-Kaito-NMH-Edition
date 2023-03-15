/*
This file include game elements
*/
#pragma once
#include "gameInfo.h"

/*--------------------------------------------------------------------
Plants and zombies
--------------------------------------------------------------------*/
struct Sun
{
    int final_col;
    int final_row;
    int y_location;
    int wait_seconds;
};

struct Sunflower
{
    int row, col;
    int bite;
    int sec_for_another_sun = SUN_GEN_SUNFLOWER_CLK_COUNT;
};

struct Walnut
{
    int row, col;
    int bite;
    string directory;
};

struct Peashooter
{
    int row, col;
    int bite;
    int sec_for_another_pea = 1;
};

struct Pea
{
    int row;
    int x_location;
};

/*
@param row(int): The row of the zombie
@param x_location(int): The begin column of zombie

@param health(int): remaining health of zombie.
@param is_moving(bool): true if zombie is moving, otherwise false
@param directory(string): decide what image to be render.
*/
struct Zombie
{
    int row;
    int x_location;
    int health;
    bool is_moving;
    string directory;
};

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
    vector<Sun> suns;
    vector<Zombie> zombies;
    vector<Peashooter> peashooters;
    vector<Pea> peas;
    vector<Sunflower> sunflowers;
    vector<Walnut> walnuts;
};

// Limited by x1-x2 and y1-y2
struct Block
{
    int x1, x2;
    int y1, y2;
    bool is_planted = false;
};

struct Player
{
    string name;
    int sun_count;
    int level;
    bool is_choosing_a_plant;
};

struct Icons
{
    bool is_sunflower_chosen = 0;
    bool is_peashooter_chosen = 0;
    bool is_walnut_chosen = 0;

    int peashooter_remaining_time = 0;
    int sunflower_remaining_time = 0;
    int walnut_remaining_time = 0;

    string sunflower_icon_directory;
    string peashooter_icon_directory;
    string walnut_icon_directory;
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

    vector<vector<int>> zombie_distr_for_wave;
    vector<int> wave_zombie_count;
    vector<int> wave_duration;
};
// Block[][]: which are limited by x1-x2 and y1-y2
typedef vector<vector<Block>> Map;