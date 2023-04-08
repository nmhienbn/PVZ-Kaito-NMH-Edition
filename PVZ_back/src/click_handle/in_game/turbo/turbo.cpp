#include "turbo.hpp"

extern bool is_fast;
/*
Handle player click on Turbo icon to speed up the game
*/
void handle_turbo_icon_click(const int &mouse_x, const int &mouse_y)
{
    if (TURBO_ICON.is_mouse_in(mouse_x, mouse_y))
    {
        is_fast ^= 1;
    }
}

/*
Display Turbo icon: x2 speed or not
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
}