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
#define string std::string
#define vector std::vector

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Speed properties: increase or decrease the following 9 define values to speed up or speed down entities in the game
// CLK meaning : clock
#define FPS 40
#define ticks_per_frame 1000 / FPS

#define SUN_GEN_SKY_CLK_COUNT 1000       // Sun from sky frequency
#define SUN_GEN_SUNFLOWER_CLK_COUNT 1000 // Sun from sunflower frequency
#define ZOMBIE_CREATE_CLK_COUNT 600      // Time between zombies' different groups
#define FIRE_PEA_CLK_COUNT 100           // Fire pea refresh frequency
#define BITE_CLK_COUNT 50                // Zombie bite plant frequency
#define ZOMBIE_CLK_COUNT 11              // Zombie moving frequency
#define SUN_CLK_COUNT 4                  // Sun moving frequency
#define PEA_CLK_COUNT 2                  // Pea moving frequency

#define PEASHOOTER_LOADING 250 // Peashooter loading
#define SUNFLOWER_LOADING 250  // Sunflower loading
#define WALNUT_LOADING 1000    // Walnut loading
#define MAX_TIME_BLINK 5
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Alter the 4 define values bellow to increase or decrease the price of plants and the initial sun count
#define INIT_SUN_COUNT 200
#define PEASHOOTER_PRICE 100
#define SUNFLOWER_PRICE 50
#define WALNUT_PRICE 50
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

/*
Moving speed of some elements
*/
#define ZOMBIE_INIT_X WINDOW_WIDTH - 75
#define ZOMBIE_DX 3
#define PEA_DX 15
#define SUN_DY 3
#define CLICKED_SUN_D 100

/*Health of plants and zombies*/
#define WALNUT_BITE_LIMIT 72
#define SUNFLOWER_BITE_LIMIT 20
#define PEASHOOTER_BITE_LIMIT 20
#define ZOMBIE_NORMAL_HEALTH_LIMIT 10

#define DEAD_ZOMBIE_FRAMES 12
#define DEAD_ZOMBIE_WIDTH 166
#define DEAD_ZOMBIE_HEIGHT 144
#define DEAD_ZOMBIE_C_SHEET 3
#define DEAD_ZOMBIE_N_SHEET 10

#define HEAD_ZOMBIE_WIDTH 150
#define HEAD_ZOMBIE_HEIGHT 187
#define HEAD_ZOMBIE_C_SHEET 12
#define HEAD_ZOMBIE_N_SHEET 12
#define HEAD_ZOMBIE_G_WIDTH 125
#define HEAD_ZOMBIE_G_HEIGHT 155

/*Playground elements size*/
#define WINDOW_WIDTH 1050
#define WINDOW_HEIGHT 600
#define BLOCK_WIDTH 81
#define BLOCK_HEIGHT 101
#define HORIZ_BLOCK_COUNT 9
#define VERT_BLOCK_COUNT 5

#define X_UPPER_LEFT 250
#define Y_UPPER_LEFT 70
#define TAP_TO_START_X1 230
#define TAP_TO_START_X2 796
#define TAP_TO_START_Y1 520
#define TAP_TO_START_Y2 585
#define ICON_BAR_X1 20
#define ICON_BAR_X2 125
#define ICON_BAR_Y1 100
#define ICON_BAR_Y2 310

#define INIT_SUN_Y 5
#define SUNFLOWER_ICON_Y1 100
#define PEASHOOTER_ICON_Y1 170
#define WALNUT_ICON_Y1 240
#define SUN_BAR_WIDTH 180
#define SUN_BAR_HEIGHT 360

#define ICON_BAR_WIDTH 120
#define ICON_BAR_HEIGHT 194
#define ICON_WIDTH 105
#define ICON_HEIGHT 70
#define SUN_COUNT_WIDTH 70
#define SUN_COUNT_HEIGHT 30
#define SUN_WIDTH 80
#define SUN_HEIGHT 80

#define PEA_WIDTH 25
#define PEA_HEIGHT 25
#define PEA_EXPLODE_TIME 35

#define PEASHOOTER_WIDTH 185
#define PEASHOOTER_HEIGHT 157
#define PEASHOOTER_G_WIDTH 200
#define PEASHOOTER_G_HEIGHT 157 //**
#define PEASHOOTER_N_SHEET 60
#define PEASHOOTER_C_SHEET 8

#define SUNFLOWER_WIDTH 100
#define SUNFLOWER_HEIGHT 108
#define SUNFLOWER_SHEET 27
#define SUNFLOWER_R_SHEET 5
#define SUNFLOWER_C_SHEET 6

