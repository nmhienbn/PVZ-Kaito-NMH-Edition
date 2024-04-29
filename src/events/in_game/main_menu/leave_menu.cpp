#include "leave_menu.hpp"
#include "elements/Level/Level.hpp"
#include "elements/button/button.hpp"
#include "music/music.hpp"

#define LEAVE_X1 325
#define LEAVE_X2 725
#define LEAVE_Y1 125
#define LEAVE_Y2 475
const Button LEAVE_MENU(LEAVE_X1, LEAVE_X2, LEAVE_Y1, LEAVE_Y2);
const Button OK_LEAVE(LEAVE_X1 + 33, LEAVE_X1 + 194, LEAVE_Y1 + 280, LEAVE_Y1 + 326);
const Button CANCEL_LEAVE(LEAVE_X1 + 206, LEAVE_X1 + 367, LEAVE_Y1 + 280, LEAVE_Y1 + 326);

extern int game_state;
extern bool quit;
extern Level level;
extern Window win;

/*
Display this when player tends to leave to main menu
*/
void display_game_leave()
{
    win.draw_bg(level.background_directory);
    display_game_paused_elements();
    MENU.display(MENU_DIRECTORY);
    BACK_TO_GAME.display(BACK_TO_GAME_BUTTON_DIRECTORY);
    BACK_TO_GAME.show_text("Back To Game", -3, -3, 37, RGB(0, 196, 0), RGB(43, 44, 58));
    LEAVE_MENU.display(LEAVE_PROMPT_DIRECTORY);
    OK_LEAVE.display_with_text(BUTTON_DIRECTORY, "LEAVE", 25, RGB(0, 196, 0), RGB(43, 44, 58));
    CANCEL_LEAVE.display_with_text(BUTTON_DIRECTORY, "CANCEL", 25, RGB(0, 196, 0), RGB(43, 44, 58));
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
    play_music(OPENING_MUSIC_DIRECTORY);
}

/*
Handle when player tends to leave to main menu:
    OK to leave to main menu
    CANCEL to return back to the pause-menu
*/
void handle_leave_menu_click(const int &mouse_x, const int &mouse_y)
{
    // OK to leave to main menu
    if (OK_LEAVE.is_mouse_in(mouse_x, mouse_y))
    {
        set_status(game_state, IS_LEAVE, false);
        leave_game();
        return;
    }
    // CANCEL to return back to the pause-menu
    if (CANCEL_LEAVE.is_mouse_in(mouse_x, mouse_y))
    {
        set_status(game_state, IS_LEAVE, false);
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        return;
    }
}