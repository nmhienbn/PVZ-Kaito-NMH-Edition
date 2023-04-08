/*
This file contains the game parameter.
Change this to make game more beautiful, faster or easier, harder or easier.
*/
#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <time.h>
using namespace std;

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Speed properties: increase or decrease the following 9 define values to speed up or speed down entities in the game
// CLK meaning : clock
#define FPS 60
#define ticks_per_frame 1000 / FPS

#define SUN_GEN_SKY_CLK_COUNT 1000       // Sun from sky frequency
#define SUN_GEN_SUNFLOWER_CLK_COUNT 2000 // Sun from sunflower frequency
#define ZOMBIE_CREATE_CLK_COUNT 600      // Time between zombies' different groups
#define BITE_CLK_COUNT 50                // Zombie bite plant frequency
#define ZOMBIE_CLK_COUNT 30              // Zombie moving frequency
#define SUN_CLK_COUNT 4                  // Sun moving frequency
#define PEA_CLK_COUNT 2                  // Pea moving frequency
#define ANNOUNCER_CLK_COUNT 180
#define SUN_CHANGE_COLOR_CLK_COUNT 6

#define PEASHOOTER_LOADING 400  // Peashooter loading
#define SUNFLOWER_LOADING 350   // Sunflower loading
#define WALNUT_LOADING 1300     // Walnut loading
#define SNOWPEA_LOADING 400     // SnowPea loading
#define CHERRYBOMB_LOADING 2000 // Cherrybomb loading
#define MAX_TIME_BLINK 5        // time blink when attack
#define MAX_COLD_TIME 600       // time being cold = 10s
#define SUN_APPEAR_LIMIT 300    // Max time that a sun can appear.
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Alter the 4 define values bellow to increase or decrease the price of plants and the initial sun count
#define INIT_SUN_COUNT 1000
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
#define ZOMBIE_DX 6
#define PEA_DX 15
#define SUN_DY 3
#define CLICKED_SUN_D 75
#define FREEZE_ZOMBIE_SLOW_TIMES 1 / 2

/*Health of plants and zombies*/
#define WALNUT_BITE_LIMIT 72
#define SUNFLOWER_BITE_LIMIT 20
#define PEASHOOTER_BITE_LIMIT 20
#define SNOWPEA_BITE_LIMIT 20
#define CHERRYBOMB_BITE_LIMIT 20
#define ZOMBIE_NORMAL_HEALTH_LIMIT 10

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
