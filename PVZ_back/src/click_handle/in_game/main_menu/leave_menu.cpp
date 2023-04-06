#include "leave_menu.hpp"

#define LEAVE_X1 325
#define LEAVE_X2 725
#define LEAVE_Y1 125
#define LEAVE_Y2 475
const Button LEAVE_MENU(LEAVE_X1, LEAVE_X2, LEAVE_Y1, LEAVE_Y2);
const Button OK_LEAVE(LEAVE_X1 + 33, LEAVE_X1 + 190, LEAVE_Y1 + 280, LEAVE_Y1 + 320);
const Button CANCEL_LEAVE(LEAVE_X1 + 210, LEAVE_X1 + 365, LEAVE_Y1 + 280, LEAVE_Y1 + 320);

extern bool level_chosen,
    quit,
    is_game_started,
    is_paused;
extern Level level;
extern window win;
extern bool is_leave;

void display_game_leave()
{
    win.draw_bg(level.background_directory);
    display_game_paused_elements();
    display_button(MENU, MENU_DIRECTORY);
    display_button(LEAVE_MENU, LEAVE_PROMPT_DIRECTORY);
    OK_LEAVE.blink();
    CANCEL_LEAVE.blink();
}

void leave_game()
{
    is_paused = false;
    is_game_started = false;
    level_chosen = false;
    play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
    play_music(OPENING_MUSIC_DIRECTORY);
}

void handle_leave_menu_click(const int &mouse_x, const int &mouse_y)
{
    if (OK_LEAVE.is_mouse_in(mouse_x, mouse_y))
    {
        is_leave = false;
        leave_game();
        return;
    }
    if (CANCEL_LEAVE.is_mouse_in(mouse_x, mouse_y))
    {
        is_leave = false;
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        return;
    }
}