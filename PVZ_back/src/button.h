/*
This file contain in-game rectangles with their x1,x2,y1,y2
*/
#pragma once
#include "gameInfo.h"
#include "rsdl.hpp"
struct Button
{
    int x1, x2, y1, y2;
    Button();
    Button(int _x1, int _x2, int _y1, int _y2);
    ~Button();
    bool is_mouse_in(int mouse_x, int mouse_y) const;
    void blink(window &win) const;
};
const Button Shovel_bar(140, 215, 500, 575);
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

const int level_unlock_new_plant[] = {1, 2, 4, 6, 9};
const Button plant_seed[] = {
    {ICON_BAR_X1 + 3, ICON_BAR_X1 + 3 + ICON_WIDTH, 100, 100 + ICON_HEIGHT}, // Peashooter
    {ICON_BAR_X1 + 3, ICON_BAR_X1 + 3 + ICON_WIDTH, 170, 170 + ICON_HEIGHT}, // Sunflower
    {ICON_BAR_X1 + 3, ICON_BAR_X1 + 3 + ICON_WIDTH, 240, 240 + ICON_HEIGHT}, // Wall-nut
    {ICON_BAR_X1 + 3, ICON_BAR_X1 + 3 + ICON_WIDTH, 310, 310 + ICON_HEIGHT}, // Snow Pea
    {ICON_BAR_X1 + 3, ICON_BAR_X1 + 3 + ICON_WIDTH, 380, 380 + ICON_HEIGHT}, // Cherry Bomb

};
const int plant_sun_cost[] = {100, 50, 50, 150, 150};
const int plant_loading_time[] = {400, 350, 1300, 350, 2000};

const Button ICON_BAR_LV[] = {
    {20, 125, 100, 170},
    {20, 125, 100, 240},
    {20, 125, 100, 310},
    {20, 125, 100, 380},
    {20, 125, 100, 450}};
const Button MENU_ICON(850, 975, 0, 35);
const Button MY_GAME(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);

#define MENU_X1 325
#define MENU_X2 725
#define MENU_Y1 66
#define MENU_Y2 534
const Button MENU(MENU_X1, MENU_X2, MENU_Y1, MENU_Y2);
const Button BACK_TO_GAME(MENU_X1 + 40, MENU_X1 + 360, MENU_Y1 + 380, MENU_Y1 + 450);
const Button MAIN_MENU(MENU_X1 + 105, MENU_X1 + 300, MENU_Y1 + 317, MENU_Y1 + 355);
const Button RESTART(MENU_X1 + 105, MENU_X1 + 300, MENU_Y1 + 274, MENU_Y1 + 312);

#define CONTINUE_WIDTH 185
#define CONTINUE_HEIGHT 37
const Button CONTINUE((WINDOW_WIDTH - CONTINUE_WIDTH) / 2, (WINDOW_WIDTH + CONTINUE_WIDTH) / 2, 500, 500 + CONTINUE_HEIGHT);

const Button RENAME_BUTTON(0, 91, 0, 37);
const Button RESET_LEVEL_BUTTON(WINDOW_WIDTH - 162, WINDOW_WIDTH, 0, 37);
const Button QUIT_BUTTON((WINDOW_WIDTH - 140) / 2, (WINDOW_WIDTH + 140) / 2, 0, 37);
