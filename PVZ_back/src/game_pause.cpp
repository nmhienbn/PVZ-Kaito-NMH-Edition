#include "game_pause.h"

extern bool is_leave, is_quit, is_reset, is_restart;

void display_game_pause(window &win, Elements &elements, Map &cells, Level &level)
{
    win.draw_bg(level.background_directory);
    display_game_paused_elements(win, elements, cells, level);
    display_button(win, MENU, MENU_DIRECTORY);
}

void handle_menu_click(Player &player, Elements &elements, Level &level, Icons &icons, Map &cells,
                       const int &mouse_x, const int &mouse_y, int &clk,
                       bool &is_paused, bool &is_game_started, bool &is_level_chosen)
{
    if (BACK_TO_GAME.is_mouse_in(mouse_x, mouse_y))
    {
        unpause_game(is_paused);
        return;
    }
    if (MAIN_MENU.is_mouse_in(mouse_x, mouse_y))
    {
        is_leave = true;
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        return;
    }
    if (RESTART.is_mouse_in(mouse_x, mouse_y))
    {
        is_restart = true;
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        return;
    }
}

void handle_menu_icon_click(const int &mouse_x, const int &mouse_y, bool &is_paused)
{
    if (MENU_ICON.is_mouse_in(mouse_x, mouse_y))
    {
        is_paused = true;
        play_sound_effect(PAUSE_MUSIC_DIRECTORY);
        Mix_PauseMusic();
    }
}

void unpause_game(bool &is_paused)
{
    is_paused = false;
    play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
    Mix_ResumeMusic();
}
