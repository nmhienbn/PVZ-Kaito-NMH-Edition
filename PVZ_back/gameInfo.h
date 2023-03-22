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
#define FPS 40
#define ticks_per_frame 1000 / FPS

#define SUN_GEN_SKY_CLK_COUNT 1000       // Sun from sky frequency
#define SUN_GEN_SUNFLOWER_CLK_COUNT 1500 // Sun from sunflower frequency
#define ZOMBIE_CREATE_CLK_COUNT 600      // Time between zombies' different groups
#define FIRE_PEA_CLK_COUNT 100           // Fire pea refresh frequency
#define BITE_CLK_COUNT 50                // Zombie bite plant frequency
#define ZOMBIE_CLK_COUNT 11              // Zombie moving frequency
#define SUN_CLK_COUNT 4                  // Sun moving frequency
#define PEA_CLK_COUNT 2                  // Pea moving frequency

#define PEASHOOTER_LOADING 400 // Peashooter loading
#define SUNFLOWER_LOADING 350  // Sunflower loading
#define WALNUT_LOADING 1300    // Walnut loading
#define MAX_TIME_BLINK 5
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Alter the 4 define values bellow to increase or decrease the price of plants and the initial sun count
#define INIT_SUN_COUNT 100
#define NORMAL_SUN_VALUE 50
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

/*Playground elements size*/
#define WINDOW_WIDTH 1050
#define WINDOW_HEIGHT 600
#define BLOCK_WIDTH 81
#define BLOCK_HEIGHT 101
#define HORIZ_BLOCK_COUNT 9
#define VERT_BLOCK_COUNT 5

#define X_UPPER_LEFT 250
#define Y_UPPER_LEFT 70

#define ICON_BAR_X1 20

#define INIT_SUN_Y 5
#define PEASHOOTER_ICON_Y1 100
#define SUNFLOWER_ICON_Y1 170
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
#define PEASHOOTER_HEIGHT 177
#define PEASHOOTER_G_WIDTH 200
#define PEASHOOTER_G_HEIGHT 157 //**
#define PEASHOOTER_N_SHEET 60
#define PEASHOOTER_C_SHEET 8

#define SUNFLOWER_WIDTH 215
#define SUNFLOWER_HEIGHT 251
#define SUNFLOWER_N_SHEET 60
#define SUNFLOWER_C_SHEET 8

#define SUNFLOWER_H_WIDTH 215
#define SUNFLOWER_H_HEIGHT 251
#define SUNFLOWER_H_N_SHEET 15
#define SUNFLOWER_H_C_SHEET 8
#define SUNFLOWER_F_SHEET 6

#define WALNUT_WIDTH 185
#define WALNUT_HEIGHT 185
#define WALNUT_G_WIDTH 200
#define WALNUT_G_HEIGHT 200

#define ZOMBIE_WIDTH 92
#define ZOMBIE_HEIGHT 126

#define ZOMBIE_FRAME 3
#define ZOMBIE_C_SHEET 6
#define ZOMBIE_N_SHEET 22

#define ZOMBIE_G_WIDTH 92
#define ZOMBIE_G_HEIGHT 126

#define ZOMBIE_EATING_FRAME 5
#define ZOMBIE_EATING_C_SHEET 7
#define ZOMBIE_EATING_N_SHEET 21

#define ZOMBIE_DIE_FRAME 10
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

#define ELEMENT_WIDTH 90
#define ELEMENT_HEIGHT 90
#define TILE_WIDTH 75
#define READY_WIDTH 300
#define READY_HEIGHT 133

#define SHOVEL_WIDTH 139
#define SHOVEL_HEIGHT 141
#define WINNING_MESSAGE_WIDTH 400
#define WINNING_MESSAGE_HEIGHT 407
#define LOSING_MESSAGE_WIDTH 564
#define LOSING_MESSAGE_HEIGHT 468

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Directories
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

enum
{ // Plant seed
    PEASHOOTER_DIRECTORY,
    PEASHOOTER_SHEET_DIRECTORY,
    PEASHOOTER_SHEET_BLINK_DIRECTORY,
    PEASHOOTER_ATTACK_DIRECTORY,
    PEASHOOTER_ATTACK_BLINK_DIRECTORY,

