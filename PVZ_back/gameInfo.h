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
#define FPS 45
#define sec_per_frame 1000 / FPS

#define SUN_GEN_SKY_CLK_COUNT 1000       // Sun from sky frequency
#define SUN_GEN_SUNFLOWER_CLK_COUNT 2000 // Sun from sunflower frequency
#define ZOMBIE_CREATE_CLK_COUNT 600      // Time between zombies' different groups
#define FIRE_PEA_CLK_COUNT 100           // Fire pea refresh frequency
#define BITE_CLK_COUNT 50                // Zombie bite plant frequency
#define ZOMBIE_CLK_COUNT 11              // Zombie moving frequency
#define SUN_CLK_COUNT 4                  // Sun moving frequency
#define PEA_CLK_COUNT 2                  // Pea moving frequency

#define PEASHOOTER_LOADING 250 // Peashooter loading
#define SUNFLOWER_LOADING 250  // Sunflower loading
#define WALNUT_LOADING 1000    // Walnut loading
#define MAX_TIME_BLINK 10
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
#define ZOMBIE_INIT_X 900
#define ZOMBIE_DX 2
#define PEA_DX 15
#define SUN_DY 3

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
#define TAP_TO_START_X1 230
#define TAP_TO_START_X2 796
#define TAP_TO_START_Y1 520
#define TAP_TO_START_Y2 585
#define ICON_BAR_X1 20
#define ICON_BAR_X2 125
#define ICON_BAR_Y1 102
#define ICON_BAR_Y2 292

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

#define PEASHOOTER_WIDTH 185
#define PEASHOOTER_HEIGHT 157
#define PEASHOOTER_SHEET 60
#define PEASHOOTER_R_SHEET 8
#define PEASHOOTER_C_SHEET 8

#define SUNFLOWER_WIDTH 100
#define SUNFLOWER_HEIGHT 108
#define SUNFLOWER_SHEET 27
#define SUNFLOWER_R_SHEET 5
#define SUNFLOWER_C_SHEET 6

#define WALNUT_WIDTH 185
#define WALNUT_HEIGHT 185

#define ZOMBIE_WIDTH 92
#define ZOMBIE_HEIGHT 126
#define ZOMBIE_SHEET 22
#define ZOMBIE_R_SHEET 4
#define ZOMBIE_C_SHEET 6
#define ZOMBIE_G_WIDTH 92
#define ZOMBIE_G_HEIGHT 126

#define ELEMENT_WIDTH 70
#define ELEMENT_HEIGHT 80

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Directories
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

enum
{
    PEASHOOTER_DIRECTORY,
    PEASHOOTER_SHEET_DIRECTORY,
    PEASHOOTER_ATTACK_DIRECTORY,
    PEA_DIRECTORY,
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
    BACKGROUND_DIRECTORY,
    BACKGROUND_DIM_DIRECTORY,
    LOSING_MESSAGE_DIRECTORY,
    WINNING_MESSAGE_DIRECTORY,
    BLACK_SCREEN_DIRECTORY,
    WALNUT_DIRECTORY,
    WALNUT_1_DIRECTORY,
    WALNUT_2_DIRECTORY,
    WALNUT_3_DIRECTORY,
    WALNUT_4_DIRECTORY,

    COUNT_USED_DIRECTORY
};

