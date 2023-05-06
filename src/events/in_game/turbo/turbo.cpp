#include "turbo.hpp"
#define MAX_DIM_SCREEN_TIME 10

extern int game_state;
extern window win;
extern Map cells;
extern Elements game_characters;
int dim_screen_time = 0;

/*
If zombie to near the house, disable the turbo.
*/
void auto_disable_turbo()
{
    for (const auto &zombie : game_characters.zombies)
    {
        if (is_in(cells[0][1].x2 - ZOMBIE_DX + 1, zombie->x_location + ZOMBIE_EXACT_LOCATION, cells[0][1].x2) &&
            check_status(game_state, IS_FAST) == true)
        {
            // disable the turbo & dim screen
            game_state ^= IS_FAST;
            dim_screen_time = MAX_DIM_SCREEN_TIME;
            return;
        }
    }
}

/*
Handle player click on Turbo icon to speed up the game.
Dim game when player click on this.
*/
void handle_turbo_icon_click(const int &mouse_x, const int &mouse_y)
{
    if (TURBO_ICON.is_mouse_in(mouse_x, mouse_y))
    {
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        game_state ^= IS_FAST;
        dim_screen_time = MAX_DIM_SCREEN_TIME;
    }
}

/*
Display Turbo icon: x2 speed or not.
Display dim_screen if necessary
*/
void display_turbo_icon()
{
    if (check_status(game_state, IS_FAST) == true)
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