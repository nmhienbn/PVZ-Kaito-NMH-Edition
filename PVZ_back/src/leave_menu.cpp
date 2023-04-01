#include "leave_menu.h"

extern bool is_leave;

#define LEAVE_X1 325
#define LEAVE_X2 725
#define LEAVE_Y1 125
#define LEAVE_Y2 475
const Button LEAVE_MENU(LEAVE_X1, LEAVE_X2, LEAVE_Y1, LEAVE_Y2);
const Button OK_LEAVE(LEAVE_X1 + 33, LEAVE_X1 + 190, LEAVE_Y1 + 280, LEAVE_Y1 + 320);
const Button CANCEL_LEAVE(LEAVE_X1 + 210, LEAVE_X1 + 365, LEAVE_Y1 + 280, LEAVE_Y1 + 320);

void display_game_leave(window &win, Elements &elements, Map &cells, Level &level)
{
    win.draw_bg(level.background_directory);
    display_game_paused_elements(win, elements, cells, level);
    display_button(win, MENU, MENU_DIRECTORY);
    display_button(win, LEAVE_MENU, LEAVE_PROMPT_DIRECTORY);
    OK_LEAVE.blink(win);
    CANCEL_LEAVE.blink(win);
}

void leave_game(Elements &elements, Icons &icons, Map &cells,
                bool &is_paused, bool &is_game_started, bool &is_level_chosen)
{
    is_paused = false;
    is_game_started = false;
    is_level_chosen = false;
    reset_level(elements, cells, icons);
    play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
    play_music(OPENING_MUSIC_DIRECTORY);
}

void handle_leave_menu_click(Player &player, Elements &elements, Level &level, Icons &icons, Map &cells,
                             const int &mouse_x, const int &mouse_y, int &clk,
                             bool &is_paused, bool &is_game_started, bool &is_level_chosen)
{
    if (OK_LEAVE.is_mouse_in(mouse_x, mouse_y))
    {
        is_leave = false;
        leave_game(elements, icons, cells, is_paused, is_game_started, is_level_chosen);
        return;
    }
    if (CANCEL_LEAVE.is_mouse_in(mouse_x, mouse_y))
    {
        is_leave = false;
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        return;
    }
}