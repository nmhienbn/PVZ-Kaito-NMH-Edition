#include "leave_menu.hpp"

#define LEAVE_X1 325
#define LEAVE_X2 725
#define LEAVE_Y1 125
#define LEAVE_Y2 475
const Button LEAVE_MENU(LEAVE_X1, LEAVE_X2, LEAVE_Y1, LEAVE_Y2);
const Button OK_LEAVE(LEAVE_X1 + 33, LEAVE_X1 + 190, LEAVE_Y1 + 280, LEAVE_Y1 + 320);
const Button CANCEL_LEAVE(LEAVE_X1 + 210, LEAVE_X1 + 365, LEAVE_Y1 + 280, LEAVE_Y1 + 320);

extern int game_state;
extern bool quit;
extern Level level;
extern window win;

/*
Display this when player tends to leave to main menu
*/
void display_game_leave()
{
    win.draw_bg(level.background_directory);
    display_game_paused_elements();
    display_button(MENU, MENU_DIRECTORY);
    display_button(LEAVE_MENU, LEAVE_PROMPT_DIRECTORY);
    OK_LEAVE.blink();
    CANCEL_LEAVE.blink();
}

/*
Reset some variable when player really leave to main menu
*/
void leave_game()
{
    set_status(game_state, IS_PAUSED, false);
    set_status(game_state, IS_GAME_STARTED, false);
    set_status(game_state, IS_LEVEL_CHOSEN, false);
    play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
    play_music(URF_DIRECTORY);
}

/*
Handle when player tends to leave to main menu:
    OK to leave to main menu
    CANCEL to return back to the pause-menu
*/
void handle_leave_menu_click(const int &mouse_x, const int &mouse_y)
{
    if (OK_LEAVE.is_mouse_in(mouse_x, mouse_y))
    {
        set_status(game_state, IS_LEAVE, false);
        leave_game();
        return;
    }
    if (CANCEL_LEAVE.is_mouse_in(mouse_x, mouse_y))
    {
        set_status(game_state, IS_LEAVE, false);
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        return;
    }
}