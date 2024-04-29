#include "quit_menu.hpp"
#include "events/choose_level_scene/render_scene/display_choose_level.hpp"
#include "music/music.hpp"

extern int game_state;
extern bool quit;

#define QUIT_X1 325
#define QUIT_X2 725
#define QUIT_Y1 153
#define QUIT_Y2 446
const Button QUIT_MENU(QUIT_X1, QUIT_X2, QUIT_Y1, QUIT_Y2);
const Button OK_QUIT(QUIT_X1 + 33, QUIT_X1 + 194, QUIT_Y1 + 229, QUIT_Y1 + 273);
const Button CANCEL_QUIT(QUIT_X1 + 206, QUIT_X1 + 367, QUIT_Y1 + 229, QUIT_Y1 + 273);

/*
Display this when player tends to quit game at choosing level screen.
2 choice: OK/CANCEL
*/
void display_game_quit()
{
    display_choose_level(false);
    QUIT_MENU.display(QUIT_PROMPT_DIRECTORY);
    OK_QUIT.display_with_text(BUTTON_DIRECTORY, "OK", 25, RGB(0, 196, 0), RGB(43, 44, 58));
    CANCEL_QUIT.display_with_text(BUTTON_DIRECTORY, "CANCEL", 25, RGB(0, 196, 0), RGB(43, 44, 58));
}

/*
Handle player click on quit menu:
    OK: Quit game
    CANCEL: return back choosing level screen
*/
void handle_quit_menu_click(const int &mouse_x, const int &mouse_y)
{
    // OK: Quit game
    if (OK_QUIT.is_mouse_in(mouse_x, mouse_y))
    {
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        exit(0);
    }
    // CANCEL: return back choosing level screen
    if (CANCEL_QUIT.is_mouse_in(mouse_x, mouse_y))
    {
        set_status(game_state, IS_QUIT, false);
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        return;
    }
}