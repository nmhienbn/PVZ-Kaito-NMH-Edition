#include "game_pause.hpp"

extern int game_state;
extern bool quit;
extern Level level;
extern window win;

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
    display_slider();
}

/*
Handle when player paused the game:
    BACK_TO_GAME to unpause
    MAIN_MENU to leave to main menu
    RESTART to restart game
*/
void handle_menu_click(const int &mouse_x, const int &mouse_y)
{
    // BACK_TO_GAME to unpause
    if (BACK_TO_GAME.is_mouse_in(mouse_x, mouse_y))
    {
        unpause_game();
        return;
    }
    // MAIN_MENU to leave to main menu
    if (MAIN_MENU.is_mouse_in(mouse_x, mouse_y))
    {
        set_status(game_state, IS_LEAVE, true);
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        return;
    }
    // RESTART to restart game
    if (RESTART.is_mouse_in(mouse_x, mouse_y))
    {
        set_status(game_state, IS_RESTART, true);
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        return;
    }
}

/*
Handle player click on Menu icon to pause the game
*/
void handle_pause_icon_click(const int &mouse_x, const int &mouse_y)
{
    if (PAUSE_ICON.is_mouse_in(mouse_x, mouse_y))
    {
        // Reset some variables
        set_status(game_state, IS_PAUSED, true);
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
    set_status(game_state, IS_PAUSED, false);
    play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
    Mix_ResumeMusic();
    Mix_Resume(-1);
}
