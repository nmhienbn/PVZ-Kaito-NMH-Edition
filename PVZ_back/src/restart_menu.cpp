#include "restart_menu.h"

extern bool is_restart;
#define RESTART_X1 325
#define RESTART_X2 725
#define RESTART_Y1 155
#define RESTART_Y2 446
const Button RESTART_MENU(RESTART_X1, RESTART_X2, RESTART_Y1, RESTART_Y2);
const Button OK_RESTART(RESTART_X1 + 33, RESTART_X1 + 190, RESTART_Y1 + 226, RESTART_Y1 + 266);
const Button CANCEL_RESTART(RESTART_X1 + 210, RESTART_X1 + 365, RESTART_Y1 + 226, RESTART_Y1 + 266);

void display_game_restart(window &win, Elements &elements, Map &cells, Level &level)
{
    win.draw_bg(level.background_directory);
    display_game_paused_elements(win, elements, cells, level);
    display_button(win, MENU, MENU_DIRECTORY);
    display_button(win, RESTART_MENU, RESTART_PROMPT_DIRECTORY);
    OK_RESTART.blink(win);
    CANCEL_RESTART.blink(win);
}

void restart_game(Player &player, Elements &elements, Level &level, Icons &icons,
                  Map &cells, int &clk, bool &is_paused, bool &is_game_started)
{
    is_paused = false;
    is_game_started = false;
    reset_level(elements, cells, icons);
    play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
    load_level(player, level);
    clk = 0;
}

void handle_restart_menu_click(Player &player, Elements &elements, Level &level, Icons &icons, Map &cells,
                               const int &mouse_x, const int &mouse_y, int &clk,
                               bool &is_paused, bool &is_game_started, bool &is_level_chosen)
{
    if (OK_RESTART.is_mouse_in(mouse_x, mouse_y))
    {
        is_restart = false;
        restart_game(player, elements, level, icons, cells, clk, is_paused, is_game_started);
        return;
    }
    if (CANCEL_RESTART.is_mouse_in(mouse_x, mouse_y))
    {
        is_restart = false;
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        return;
    }
}