#define WALNUT_WIDTH 185
#define WALNUT_HEIGHT 185
#define WALNUT_G_WIDTH 200
#define WALNUT_G_HEIGHT 200

#define ZOMBIE_WIDTH 92
#define ZOMBIE_HEIGHT 126

#define ZOMBIE_FRAME 6
#define ZOMBIE_C_SHEET 6
#define ZOMBIE_N_SHEET 22

#define ZOMBIE_G_WIDTH 92
#define ZOMBIE_G_HEIGHT 126

#define ZOMBIE_EATING_C_SHEET 7
#define ZOMBIE_EATING_N_SHEET 21

#define ELEMENT_WIDTH 90
#define ELEMENT_HEIGHT 90
#define TILE_WIDTH 75
#define READY_WIDTH 300
#define READY_HEIGHT 133

#define SHOVEL_X1 140
#define SHOVEL_X2 215
#define SHOVEL_Y1 500
#define SHOVEL_Y2 575
#define SHOVEL_WIDTH 139
#define SHOVEL_HEIGHT 141

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Directories
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

enum
{
    PEASHOOTER_DIRECTORY,
    PEASHOOTER_SHEET_DIRECTORY,
    PEASHOOTER_SHEET_BLINK_DIRECTORY,
    PEASHOOTER_ATTACK_DIRECTORY,
    PEASHOOTER_ATTACK_BLINK_DIRECTORY,
    PEA_DIRECTORY,
    PEA_EXPLODE_DIRECTORY,
    SUNFLOWER_DIRECTORY,
    SUNFLOWER_SHEET_DIRECTORY,
    SUNFLOWER_SHEET_BLINK_DIRECTORY,
    SUN_DIRECTORY,
    SUN_BAR_DIRECTORY,
    ICON_BAR_DIRECTORY,
    PEASHOOTER_ICON_BRIGHT_DIRECTORY,
    PEASHOOTER_ICON_DIM_DIRECTORY,
    SUNFLOWER_ICON_BRIGHT_DIRECTORY,
    SUNFLOWER_ICON_DIM_DIRECTORY,
    WALNUT_ICON_BRIGHT_DIRECTORY,
    WALNUT_ICON_DIM_DIRECTORY,
    ZOMBIE_HEALTHY_DIRECTORY,
    ZOMBIE_SHEET_DIRECTORY,
    ZOMBIE_BLINK_SHEET_DIRECTORY,
    ZOMBIE_INJURED_DIRECTORY,
    STARTING_SCREEN_DIRECTORY,
    CHOOSE_LEVELS_DIRECTORY,
    BACKGROUND_DIRECTORY,
    BACKGROUND_LV1_DIRECTORY,
    BACKGROUND_LV2_DIRECTORY,
    LOSING_MESSAGE_DIRECTORY,
    WINNING_MESSAGE_DIRECTORY,
    BLACK_SCREEN_DIRECTORY,
    WALNUT_DIRECTORY,
    WALNUT_1_DIRECTORY,
    WALNUT_2_DIRECTORY,
    WALNUT_3_DIRECTORY,
    WALNUT_4_DIRECTORY,
    WALNUT_1_BLINK_DIRECTORY,
    WALNUT_2_BLINK_DIRECTORY,
    WALNUT_3_BLINK_DIRECTORY,
    WALNUT_4_BLINK_DIRECTORY,
    ZOMBIE_DIE_DIRECTORY,
    ZOMBIE_HEAD_DIRECTORY,
    ZOMBIE_EATING_DIRECTORY,
    ZOMBIE_EATING_BLINK_DIRECTORY,
    START_READY_DIRECTORY,
    START_SET_DIRECTORY,
    START_PLANT_DIRECTORY,
    SHOVEL_BAR_DIRECTORY,
    SHOVEL_DIRECTORY,

    COUNT_USED_DIRECTORY
};

