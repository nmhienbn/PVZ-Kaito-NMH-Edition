#include "reset_menu.hpp"
#include "events/choose_level_scene/render_scene/display_choose_level.hpp"
#include "music/music.hpp"
#include "player/player_data.hpp"

extern int game_state;

#define RESET_X1 325
#define RESET_X2 725
#define RESET_Y1 155
#define RESET_Y2 446
const Button RESET_MENU(RESET_X1, RESET_X2, RESET_Y1, RESET_Y2);
const Button OK_RESET(RESET_X1 + 33, RESET_X1 + 194, RESET_Y1 + 226, RESET_Y1 + 270);
const Button CANCEL_RESET(RESET_X1 + 206, RESET_X1 + 367, RESET_Y1 + 226, RESET_Y1 + 270);

/*
Display this when player tends to reset game level.
2 choices:
    OK
    CANCEL
*/
void display_game_reset()
{
    display_choose_level(false);
    RESET_MENU.display(RESET_PROMPT_DIRECTORY);
    OK_RESET.display_with_text(BUTTON_DIRECTORY, "OK", 25, RGB(0, 196, 0), RGB(43, 44, 58));
    CANCEL_RESET.display_with_text(BUTTON_DIRECTORY, "CANCEL", 25, RGB(0, 196, 0), RGB(43, 44, 58));
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