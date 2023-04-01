#include "reset_menu.h"

extern bool level_chosen;
extern bool reset;
extern bool is_game_started;
extern bool is_paused;
extern bool is_unlocking_plant;
extern int clk;
extern Level level;
extern Elements game_characters;
extern Icons icons;
extern Player player;
extern Map cells;
extern window win;
extern bool is_reset;

#define RESET_X1 325
#define RESET_X2 725
#define RESET_Y1 155
#define RESET_Y2 446
const Button RESET_MENU(RESET_X1, RESET_X2, RESET_Y1, RESET_Y2);
const Button OK_RESET(RESET_X1 + 33, RESET_X1 + 190, RESET_Y1 + 226, RESET_Y1 + 266);
const Button CANCEL_RESET(RESET_X1 + 210, RESET_X1 + 365, RESET_Y1 + 226, RESET_Y1 + 266);

void display_game_reset()
{
    render_choose_level_no_mouse(win, player);
    display_button(win, RESET_MENU, RESET_PROMPT_DIRECTORY);
    OK_RESET.blink(win);
    CANCEL_RESET.blink(win);
}

void handle_reset_menu_click(const int &mouse_x, const int &mouse_y)
{
    if (OK_RESET.is_mouse_in(mouse_x, mouse_y))
    {
        is_reset = false;
        reset_unlocked_level(player);
    }
    if (CANCEL_RESET.is_mouse_in(mouse_x, mouse_y))
    {
        is_reset = false;
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        return;
    }
}