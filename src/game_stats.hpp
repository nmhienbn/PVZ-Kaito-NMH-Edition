/*
This file contains the game parameter.
Change this to make game more beautiful, faster or easier, harder or easier.
*/
#pragma once
#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <time.h>
using namespace std;

#define INF 1000000000

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Speed properties: increase or decrease the following 9 define values to speed up or speed down entities in the game
// CLK meaning : clock
#define FPS 60
#define ticks_per_frame 1000 / FPS

#define SUN_GEN_SKY_CLK_COUNT 1500       // Sun from sky frequency
#define SUN_GEN_SUNFLOWER_CLK_COUNT 1440 // Sun from sunflower frequency
#define FIRST_WAVE_CLK_COUNT 900         // Time appear of first wave of zombies
#define ZOMBIE_CREATE_CLK_COUNT 2000     // Time between zombies' different groups
#define BITE_CLK_COUNT 30                // Zombie bite plant frequency
#define ZOMBIE_CLK_COUNT 15              // Zombie moving frequency
#define ANNOUNCER_CLK_COUNT 180
#define SUN_CHANGE_COLOR_CLK_COUNT 6

#define PEASHOOTER_LOADING 450  // Peashooter loading
#define SUNFLOWER_LOADING 450   // Sunflower loading
#define WALNUT_LOADING 1800     // Walnut loading
#define SNOWPEA_LOADING 450     // SnowPea loading
#define CHERRYBOMB_LOADING 4500 // Cherrybomb loading
#define MAX_TIME_BLINK 10       // time blink when attack
#define MAX_COLD_TIME 600       // time being cold = 10s
#define SUN_APPEAR_LIMIT 600    // Max time that a sun can appear.
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Alter the 4 define values bellow to increase or decrease the price of plants and the initial sun count
#define INIT_SUN_COUNT 100
#define NORMAL_SUN_VALUE 50
#define PEASHOOTER_PRICE 100
#define SUNFLOWER_PRICE 50
#define WALNUT_PRICE 50
#define SNOWPEA_PRICE 150
#define CHERRYBOMB_PRICE 150
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

/*
Moving speed of some elements
*/
#define ZOMBIE_INIT_X WINDOW_WIDTH
#define ZOMBIE_DX 4
#define ZOMBIE_COLD_DX 2
#define PEA_DX 10
#define SUN_DY 1
#define CLICKED_SUN_D 50

enum PlantType
{
    SUNFLOWER_TROPHY = -1,
    PEASHOOTER_TYPE,
    SUNFLOWER_TYPE,
    WALNUT_TYPE,
    SNOWPEA_TYPE,
    CHERRYBOMB_TYPE,

    PLANT_COUNT
};

enum ZombieType
{
    NORMAL_TYPE,
    FLAG_TYPE,
    CONE_TYPE,
    BUCKET_TYPE,
    DOOR_TYPE,

    COUNT_ZOMBIE_TYPE,
};

/*Health of plants*/
const int PLANT_HEALTH_LIMIT[] = {
    6,   // SUNFLOWER
    6,   // PEASHOOTER
    72,  // WALNUT
    6,   // SNOWPEA
    9999 // CHERRYBOMB

};

/*Health of zombies*/
const set<int> ZOMBIE_HEALTH_LIMIT[] = {
    {5, 10},             // NORMAL
    {3, 6, 9, 12},       // FLAG
    {5, 10, 16, 22, 28}, // CONE
    {5, 10, 25, 45, 65}, // BUCKET
    {5, 10, 25, 45, 65}  // DOOR

};

/*
WINDOW size
*/
#define WINDOW_WIDTH 1050
#define WINDOW_HEIGHT 600

/*Playground elements size*/
#define BLOCK_WIDTH 81
#define BLOCK_HEIGHT 101
#define HORIZ_BLOCK_COUNT 9
#define VERT_BLOCK_COUNT 5

#define X_UPPER_LEFT 250
#define Y_UPPER_LEFT 70

#define ELEMENT_WIDTH 90
#define ELEMENT_HEIGHT 90

enum Game_State
{
    IS_LEVEL_CHOSEN = 1,
    IS_GAME_STARTED = 2,
    IS_PAUSED = 4,
    IS_UNLOCKING_PLANT = 8,
    IS_LEAVE = 16,
    IS_QUIT = 32,
    IS_RESET = 64,
    IS_RESTART = 128,
    IS_FAST = 256

};

inline bool check_status(const int &x, Game_State state)
{
    return ((x & state) > 0);
}

inline void set_status(int &x, Game_State state, const bool &status)
{
    if (status == false)
    {
        if (check_status(x, state))
        {
            x ^= state;
        }
    }
    else
    {
        if (check_status(x, state) == false)
        {
            x |= state;
        }
    }
}