const string image_directory[] = {
    "./Image_Assets/Peashooter.png",              // PEASHOOTER_DIRECTORY
    "./Image_Assets/peashooter_sheet.png",        // PEASHOOTER_SHEET_DIRECTORY
    "./Image_Assets/peashooter_attack.png",       // PEASHOOTER_ATTACK_DIRECTORY
    "./Image_Assets/pea.png",                     // PEA_DIRECTORY
    "./Image_Assets/Sunflower.png",               // SUNFLOWER_DIRECTORY
    "./Image_Assets/sunflower_sheet.png",         // SUNFLOWER_SHEET_DIRECTORY
    "./Image_Assets/sunflower_sheet_blink.png",   // SUNFLOWER_SHEET_BLINK_DIRECTORY
    "./Image_Assets/sun.png",                     // SUN_DIRECTORY
    "./Image_Assets/sun_bar.png",                 // SUN_BAR_DIRECTORY
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
    "./Image_Assets/Frontyard.png",               // BACKGROUND_DIRECTORY
    "./Image_Assets/Frontyard_dim.jpeg",          // BACKGROUND_DIM_DIRECTORY
    "./Image_Assets/ZombiesWon.png",              // LOSING_MESSAGE_DIRECTORY
    "./Image_Assets/Winning_Pic.png",             // WINNING_MESSAGE_DIRECTORY
    "./Image_Assets/Black_Screen.png",            // BLACK_SCREEN_DIRECTORY
    "./Image_Assets/Walnut.png",                  // WALNUT_DIRECTORY
    "./Image_Assets/Walnut-sheet-1.png",          // WALNUT_1_DIRECTORY
    "./Image_Assets/Walnut-sheet-2.png",          // WALNUT_2_DIRECTORY
    "./Image_Assets/Walnut-sheet-3.png",          // WALNUT_3_DIRECTORY
    "./Image_Assets/Walnut-sheet-4.png"           // WALNUT_4_DIRECTORY

};
/*
#define PEASHOOTER_DIRECTORY "./Image_Assets/Peashooter.png"
#define PEASHOOTER_SHEET_DIRECTORY "./Image_Assets/peashooter_sheet.png"
#define PEASHOOTER_ATTACK_DIRECTORY "./Image_Assets/peashooter_attack.png"
#define PEA_DIRECTORY "./Image_Assets/pea.png"

#define SUNFLOWER_DIRECTORY "./Image_Assets/Sunflower.png"
#define SUNFLOWER_SHEET_DIRECTORY "./Image_Assets/sunflower_sheet.png"

#define WALNUT_UNTOUCHED_DIRECTORY "./Image_Assets/Wallnut_body.png"
#define WALNUT_CRACKED_1_DIRECTORY "./Image_Assets/Wallnut_cracked1.png"
#define WALNUT_CRACKED_2_DIRECTORY "./Image_Assets/Wallnut_cracked2.png"

#define SUN_DIRECTORY "./Image_Assets/sun.png"
#define SUN_BAR_DIRECTORY "./Image_Assets/sun_bar.png"

#define ICON_BAR_DIRECTORY "./Image_Assets/Item_Bar.png"
#define PEASHOOTER_ICON_BRIGHT_DIRECTORY "./Image_Assets/icon_peashooter_100.png"
#define PEASHOOTER_ICON_DIM_DIRECTORY "./Image_Assets/icon_peashooter_100_dim.png"
#define SUNFLOWER_ICON_BRIGHT_DIRECTORY "./Image_Assets/icon_sunflower.png"
#define SUNFLOWER_ICON_DIM_DIRECTORY "./Image_Assets/icon_sunflower_dim.png"
#define WALNUT_ICON_BRIGHT_DIRECTORY "./Image_Assets/icon_walnut.png"
#define WALNUT_ICON_DIM_DIRECTORY "./Image_Assets/icon_walnut_dim.png"

#define ZOMBIE_HEALTHY_DIRECTORY "./Image_Assets/Zombie_healthy.png"
#define ZOMBIE_SHEET_DIRECTORY "./Image_Assets/ZOMBIEWALK1.png"
#define ZOMBIE_BLINK_SHEET_DIRECTORY "./Image_Assets/ZOMBIEWALK1_blink.png"
#define ZOMBIE_INJURED_DIRECTORY "./Image_Assets/Zombie_Injured.png"

#define STARTING_SCREEN_DIRECTORY "./Image_Assets/Starting_Screen.png"
#define BACKGROUND_DIRECTORY "./Image_Assets/Frontyard.png"
#define BACKGROUND_DIM_DIRECTORY "./Image_Assets/Frontyard_dim.jpeg"
#define LOSING_MESSAGE_DIRECTORY "./Image_Assets/ZombiesWon.png"
#define WINNING_MESSAGE_DIRECTORY "./Image_Assets/Winning_Pic.png"
#define BLACK_SCREEN_DIRECTORY "./Image_Assets/Black_Screen.png"
*/