    SUNFLOWER_DIRECTORY,
    SUNFLOWER_SHEET_DIRECTORY,
    SUNFLOWER_SHEET_BLINK_DIRECTORY,
    SUNFLOWER_HAPPY_DIRECTORY,
    SUNFLOWER_HAPPY_BLINK_DIRECTORY,

    WALNUT_DIRECTORY,
    WALNUT_1_DIRECTORY,
    WALNUT_2_DIRECTORY,
    WALNUT_3_DIRECTORY,
    WALNUT_4_DIRECTORY,
    WALNUT_1_BLINK_DIRECTORY,
    WALNUT_2_BLINK_DIRECTORY,
    WALNUT_3_BLINK_DIRECTORY,
    WALNUT_4_BLINK_DIRECTORY,

    PEASHOOTER_ICON_BRIGHT_DIRECTORY,
    PEASHOOTER_ICON_DIM_DIRECTORY,
    SUNFLOWER_ICON_BRIGHT_DIRECTORY,
    SUNFLOWER_ICON_DIM_DIRECTORY,
    WALNUT_ICON_BRIGHT_DIRECTORY,
    WALNUT_ICON_DIM_DIRECTORY,

    PEA_DIRECTORY,
    PEA_EXPLODE_DIRECTORY,

    STARTING_SCREEN_DIRECTORY,
    CHOOSE_LEVELS_DIRECTORY,
    BACKGROUND_DIRECTORY,
    BACKGROUND_LV1_DIRECTORY,
    BACKGROUND_LV2_DIRECTORY,
    LOSING_MESSAGE_DIRECTORY,
    WINNING_MESSAGE_DIRECTORY,

    SUN_DIRECTORY,
    SUN_BAR_DIRECTORY,
    ICON_BAR_DIRECTORY,
    SHOVEL_BAR_DIRECTORY,
    SHOVEL_DIRECTORY,
    MENU_ICON_DIRECTORY,
    MENU_DIRECTORY,
    CONTINUE_DIRECTORY,

    ZOMBIE_HEALTHY_DIRECTORY,
    ZOMBIE_SHEET_DIRECTORY,
    ZOMBIE_BLINK_SHEET_DIRECTORY,
    ZOMBIE_INJURED_DIRECTORY,
    ZOMBIE_DIE_DIRECTORY,
    ZOMBIE_HEAD_DIRECTORY,
    ZOMBIE_EATING_DIRECTORY,
    ZOMBIE_EATING_BLINK_DIRECTORY,

    START_READY_DIRECTORY,
    START_SET_DIRECTORY,
    START_PLANT_DIRECTORY,
    BLACK_SCREEN_DIRECTORY,

    COUNT_USED_DIRECTORY
};

