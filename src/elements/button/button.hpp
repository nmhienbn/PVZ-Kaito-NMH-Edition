/*
This file contain in-game rectangles with their x1,x2,y1,y2
*/
#pragma once
#include "draw/rsdl.hpp"
#include "game_param.hpp"
#define ICON_BAR_X1 20
#define ICON_BAR_Y1 90
#define ICON_BAR_WIDTH 120
#define ICON_WIDTH 105
#define ICON_HEIGHT 65

// A rectangle that may be clicked in
struct Button
{
    int x1, x2, y1, y2;
    Button();
    Button(int _x1, int _x2, int _y1, int _y2);
    ~Button();
    bool is_mouse_in(int mouse_x, int mouse_y) const;
    void blink() const;
    void show_text(const string &text, const int &x_bias, const int &y_bias,
                   const int &font_size, const RGB &color, const RGB &shadow_color) const;
};
void display_button(const Button &button, const int &button_directory, int angle = 0, SDL_RendererFlip flip = SDL_FLIP_NONE);
void display_level_is_locked(const Button &button);

const Button Shovel_bar(ICON_BAR_X1, ICON_BAR_X1 + 75, 500, 575);
const Button TAP_TO_START(204, 846, 490, 596);

const vector<Button> plant_seed = []()
{
    vector<Button> buttons;
    buttons.reserve(PLANT_COUNT);
    int y_start = ICON_BAR_Y1 + 1;

    for (int i = 0; i < PLANT_COUNT; ++i)
    {
        buttons.push_back({ICON_BAR_X1, ICON_BAR_X1 + ICON_WIDTH, y_start, y_start + ICON_HEIGHT});
        y_start += ICON_HEIGHT;
    }

    return buttons;
}();

// const vector<Button> ICON_BAR_LV = []()
// {
//     vector<Button> buttons(plant_seed.size(), plant_seed[0]);
//     for (int i = 0, _ = buttons.size(); i < _; ++i)
//     {
//         buttons[i].y2 = plant_seed[i].y2;
//     }
//     return buttons;
// }();

const Button PAUSE_ICON(1000, 1045, 0, 45);
const Button TURBO_ICON(955, 1000, 0, 45);
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
const Button CONTINUE((WINDOW_WIDTH - CONTINUE_WIDTH) / 2, (WINDOW_WIDTH + CONTINUE_WIDTH) / 2, 540, 540 + CONTINUE_HEIGHT);

const Button RENAME_BUTTON(10, 237, 265, 405);
const Button RESET_LEVEL_BUTTON(155, 155 + 252, 520, 520 + 72);
const Button QUIT_BUTTON(750, 750 + 252, 535, 535 + 72);

const Button PREV_PAGE_BUTTON(15, 65, 125, 175);
const Button NEXT_PAGE_BUTTON(985, 1035, 125, 175);
