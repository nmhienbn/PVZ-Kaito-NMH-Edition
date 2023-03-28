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

    Zombie();
    Zombie(int _type);
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
    vector<Sun> suns;
    vector<Zombie> zombies;
    vector<DeadZombie> dead_zombies;
    vector<Peashooter> peashooters;
    vector<Pea> peas;
    vector<Sunflower> sunflowers;
    vector<Walnut> walnuts;
    vector<Snowpea> snowpeas;
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
    bool is_snowpea_chosen = 0;

    int peashooter_remaining_time = 0;
    int sunflower_remaining_time = 0;
    int walnut_remaining_time = 0;
    int snowpea_remaining_time = 0;

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

struct Button
{
    int x1, x2, y1, y2;
    Button();
    Button(int _x1, int _x2, int _y1, int _y2);
    ~Button();
    bool is_mouse_in(int mouse_x, int mouse_y) const;
};
const Button Shovel(140, 215, 500, 575);
const Button TAP_TO_START(230, 796, 520, 585);

#define LEVEL_COUNT 12
const Button LEVEL_BUTTON[] = {
    {0, 0, 0, 0},         // LV NULL
    {15, 240, 40, 205},   // LV 1
    {260, 485, 40, 205},  // LV 2
    {505, 730, 40, 205},  // LV 3
    {750, 975, 40, 205},  // LV 4
    {15, 240, 230, 395},  // LV 5
    {260, 485, 230, 395}, // LV 6
    {505, 730, 230, 395}, // LV 7
    {750, 975, 230, 395}, // LV 8
    {15, 240, 420, 585},  // LV 9
    {260, 485, 420, 585}, // LV 10
    {505, 730, 420, 585}, // LV 11
    {750, 975, 420, 585}  // LV 12

};

const Button ICON_BAR_LV1(20, 125, 100, 170);
const Button ICON_BAR_LV2(20, 125, 100, 240);
const Button ICON_BAR_LV3(20, 125, 100, 310);
const Button ICON_BAR_LV4(20, 125, 100, 380);
const Button MENU_ICON(850, 975, 0, 35);
const Button MY_GAME(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
#define MENU_X1 325
#define MENU_X2 725
#define MENU_Y1 66
#define MENU_Y2 534
const Button MENU(MENU_X1, MENU_X2, MENU_Y1, MENU_Y2);
const Button BACK_TO_GAME(MENU_X1 + 40, MENU_X1 + 360, MENU_Y1 + 380, MENU_Y1 + 450);
const Button MAIN_MENU(MENU_X1 + 105, MENU_X1 + 300, MENU_Y1 + 317, MENU_Y1 + 355);
const Button RESTART(MENU_X1 + 105, MENU_X1 + 300, MENU_Y1 + 220, MENU_Y1 + 312);
#define CONTINUE_WIDTH 187
#define CONTINUE_HEIGHT 44
const Button CONTINUE((WINDOW_WIDTH - CONTINUE_WIDTH) / 2, (WINDOW_WIDTH + CONTINUE_WIDTH) / 2, 500, 500 + CONTINUE_HEIGHT);
