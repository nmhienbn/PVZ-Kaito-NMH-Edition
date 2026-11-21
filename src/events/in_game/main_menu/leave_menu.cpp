#include "leave_menu.hpp"
#include "elements/Level/Level.hpp"
#include "elements/button/button.hpp"
#include "events/in_game/pause_menu/pvz1_ui.hpp"
#include "music/music.hpp"

extern int game_state;
extern bool quit;
extern Level level;
extern Window win;

/*Retail strings (patoke/re-plants-vs-zombies LawnApp.cpp:676
DoConfirmBackToMain): header "Leave Game?" / body
[LEAVE_GAME_HEADER], YES relabeled [LEAVE_BUTTON].*/
static const char *LEAVE_HDR = "Leave Game?";
static const char *LEAVE_BODY = "Do you want to return\nto the main menu?\n\nYour game will be saved.";
static const char *LEAVE_YES = "LEAVE";
static const char *LEAVE_NO = "CANCEL";

/*
Display this when player tends to leave to main menu: pause options panel
behind, the LawnDialog-style confirm on top (like the original).
*/
void display_game_leave()
{
    win.draw_bg(level.background_directory);
    display_game_paused_elements();
    draw_pause_base();
    Pvz1ConfirmLayout box = lay_pvz1_confirm(LEAVE_HDR, LEAVE_BODY);
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    int hover = box.yes.is_mouse_in(mx, my) ? 1 : box.no.is_mouse_in(mx, my) ? 2 : 0;
    draw_pvz1_confirm(box, LEAVE_HDR, LEAVE_YES, LEAVE_NO, hover);
}

/*
Reset some variable when player really leave to main menu
*/
void leave_game()
{
    set_status(game_state, IS_PAUSED, false);
    set_status(game_state, IS_GAME_STARTED, false);
    set_status(game_state, IS_LEVEL_CHOSEN, false);
    play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
    play_music(OPENING_MUSIC_DIRECTORY);
}

/*
Handle when player tends to leave to main menu:
    OK to leave to main menu
    CANCEL to return back to the pause-menu
*/
void handle_leave_menu_click(const int &mouse_x, const int &mouse_y)
{
    Pvz1ConfirmLayout box = lay_pvz1_confirm(LEAVE_HDR, LEAVE_BODY);
    // OK to leave to main menu
    if (box.yes.is_mouse_in(mouse_x, mouse_y))
    {
        set_status(game_state, IS_LEAVE, false);
        leave_game();
        return;
    }
    // CANCEL to return back to the pause-menu
    if (box.no.is_mouse_in(mouse_x, mouse_y))
    {
        set_status(game_state, IS_LEAVE, false);
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        return;
    }
}
