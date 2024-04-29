/*
This file contain in-game rectangles with their x1,x2,y1,y2
*/
#pragma once
#include "draw/rsdl.hpp"

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
    void display(const int &button_directory, int angle = 0,
                 SDL_RendererFlip flip = SDL_FLIP_NONE) const;
    void display_with_text(const int &button_dir, const string &text,
                           const int &font_size, const RGB &color, const RGB &shadow_color) const;
};
void display_level_is_locked(const Button &button);

extern const int ICON_BAR_X1;
extern const int ICON_BAR_Y1;
extern const int ICON_BAR_WIDTH;
extern const int ICON_WIDTH;
extern const int ICON_HEIGHT;

extern const int MENU_X1;
extern const int MENU_X2;
extern const int MENU_Y1;
extern const int MENU_Y2;

extern const int CONTINUE_WIDTH;
extern const int CONTINUE_HEIGHT;

extern const Button Shovel_bar;
extern const vector<Button> plant_seed;

extern const Button PAUSE_ICON;
extern const Button TURBO_ICON;

extern const Button MENU;
extern const Button BACK_TO_GAME;
extern const Button MAIN_MENU;
extern const Button RESTART;

extern const Button CONTINUE;

extern const Button PREV_PAGE_BUTTON;
extern const Button NEXT_PAGE_BUTTON;

extern const Button MUSIC_VOLUME;
extern const Button SFX_VOLUME;
