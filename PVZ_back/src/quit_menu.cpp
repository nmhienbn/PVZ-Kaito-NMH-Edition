#include "quit_menu.h"

extern bool level_chosen;
extern bool quit;
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
extern bool is_quit;

#define QUIT_X1 325
#define QUIT_X2 725
#define QUIT_Y1 153
#define QUIT_Y2 446
const Button QUIT_MENU(QUIT_X1, QUIT_X2, QUIT_Y1, QUIT_Y2);
const Button OK_QUIT(QUIT_X1 + 33, QUIT_X1 + 190, QUIT_Y1 + 229, QUIT_Y1 + 269);
const Button CANCEL_QUIT(QUIT_X1 + 210, QUIT_X1 + 365, QUIT_Y1 + 229, QUIT_Y1 + 269);

void display_game_quit()
{
    render_choose_level_no_mouse(win, player);
    display_button(win, QUIT_MENU, QUIT_PROMPT_DIRECTORY);
    OK_QUIT.blink(win);
    CANCEL_QUIT.blink(win);
}

void handle_quit_menu_click(const int &mouse_x, const int &mouse_y)
{
    if (OK_QUIT.is_mouse_in(mouse_x, mouse_y))
    {
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        exit(0);
    }
    if (CANCEL_QUIT.is_mouse_in(mouse_x, mouse_y))
    {
        is_quit = false;
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        return;
    }
}