const string image_directory[] = {
    "./Image_Assets/Peashooter.png",              // PEASHOOTER_DIRECTORY
    "./Image_Assets/peashooter_sheet.png",        // PEASHOOTER_SHEET_DIRECTORY
    "./Image_Assets/peashooter_sheet_blink.png",  // PEASHOOTER_SHEET_BLINK_DIRECTORY
    "./Image_Assets/peashooter_attack.png",       // PEASHOOTER_ATTACK_DIRECTORY
    "./Image_Assets/peashooter_attack_blink.png", // PEASHOOTER_ATTACK_BLINK_DIRECTORY
    "./Image_Assets/pea.png",                     // PEA_DIRECTORY
    "./Image_Assets/pea_explode.png",             // PEA_EXPLODE_DIRECTORY
    "./Image_Assets/Sunflower.png",               // SUNFLOWER_DIRECTORY
    "./Image_Assets/sunflower_sheet.png",         // SUNFLOWER_SHEET_DIRECTORY
    "./Image_Assets/sunflower_sheet_blink.png",   // SUNFLOWER_SHEET_BLINK_DIRECTORY
    "./Image_Assets/sun.png",                     // SUN_DIRECTORY
    "./Image_Assets/sun_bar2.png",                // SUN_BAR_DIRECTORY
    "./Image_Assets/Item_Bar.png",                // ICON_BAR_DIRECTORY
    "./Image_Assets/icon_peashooter_100.png",     // PEASHOOTER_ICON_BRIGHT_DIRECTORY
    "./Image_Assets/icon_peashooter_100_dim.png", // PEASHOOTER_ICON_DIM_DIRECTORY
    "./Image_Assets/icon_sunflower.png",          // SUNFLOWER_ICON_BRIGHT_DIRECTORY
    "./Image_Assets/icon_sunflower_dim.png",      // SUNFLOWER_ICON_DIM_DIRECTORY
    "./Image_Assets/icon_walnut.png",             // WALNUT_ICON_BRIGHT_DIRECTORY
    "./Image_Assets/icon_walnut_dim.png",         // WALNUT_ICON_DIM_DIRECTORY
    "./Image_Assets/Zombie_healthy.png",          // ZOMBIE_HEALTHY_DIRECTORY
    "./Image_Assets/ZOMBIEWALK1.png",             // ZOMBIE_SHEET_DIRECTORY
    "./Image_Assets/ZOMBIEWALK1_blink.png",       // ZOMBIE_BLINK_SHEET_DIRECTORY
    "./Image_Assets/Zombie_Injured.png",          // ZOMBIE_INJURED_DIRECTORY
    "./Image_Assets/Starting_Screen.png",         // STARTING_SCREEN_DIRECTORY
    "./Image_Assets/Choose_levels.png",           // CHOOSE_LEVELS_DIRECTORY
    "./Image_Assets/Frontyard.png",               // BACKGROUND_DIRECTORY
    "./Image_Assets/Frontyard_LV1.png",           // BACKGROUND_LV1_DIRECTORY
    "./Image_Assets/Frontyard_LV2.png",           // BACKGROUND_LV2_DIRECTORY
    "./Image_Assets/ZombiesWon.png",              // LOSING_MESSAGE_DIRECTORY
    "./Image_Assets/Winning_Pic.png",             // WINNING_MESSAGE_DIRECTORY
    "./Image_Assets/Black_Screen.png",            // BLACK_SCREEN_DIRECTORY
    "./Image_Assets/Walnut.png",                  // WALNUT_DIRECTORY
    "./Image_Assets/Walnut-sheet-1.png",          // WALNUT_1_DIRECTORY
    "./Image_Assets/Walnut-sheet-2.png",          // WALNUT_2_DIRECTORY
    "./Image_Assets/Walnut-sheet-3.png",          // WALNUT_3_DIRECTORY
    "./Image_Assets/Walnut-sheet-4.png",          // WALNUT_4_DIRECTORY
    "./Image_Assets/Walnut-sheet-1_blink.png",    // WALNUT_1_BLINK_DIRECTORY
    "./Image_Assets/Walnut-sheet-2_blink.png",    // WALNUT_2_BLINK_DIRECTORY
    "./Image_Assets/Walnut-sheet-3_blink.png",    // WALNUT_3_BLINK_DIRECTORY
    "./Image_Assets/Walnut-sheet-4_blink.png",    // WALNUT_4_BLINK_DIRECTORY
    "./Image_Assets/Zombie_die.png",              // ZOMBIE_DIE_DIRECTORY
    "./Image_Assets/Zombie_head.png",             // ZOMBIE_HEAD_DIRECTORY
    "./Image_Assets/Zombie_eating.png",           // ZOMBIE_EATING_DIRECTORY
    "./Image_Assets/Zombie_eating_blink.png",     // ZOMBIE_EATING_BLINK_DIRECTORY
    "./Image_Assets/StartReady.png",              // START_READY_DIRECTORY
    "./Image_Assets/StartSet.png",                // START_SET_DIRECTORY
    "./Image_Assets/StartPlant.png",              // START_PLANT_DIRECTORY
    "./Image_Assets/Shovel.png",                  // SHOVEL_BAR_DIRECTORY
    "./Image_Assets/Shovel2.png"                  // SHOVEL_DIRECTORY

};