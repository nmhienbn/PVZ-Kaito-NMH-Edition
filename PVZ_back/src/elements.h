/*
This file include game elements
*/
#pragma once
#include "gameInfo.h"
#include <random>
#include <chrono>
#include <ctime>
static mt19937 rd(chrono::high_resolution_clock::now().time_since_epoch().count());

int rand(int L, int R);
bool is_in(const int &L, const int &x, const int &R);

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
    int directory_num = SUNFLOWER_SHEET_DIRECTORY;
    int frame = 0;
    int is_attacked = 0;
};

struct Walnut
{
    int row, col;
    int bite;
    int directory_num = WALNUT_1_DIRECTORY;
    int frame = 0;
    int is_attacked = 0;
};

struct Peashooter
{
    int row, col;
    int bite;
    int sec_for_another_pea = 1;
    int directory_num = PEASHOOTER_SHEET_DIRECTORY;
    int frame = 0;
    int is_attacked = 0;
};

struct Snowpea
{
    int row, col;
    int bite;
    int sec_for_another_pea = 1;
    int directory_num = SNOWPEA_SHEET_DIRECTORY;
    int frame = 0;
    int is_attacked = 0;
};

struct Pea
{
    int type;
    int row;
    int x_location;
    int directory_num;
    int explode;

    Pea();
    Pea(int _type, int _row, int _x);
};

struct CherryBomb
{
    int row, col;
    int bite;
    int directory_num = CHERRYBOMB_SHEET_DIRECTORY;
    int frame = 0;
    int is_attacked = 0;
};

enum ZombieType
{
    NORMAL_TYPE,
    CONE_TYPE,
    BUCKET_TYPE,

    COUNT_ZOMBIE_TYPE,
};
/*
@param row(int): The row of the zombie
@param x_location(int): The begin column of zombie
@param health(int): remaining health of zombie.
@param is_moving(bool): true if zombie is moving, otherwise false.
@param directory_num: order of string that is the path to image.
@param blinkdirectory_num: order of string that is the path to blink image.
@param n_sheet: number of sprites.
@param c_sheet: number of column in sprite sheet
@param directory(string): decide what image to be render.
*/
struct Zombie
{
    int type;

    int row, x_location;
    int health;
    bool is_moving;
    int directory_num;
    int frame;
    int is_attacked, cold_time;
    int bite_time;

    Zombie();
    Zombie(int _type);
    Zombie(int _type, int _directory_num);
    Zombie(int _type, int _row, int _x);
    void change_zombie_eating_status();
    void determine_appearance();
    bool decrease_health();
    bool operator<(const Zombie &other) const;
};
struct DeadZombie
{
    int row;
    int x_location;
    int body = ZOMBIE_DIE_DIRECTORY;
    int head = ZOMBIE_HEAD_DIRECTORY;
    int frame = 0;
    bool is_cold = false;

    DeadZombie(int _r, int _x, bool _cold);
    DeadZombie(int _r, int _x, int _body, int _head);
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
    int unlocked_level = 0;
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

    int background_directory = BACKGROUND_DIRECTORY;
    int announce_directory = NULL_DIRECTORY;

    vector<vector<int>> zombie_distr_for_wave[COUNT_ZOMBIE_TYPE];
    vector<int> wave_zombie_count[COUNT_ZOMBIE_TYPE];
    vector<int> wave_duration;

    bool is_huge_wave();
};
// Block[][]: which are limited by x1-x2 and y1-y2
typedef vector<vector<Block>> Map;