const string image_directory[] = {
    // Plant sheet
    "./image/Plant_sheet/Peashooter.png",              // PEASHOOTER_DIRECTORY
    "./image/Plant_sheet/peashooter_sheet.png",        // PEASHOOTER_SHEET_DIRECTORY
    "./image/Plant_sheet/peashooter_sheet_blink.png",  // PEASHOOTER_SHEET_BLINK_DIRECTORY
    "./image/Plant_sheet/peashooter_attack.png",       // PEASHOOTER_ATTACK_DIRECTORY
    "./image/Plant_sheet/peashooter_attack_blink.png", // PEASHOOTER_ATTACK_BLINK_DIRECTORY

    "./image/Plant_sheet/Sunflower.png",             // SUNFLOWER_DIRECTORY
    "./image/Plant_sheet/sunflower_sheet.png",       // SUNFLOWER_SHEET_DIRECTORY
    "./image/Plant_sheet/sunflower_sheet_blink.png", // SUNFLOWER_SHEET_BLINK_DIRECTORY
    "./image/Plant_sheet/sunflower_happy.png",       // SUNFLOWER_HAPPY_DIRECTORY
    "./image/Plant_sheet/sunflower_happy_blink.png", // SUNFLOWER_HAPPY_BLINK_DIRECTORY

    "./image/Plant_sheet/Walnut.png",               // WALNUT_DIRECTORY
    "./image/Plant_sheet/Walnut-sheet-1.png",       // WALNUT_1_DIRECTORY
    "./image/Plant_sheet/Walnut-sheet-2.png",       // WALNUT_2_DIRECTORY
    "./image/Plant_sheet/Walnut-sheet-3.png",       // WALNUT_3_DIRECTORY
    "./image/Plant_sheet/Walnut-sheet-4.png",       // WALNUT_4_DIRECTORY
    "./image/Plant_sheet/Walnut-sheet-1_blink.png", // WALNUT_1_BLINK_DIRECTORY
    "./image/Plant_sheet/Walnut-sheet-2_blink.png", // WALNUT_2_BLINK_DIRECTORY
    "./image/Plant_sheet/Walnut-sheet-3_blink.png", // WALNUT_3_BLINK_DIRECTORY
    "./image/Plant_sheet/Walnut-sheet-4_blink.png", // WALNUT_4_BLINK_DIRECTORY

    "./image/Plant_seed/icon_peashooter_100.png",     // PEASHOOTER_ICON_BRIGHT_DIRECTORY
    "./image/Plant_seed/icon_peashooter_100_dim.png", // PEASHOOTER_ICON_DIM_DIRECTORY
    "./image/Plant_seed/icon_sunflower.png",          // SUNFLOWER_ICON_BRIGHT_DIRECTORY
    "./image/Plant_seed/icon_sunflower_dim.png",      // SUNFLOWER_ICON_DIM_DIRECTORY
    "./image/Plant_seed/icon_walnut.png",             // WALNUT_ICON_BRIGHT_DIRECTORY
    "./image/Plant_seed/icon_walnut_dim.png",         // WALNUT_ICON_DIM_DIRECTORY

    "./image/Bullets/Pea/pea.png",         // PEA_DIRECTORY
    "./image/Bullets/Pea/pea_explode.png", // PEA_EXPLODE_DIRECTORY

    "./image/Background/Starting_Screen.png", // STARTING_SCREEN_DIRECTORY
    "./image/Background/Choose_levels.png",   // CHOOSE_LEVELS_DIRECTORY
    "./image/Background/Frontyard.png",       // BACKGROUND_DIRECTORY
    "./image/Background/Frontyard_LV1.png",   // BACKGROUND_LV1_DIRECTORY
    "./image/Background/Frontyard_LV2.png",   // BACKGROUND_LV2_DIRECTORY
    "./image/Background/ZombiesWon.png",      // LOSING_MESSAGE_DIRECTORY
    "./image/Background/Winning_Pic.png",     // WINNING_MESSAGE_DIRECTORY

    "./image/Icons/sun.png",       // SUN_DIRECTORY
    "./image/Icons/sun_bar.png",   // SUN_BAR_DIRECTORY
    "./image/Icons/Item_Bar.png",  // ICON_BAR_DIRECTORY
    "./image/Icons/Shovel.png",    // SHOVEL_BAR_DIRECTORY
    "./image/Icons/Shovel2.png",   // SHOVEL_DIRECTORY
    "./image/Icons/menu_icon.png", // MENU_ICON_DIRECTORY
    "./image/Icons/menu.png",      // MENU_DIRECTORY
    "./image/Icons/continue.png",  // CONTINUE_DIRECTORY

    "./image/Zombie/Zombie_healthy.png",      // ZOMBIE_HEALTHY_DIRECTORY
    "./image/Zombie/ZOMBIEWALK1.png",         // ZOMBIE_SHEET_DIRECTORY
    "./image/Zombie/ZOMBIEWALK1_blink.png",   // ZOMBIE_BLINK_SHEET_DIRECTORY
    "./image/Zombie/Zombie_Injured.png",      // ZOMBIE_INJURED_DIRECTORY
    "./image/Zombie/Zombie_die.png",          // ZOMBIE_DIE_DIRECTORY
    "./image/Zombie/Zombie_head.png",         // ZOMBIE_HEAD_DIRECTORY
    "./image/Zombie/Zombie_eating.png",       // ZOMBIE_EATING_DIRECTORY
    "./image/Zombie/Zombie_eating_blink.png", // ZOMBIE_EATING_BLINK_DIRECTORY

    "./image/Annoucement/StartReady.png", // START_READY_DIRECTORY
    "./image/Annoucement/StartSet.png",   // START_SET_DIRECTORY
    "./image/Annoucement/StartPlant.png", // START_PLANT_DIRECTORY
    "./image/Black_Screen.png",           // BLACK_SCREEN_DIRECTORY

};