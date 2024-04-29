#include "game_pause.hpp"
#include "elements/Level/Level.hpp"
#include "elements/button/button.hpp"

extern int game_state;
extern bool quit;
extern Level level;
extern Window win;

/*
Display all elements when player paused the game
*/
void display_game_pause()
{
    win.draw_bg(level.background_directory);
    display_game_paused_elements();
    MENU.display(MENU_DIRECTORY);
    BACK_TO_GAME.display_with_text(BACK_TO_GAME_BUTTON_DIRECTORY, "Back To Game",
                                   37, RGB(0, 196, 0), RGB(43, 44, 58));
    MAIN_MENU.display_with_text(BUTTON_DIRECTORY, "MAIN MENU", 20, RGB(0, 196, 0), RGB(43, 44, 58));
    RESTART.display_with_text(BUTTON_DIRECTORY, "RESTART", 20, RGB(0, 196, 0), RGB(43, 44, 58));
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
