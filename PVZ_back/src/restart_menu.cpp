#include "restart_menu.h"

#define RESTART_X1 325
#define RESTART_X2 725
#define RESTART_Y1 155
#define RESTART_Y2 446
const Button RESTART_MENU(RESTART_X1, RESTART_X2, RESTART_Y1, RESTART_Y2);
const Button OK_RESTART(RESTART_X1 + 33, RESTART_X1 + 190, RESTART_Y1 + 226, RESTART_Y1 + 266);
const Button CANCEL_RESTART(RESTART_X1 + 210, RESTART_X1 + 365, RESTART_Y1 + 226, RESTART_Y1 + 266);

extern bool is_game_started,
    is_paused;
extern int clk;
extern Level level;
extern window win;
extern bool is_restart;

void display_game_restart()
{
    win.draw_bg(level.background_directory);
    display_game_paused_elements();
    display_button(MENU, MENU_DIRECTORY);
    display_button(RESTART_MENU, RESTART_PROMPT_DIRECTORY);
    OK_RESTART.blink();
    CANCEL_RESTART.blink();
}

void restart_game()
{
    is_paused = false;
    is_game_started = false;
    play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
    load_level();
    clk = 0;
}

void handle_restart_menu_click(const int &mouse_x, const int &mouse_y)
{
    if (OK_RESTART.is_mouse_in(mouse_x, mouse_y))
    {
        is_restart = false;
        restart_game();
        return;
    }
    if (CANCEL_RESTART.is_mouse_in(mouse_x, mouse_y))
    {
        is_restart = false;
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        return;
    }
}