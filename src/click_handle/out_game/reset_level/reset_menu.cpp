#include "reset_menu.hpp"

extern int game_state;
extern Level level;

#define RESET_X1 325
#define RESET_X2 725
#define RESET_Y1 155
#define RESET_Y2 446
const Button RESET_MENU(RESET_X1, RESET_X2, RESET_Y1, RESET_Y2);
const Button OK_RESET(RESET_X1 + 33, RESET_X1 + 190, RESET_Y1 + 226, RESET_Y1 + 266);
const Button CANCEL_RESET(RESET_X1 + 210, RESET_X1 + 365, RESET_Y1 + 226, RESET_Y1 + 266);

/*
Display this when player tends to reset game level.
2 choices:
    OK
    CANCEL
*/
void display_game_reset()
{
    display_choose_level(false);
    display_button(RESET_MENU, RESET_PROMPT_DIRECTORY);
    OK_RESET.blink();
    CANCEL_RESET.blink();
}

/*
Handle player click on reset game level menu.
2 choices:
    OK: Reset game level.
    CANCEL: Do nothing.
Then both return back choosing level screen.
*/
void handle_reset_menu_click(const int &mouse_x, const int &mouse_y)
{
    // OK: Reset game level.
    if (OK_RESET.is_mouse_in(mouse_x, mouse_y))
    {
        set_status(game_state, IS_RESET, false);
        reset_unlocked_level();
    }
    // CANCEL: Do nothing.
    if (CANCEL_RESET.is_mouse_in(mouse_x, mouse_y))
    {
        set_status(game_state, IS_RESET, false);
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        return;
    }
}