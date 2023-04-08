#include "quit_menu.hpp"

extern bool quit;
extern Level level;
extern bool is_quit;

#define QUIT_X1 325
#define QUIT_X2 725
#define QUIT_Y1 153
#define QUIT_Y2 446
const Button QUIT_MENU(QUIT_X1, QUIT_X2, QUIT_Y1, QUIT_Y2);
const Button OK_QUIT(QUIT_X1 + 33, QUIT_X1 + 190, QUIT_Y1 + 229, QUIT_Y1 + 269);
const Button CANCEL_QUIT(QUIT_X1 + 210, QUIT_X1 + 365, QUIT_Y1 + 229, QUIT_Y1 + 269);

/*
Display this when player tends to quit game at choosing level screen.
2 choice: OK/CANCEL
*/
void display_game_quit()
{
    display_choose_level(false);
    display_button(QUIT_MENU, QUIT_PROMPT_DIRECTORY);
    OK_QUIT.blink();
    CANCEL_QUIT.blink();
}

/*
Handle player click on quit menu:
    OK: Quit game
    CANCEL: return back choosing level screen
*/
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