#include "game_pause.h"
void handle_menu_click(Player &player, Elements &elements, Level &level, Icons &icons, Map &cells,
                       const int &mouse_x, const int &mouse_y, int &clk,
                       bool &is_paused, bool &is_game_started, bool &is_level_chosen)
{
    if (BACK_TO_GAME.is_mouse_in(mouse_x, mouse_y))
    {
        is_paused = false;
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        Mix_ResumeMusic();
        return;
    }
    if (MAIN_MENU.is_mouse_in(mouse_x, mouse_y))
    {
        is_paused = false;
        is_game_started = false;
        is_level_chosen = false;
        reset_level(elements, cells, icons);
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        play_music(OPENING_MUSIC_DIRECTORY);
        return;
    }
    if (RESTART.is_mouse_in(mouse_x, mouse_y))
    {
        is_paused = false;
        is_game_started = false;
        reset_level(elements, cells, icons);
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        load_level(player, level);
        clk = 0;
        is_game_started = false;
        return;
    }
}

void handle_menu_icon_click(const int &mouse_x, const int &mouse_y, bool &is_paused)
{
    if (MENU_ICON.is_mouse_in(mouse_x, mouse_y))
    {
        is_paused = true;
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        Mix_PauseMusic();
    }
}
