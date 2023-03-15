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
#define CLK_LENGTH 5
#define SUN_GEN_SKY_CLK_COUNT 2000
#define SUN_GEN_SUNFLOWER_CLK_COUNT 4000
#define ZOMBIE_CREATE_CLK_COUNT 1200
#define FIRE_PEA_CLK_COUNT 300 // Pea refresh frequency
#define BITE_CLK_COUNT 100     // Zombie bite plant frequency
#define ZOMBIE_CLK_COUNT 22    // Zombie moving frequency
#define SUN_CLK_COUNT 8        // Sun moving frequency
#define PEA_CLK_COUNT 4        // Pea moving frequency

#define PEASHOOTER_LOADING 500 // Peashooter loading
#define SUNFLOWER_LOADING 500  // Sunflower loading
#define WALNUT_LOADING 2000    // Walnut loading
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Alter the 4 define values bellow to increase or decrease the price of plants and the initial sun count
#define INIT_SUN_COUNT 200
#define PEASHOOTER_PRICE 100
#define SUNFLOWER_PRICE 50
#define WALNUT_PRICE 50
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

#define ZOMBIE_INIT_X 900
#define ZOMBIE_DX 1
#define PEA_DX 15
#define SUN_DY 1

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
#define ICON_BAR_X2 113
#define ICON_BAR_Y1 102
#define ICON_BAR_Y2 292

#define INIT_SUN_Y 5
#define SUNFLOWER_ICON_Y1 103
#define PEASHOOTER_ICON_Y1 167
#define WALNUT_ICON_Y1 231

#define ICON_BAR_WIDTH 95
#define ICON_BAR_HEIGHT 194
#define ICON_WIDTH 89
#define ICON_HEIGHT 60
#define SUN_COUNT_WIDTH 70
#define SUN_COUNT_HEIGHT 30
#define SUN_WIDTH 80
#define SUN_HEIGHT 80
#define PEA_WIDTH 25
#define PEA_HEIGHT 25
#define ELEMENT_WIDTH 70
#define ELEMENT_HEIGHT 80

#define STARTING_SCREEN_DIRECTORY "./Image_Assets/Starting_Screen.png"
#define BACKGROUND_DIRECTORY "./Image_Assets/Frontyard.png"
#define BACKGROUND_DIM_DIRECTORY "./Image_Assets/Frontyard_dim.jpeg"
#define LOSING_MESSAGE_DIRECTORY "./Image_Assets/Losing_Message.png"
#define WINNING_MESSAGE_DIRECTORY "./Image_Assets/Winning_Pic.png"
#define BLACK_SCREEN_DIRECTORY "./Image_Assets/Black_Screen.png"
#define ZOMBIE_HEALTHY_DIRECTORY "./Image_Assets/Zombie_healthy.png"
#define ZOMBIE_INJURED_DIRECTORY "./Image_Assets/Zombie_Injured.png"
#define SUN_DIRECTORY "./Image_Assets/sun.png"
#define SUNFLOWER_DIRECTORY "./Image_Assets/Sunflower.png"
#define PEASHOOTER_DIRECTORY "./Image_Assets/Peashooter.png"
#define PEA_DIRECTORY "./Image_Assets/pea.png"
#define WALNUT_UNTOUCHED_DIRECTORY "./Image_Assets/Wallnut_body.png"
#define WALNUT_CRACKED_1_DIRECTORY "./Image_Assets/Wallnut_cracked1.png"
#define WALNUT_CRACKED_2_DIRECTORY "./Image_Assets/Wallnut_cracked2.png"
#define ICON_BAR_DIRECTORY "./Image_Assets/Item_Bar.png"
#define SUN_COUNT_TEXT_FIELD_DIRECTORY "./Image_Assets/sun_count_text_field.png"
#define PEASHOOTER_ICON_BRIGHT_DIRECTORY "./Image_Assets/icon_peashooter_100.png"
#define PEASHOOTER_ICON_DIM_DIRECTORY "./Image_Assets/icon_peashooter_100_dim.png"
#define SUNFLOWER_ICON_BRIGHT_DIRECTORY "./Image_Assets/icon_sunflower.png"
#define SUNFLOWER_ICON_DIM_DIRECTORY "./Image_Assets/icon_sunflower_dim.png"
#define WALNUT_ICON_BRIGHT_DIRECTORY "./Image_Assets/icon_walnut.png"
#define WALNUT_ICON_DIM_DIRECTORY "./Image_Assets/icon_walnut_dim.png"

#define BLACK_SCREEN_DIRECTORY "./Image_Assets/Black_Screen.png"
