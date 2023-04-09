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

#define SUN_GEN_SKY_CLK_COUNT 1500       // Sun from sky frequency
#define SUN_GEN_SUNFLOWER_CLK_COUNT 1500 // Sun from sunflower frequency
#define ZOMBIE_CREATE_CLK_COUNT 600      // Time between zombies' different groups
#define BITE_CLK_COUNT 30                // Zombie bite plant frequency
#define ZOMBIE_CLK_COUNT 15              // Zombie moving frequency
#define ANNOUNCER_CLK_COUNT 180
#define SUN_CHANGE_COLOR_CLK_COUNT 6

#define PEASHOOTER_LOADING 450  // Peashooter loading
#define SUNFLOWER_LOADING 450   // Sunflower loading
#define WALNUT_LOADING 1800     // Walnut loading
#define SNOWPEA_LOADING 450     // SnowPea loading
#define CHERRYBOMB_LOADING 3000 // Cherrybomb loading
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

/*Health of plants and zombies*/
#define WALNUT_BITE_LIMIT 72
#define SUNFLOWER_BITE_LIMIT 6
#define PEASHOOTER_BITE_LIMIT 6
#define SNOWPEA_BITE_LIMIT 6
#define CHERRYBOMB_BITE_LIMIT 9999

#define ZOMBIE_NORMAL_HEALTH_LIMIT 10
#define ZOMBIE_CONE1_HEALTH_LIMIT 28
#define ZOMBIE_CONE2_HEALTH_LIMIT 22
#define ZOMBIE_CONE3_HEALTH_LIMIT 16
#define ZOMBIE_BUCKET1_HEALTH_LIMIT 65
#define ZOMBIE_BUCKET2_HEALTH_LIMIT 45
#define ZOMBIE_BUCKET3_HEALTH_LIMIT 25

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
