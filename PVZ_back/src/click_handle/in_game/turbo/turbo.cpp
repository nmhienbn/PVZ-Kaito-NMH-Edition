#include "turbo.hpp"
#define MAX_DIM_SCREEN_TIME 10

extern bool is_fast;
extern window win;
int dim_screen_time = 0;

/*
Handle player click on Turbo icon to speed up the game.
Dim game when player click on this.
*/
void handle_turbo_icon_click(const int &mouse_x, const int &mouse_y)
{
    if (TURBO_ICON.is_mouse_in(mouse_x, mouse_y))
    {
        is_fast ^= 1;
        dim_screen_time = MAX_DIM_SCREEN_TIME;
    }
}

/*
Display Turbo icon: x2 speed or not.
Display dim_screen if necessary
*/
void display_turbo_icon()
{
    if (is_fast)
    {
        display_button(TURBO_ICON, TURBO_ICON_A_DIRECTORY);
    }
    else
    {
        display_button(TURBO_ICON, TURBO_ICON_DIRECTORY);
    }
    if (dim_screen_time)
    {
        win.set_texture_alpha(BLACK_SCREEN_DIRECTORY, 70);
        win.draw_png(BLACK_SCREEN_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        win.set_texture_alpha(BLACK_SCREEN_DIRECTORY, 150);
        dim_screen_time--;
    }
}