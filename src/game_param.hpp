#pragma once
#include <fstream>
#include <iostream>
#include <set>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <vector>
using namespace std;

#define WINDOW_WIDTH 1050
#define WINDOW_HEIGHT 600

enum PlantType
{
    SUNFLOWER_TROPHY = -1,
    PEASHOOTER_TYPE,
    SUNFLOWER_TYPE,
    WALLNUT_TYPE,
    SNOWPEA_TYPE,
    POTATOMINE_TYPE,
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

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

// Playground elements size
extern const int BLOCK_WIDTH;
extern const int BLOCK_HEIGHT;
extern const int HORIZ_BLOCK_COUNT;
extern const int VERT_BLOCK_COUNT;

extern const int X_UPPER_LEFT;
extern const int Y_UPPER_LEFT;

extern const int ELEMENT_WIDTH;
extern const int ELEMENT_HEIGHT;

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Speed properties: increase or decrease the following 9 define values to speed up or speed down entities in the game
extern const int FPS;
// ticks per frame
extern const int TICKS_PER_FRAME;
// Sun from sky frequency
extern const int SUN_GEN_SKY_CLK_COUNT;
// Sun from sunflower frequency
extern const int SUN_GEN_SUNFLOWER_CLK_COUNT;
// Time appear of first wave of zombies
extern const int FIRST_WAVE_CLK_COUNT;
// Time between zombies' different groups
extern const int ZOMBIE_CREATE_CLK_COUNT;
// Zombie bite plant frequency
extern const int BITE_CLK_COUNT;
// Zombie moving frequency
extern const int ZOMBIE_CLK_COUNT;

// Announcer appear time
extern const int ANNOUNCER_CLK_COUNT;
// Sun change color frequency
extern const int SUN_CHANGE_COLOR_CLK_COUNT;
// Max time blink when attack
extern const int MAX_TIME_BLINK;
// Max time being cold (10s)
extern const int MAX_COLD_TIME;
// Max time that a sun can appear
extern const int SUN_APPEAR_LIMIT;

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Initial sun count and normal sun value
extern const int INIT_SUN_COUNT;
extern const int NORMAL_SUN_VALUE;
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

extern const int INF;

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

/*
Moving speed of some elements
*/
extern const int ZOMBIE_INIT_X;
extern const int ZOMBIE_DX;
extern const int ZOMBIE_COLD_DX;
extern const int PEA_DX;
extern const int SUN_DY;
extern const int CLICKED_SUN_D;

extern const vector<int> PLANT_SUN_COST;

extern const vector<int> PLANT_LOADING_TIME;

extern const vector<int> PLANT_HEALTH_LIMIT;

extern const vector<set<int>> ZOMBIE_HEALTH_LIMIT;