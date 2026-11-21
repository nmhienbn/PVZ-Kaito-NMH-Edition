#include "restart_menu.hpp"
#include "elements/Level/Level.hpp"
#include "elements/button/button.hpp"
#include "events/in_game/pause_menu/pvz1_ui.hpp"
#include "events/seed_chooser/seed_chooser.hpp"
#include "music/music.hpp"

extern int game_state;
extern int clk;
extern Level level;
extern Window win;

/*Retail strings (patoke/re-plants-vs-zombies LawnApp.cpp:335-363
NewOptionsDialog::NewOptionsDialog_Restart -> DIALOG_CONFIRM_RESTART
LawnDialog): header [RESTART_LEVEL_HEADER] / body [RESTART_LEVEL_BODY],
YES relabeled RESTART. The clone keeps its own OK/CANCEL footer labels.*/
static const char *RESTART_HDR = "Restart Level?";
static const char *RESTART_BODY = "Do you want to try this level\nagain from the beginning?";
static const char *RESTART_YES = "OK";
static const char *RESTART_NO = "CANCEL";

/*
Display all needed elements when player tends to restart: the pause options
panel stays up behind the modal LawnDialog-style confirm (like the original,
where DoConfirmRestart opens over the options dialog).
*/
void display_game_restart()
{
    win.draw_bg(level.background_directory);
    display_game_paused_elements();
    draw_pause_base();
    Pvz1ConfirmLayout box = lay_pvz1_confirm(RESTART_HDR, RESTART_BODY);
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    int hover = box.yes.is_mouse_in(mx, my) ? 1 : box.no.is_mouse_in(mx, my) ? 2 : 0;
    draw_pvz1_confirm(box, RESTART_HDR, RESTART_YES, RESTART_NO, hover);
}

/*
Reset some variables to restart game. Retry re-runs "Choose Your Seeds" for
levels that have it (repvz FinishRestartConfirmDialog -> PreNewGame(false):
the board rebuild goes back through ShowSeedChooserScreen; the chooser then
pre-fills the previous pick of this level from level.seed_plants). Conveyor /
fixed-seed levels skip the chooser here just like they do on a fresh entry.
*/
void restart_game()
{
    set_status(game_state, IS_PAUSED, false);
    set_status(game_state, IS_GAME_STARTED, false);
    play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
    load_level();
    clk = 0;
    if (level_needs_chooser())
    {
        set_status(game_state, IS_CHOOSING_SEEDS, true);
        SeedChooserExit chooser_exit = run_seed_chooser();
        set_status(game_state, IS_CHOOSING_SEEDS, false);
        if (chooser_exit == SeedChooserExit::BACK_TO_MENU)
        {
            // MENU in the retry chooser: leave the level entirely.
            set_status(game_state, IS_LEVEL_CHOSEN, false);
            set_status(game_state, IS_SELECTOR_SCREEN, true);
        }
    }
}

/*
Handle when player tends to restart game.
    OK to restart.
    CANCEL to return back to Pause-Menu
*/
void handle_restart_menu_click(const int &mouse_x, const int &mouse_y)
{
    Pvz1ConfirmLayout box = lay_pvz1_confirm(RESTART_HDR, RESTART_BODY);
    // OK to restart.
    if (box.yes.is_mouse_in(mouse_x, mouse_y))
    {
        set_status(game_state, IS_RESTART, false);
        restart_game();
        return;
    }
    // CANCEL to return back to Pause-Menu
    if (box.no.is_mouse_in(mouse_x, mouse_y))
    {
        set_status(game_state, IS_RESTART, false);
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        return;
    }
}
