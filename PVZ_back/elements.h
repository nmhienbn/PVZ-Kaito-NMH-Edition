/*
This file include game elements
*/
#pragma once
#include "gameInfo.h"

#define LEVEL_1_X1 15
#define LEVEL_1_X2 240
#define LEVEL_1_Y1 40
#define LEVEL_1_Y2 205

/*--------------------------------------------------------------------
Plants and zombies
--------------------------------------------------------------------*/
struct Sun
{
    int final_col, final_row;
    int x_location, y_location;
    int wait_seconds;
    bool is_clicked = false;
};

struct Sunflower
{
    int row, col;
    int bite;
    int sec_for_another_sun = SUN_GEN_SUNFLOWER_CLK_COUNT;
    int frame = 0;
    int is_attacked = 0;
};

struct Walnut
{
    int row, col;
    int bite;
    int directory_num = WALNUT_1_DIRECTORY;
    int blink_directory_num = WALNUT_1_BLINK_DIRECTORY;
    int frame = 0;
    int is_attacked = 0;
};

struct Peashooter
{
    int row, col;
    int bite;
    int sec_for_another_pea = 1;
    int directory_num = PEASHOOTER_SHEET_DIRECTORY;
    int blink_directory_num = PEASHOOTER_SHEET_BLINK_DIRECTORY;
    int frame = 0;
    int is_attacked = 0;
};

struct Pea
{
    int row;
    int x_location;
    int directory_num = PEA_DIRECTORY;
    int explode = PEA_EXPLODE_TIME;
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
    int directory_num = ZOMBIE_SHEET_DIRECTORY;
    int blink_directory_num = ZOMBIE_BLINK_SHEET_DIRECTORY;
    int n_sheet = ZOMBIE_N_SHEET;
    int c_sheet = ZOMBIE_C_SHEET;
    int frame = 0;
    int is_attacked = 0;

    void change_zombie_eating_status();
};
struct DeadZombie
{
    int row;
    int x_location;
    int body = ZOMBIE_DIE_DIRECTORY;
    int head = ZOMBIE_HEAD_DIRECTORY;
    int frame = 0;
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
    vector<DeadZombie> dead_zombies;
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
    bool is_choosing_a_plant, is_shoveling;
    int unlocked_level;
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

struct Button
{
    int x1, x2, y1, y2;
    Button();
    Button(int _x1, int _x2, int _y1, int _y2);
    ~Button();
    bool is_mouse_in(int mouse_x, int mouse_y) const;
};

const Button Shovel(SHOVEL_X1, SHOVEL_X2, SHOVEL_Y1, SHOVEL_Y2);
const Button TAP_TO_START(TAP_TO_START_X1, TAP_TO_START_X2, TAP_TO_START_Y1, TAP_TO_START_Y2);
const Button LEVEL_1 = Button(15, 240, 40, 205);
const Button LEVEL_2 = Button(260, 485, 40, 205);
const Button LEVEL_3 = Button(505, 730, 40, 205);