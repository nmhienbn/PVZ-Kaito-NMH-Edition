#include "game_pause.hpp"

extern bool quit,
    is_paused;
extern Level level;
extern window win;
extern bool is_leave,
    is_restart;

/*
Display all elements when player paused the game
*/
void display_game_pause()
{
    win.draw_bg(level.background_directory);
    display_game_paused_elements();
    display_button(MENU, MENU_DIRECTORY);
    BACK_TO_GAME.blink();
    MAIN_MENU.blink();
    RESTART.blink();
}

/*
Handle when player paused the game:
    BACK_TO_GAME to unpause
    MAIN_MENU to leave to main menu
    RESTART to restart game
*/
void handle_menu_click(const int &mouse_x, const int &mouse_y)
{
    if (BACK_TO_GAME.is_mouse_in(mouse_x, mouse_y))
    {
        unpause_game();
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

/*
Handle player click on Menu icon to pause the game
*/
void handle_menu_icon_click(const int &mouse_x, const int &mouse_y)
{
    if (MENU_ICON.is_mouse_in(mouse_x, mouse_y))
    {
        is_paused = true;
        Mix_PauseMusic();
        Mix_Pause(-1);
        play_sound_effect(PAUSE_MUSIC_DIRECTORY);
    }
}

/*
Update some variables to resume game.
*/
void unpause_game()
{
    is_paused = false;
    play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
    Mix_ResumeMusic();
    Mix_Resume(-1);
}
