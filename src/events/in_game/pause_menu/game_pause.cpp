#include "game_pause.hpp"
#include "pvz1_ui.hpp"
#include "elements/Level/Level.hpp"
#include "elements/button/button.hpp"

extern int game_state;
extern bool quit;
extern Level level;
extern Window win;

/*
Display all elements when player paused the game: the retail OPTIONS panel
(OPTIONS_MENUBACK art with baked "Options" title, NewOptionsDialog layout --
retail pause = the options dialog), with the BACK TO GAME big stone button
and the RESTART / MAIN MENU stone rows; Music/Sound FX slot bars sit under
the shared knob widgets (display_slider, volume.cpp).
*/
void display_game_pause()
{
    win.draw_bg(level.background_directory);
    display_game_paused_elements();
    draw_pause_base();
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
    if (OPT_BACK_TO_GAME_BUTTON.is_mouse_in(mouse_x, mouse_y))
    {
        unpause_game();
        return;
    }
    // MAIN_MENU to leave to main menu
    if (OPT_MAIN_MENU_ROW.is_mouse_in(mouse_x, mouse_y))
    {
        set_status(game_state, IS_LEAVE, true);
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        return;
    }
    // RESTART to restart game
    if (OPT_RESTART_ROW.is_mouse_in(mouse_x, mouse_y))
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
