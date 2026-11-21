/*
Seed chooser scene ("Choose Your Seeds") for all_unlocked levels, rework 3:
the original PvZ1 intro flow (repvz LawnApp::NewGame ->
SCENE_LEVEL_INTRO -> ShowSeedChooserScreen + CutScene::StartLevelIntro,
LawnApp.cpp:481-492: the chooser slides on BY ITSELF when the pan-right
completes) with the user's credit-first ordering:

  CREDIT_PAN   the credit roll itself: camera pans house -> street
               (repvz CutScene pan constants, CutScene.cpp:29-38:
               TimePanRightStart 1500 .. TimePanRightEnd 3500) while the
               level's credit zombies walk the lawn; night gravestones
               sprout under the camera with the rumble foley (repvz grows the
               initial graves during the level-intro cutscene). Welcome
               announcer on top.
               The pan landing on the street edge hands straight to SLIDE_IN
               (repvz: TimeSeedChoserSlideOn starts at TimePanRightEnd) - no
               "click to choose" gate.
  VIEW_LAWN    camera rewound to the lawn; the eye button (BACK) pans back to
               the chooser. Matches repvz SeedChooserScreen lawn view cancelled
               via CancelLawnView (Board.cpp:4593-4596).
  SLIDE_IN     panel glides in from the left (repvz TimeSeedChoserSlideOn*
               4000..4250, CutScene.cpp:35-36).
  CHOOSING     scrolling packet list (packets at the in-game ICON_WIDTH size,
               4 touch columns like the seed bank; arrows / drag / wheel /
               the slim rail scrollbar scroll the window over the pool) on the
               catalog panel, which docks right of the bank at
               ICON_BAR_X1 + ICON_WIDTH + 9 with everything on it
               panel-local. The chosen-seeds strip IS the in-game seed bank
               (sun bar + initial sun at (5,5)/(100,17), plant_seed packet
               column at ICON_BAR_X1/ICON_BAR_Y1, plus silhouettes for the
               open slots) at its absolute in-game spot - the bank does not
               ride the panel. Packets carry the plant's DEBUT-world back
               (PvZ2-style, plant_packet_bg_dir). LET'S ROCK
               arms once `needed` slots are filled. MENU aborts to the
               selector screen; VIEW ALMANAC (repvz SeedChooserScreen
               mAlmanacButton, Board.cpp:3385) opens the almanac.
  SLIDE_OUT    after LET'S ROCK the panel slides back out (repvz
               TimeSeedChoserSlideOff* 4500..4750, CutScene.cpp:37-38).
  CREDIT_OUT   the pan is handed back: camera rewinds street -> house (repvz
               EndSeedChooser jumps the cutscene past the slide-off and the
               intro pans back to the lawn, CutScene.cpp:1390-1396), then
               DONE hands off to display_credit(), which skips its own pan
               (credit_chooser_pan_pending) and goes straight to the sod roll
               + READY-SET-PLANT.

Reset rules (repvz): restart rebuilds the board with the same level
(FinishRestartConfirmDialog -> PreNewGame(false) -> NewGame,
LawnApp.cpp:1055-1064) and the chooser pre-fills the previous pick of THIS
level; a different level starts from an empty pick (per-level saved choice
table in level_data.cpp).

The class is a frame-stepped state machine so the headless harness can drive
it like the other *_check demos (run_seed_chooser is just a blocking loop
around SeedChooser::tick).
*/
#pragma once

#include "draw/rsdl.hpp" // SDL_Rect (scrollbar geometry)
#include "elements/button/button.hpp"
#include <vector>

class Zombie;

/*true when the level's seeds are player-picked: seed_mode all_unlocked only.
fixed / conveyor / bowling keep their scripted seed handling, and the
vasebreaker / boss specials never show the picker either.*/
bool level_needs_chooser();

/*How the blocking chooser loop ended.*/
enum class SeedChooserExit
{
    ROCKED,      // LET'S ROCK: pick committed, the level may start
    BACK_TO_MENU // MENU button: abort back to the selector/mode screen
};

/*Blocking chooser loop. On ROCKED, level.seed_plants holds the pick and
player.seed_packets is rebuilt from it (set_level_seeds), and the credit pan
is marked done so display_credit skips its own pan. On BACK_TO_MENU nothing
is committed.*/
SeedChooserExit run_seed_chooser();

class SeedChooser
{
public:
    enum State
    {
        CREDIT_PAN,
        VIEW_LAWN,
        SLIDE_IN,
        CHOOSING,
        SLIDE_OUT,
        CREDIT_OUT,
        DONE
    };

    State state = CREDIT_PAN;
    bool rocked = false;        // left via LET'S ROCK (not a window quit)
    bool back_to_menu = false;  // left via the MENU button
    // Slide offset of the catalog panel: 0 = docked at PANEL_X0 (right of
    // the absolute in-game seed bank), slides -(PANEL_W + PANEL_X0)..0
    // (fully off-screen left .. docked).
    int panel_x = 0;
    int camera = 0;             // current pan offset: 0 = house, PAN_MAX = street
    int scroll_row = 0;         // first visible row of the packet grid
    vector<int> pool;           // unlocked plant types on the list
    vector<int> chosen;         // current pick (pre-filled on restart)
    int needed = 0;             // slots to fill before LET'S ROCK arms
    vector<Zombie *> roster;    // credit-preview zombies of the level

    /*Build pool/chosen/roster (level must already be loaded).*/
    void init();
    /*Draw the current frame's scene into the back buffer (no swap, no
    input handling). Harness/debug code uses this to screenshot a state
    deterministically; tick() is render + present + input + transitions.*/
    void render();
    /*One frame: render + handle input. Returns false once DONE.*/
    bool tick();
    /*Free the roster zombies; commit the pick when rocked.*/
    void finish();

    int max_scroll_row() const; // last valid scroll_row for the current pool

private:
    // Drag scrolling: a press inside the grid view becomes a scroll once the
    // mouse moves past the drag threshold; a non-drag release toggles a pick.
    bool press_active = false;
    bool press_dragging = false;
    int press_y = 0;
    int press_scroll_row = 0;
    // Slim rail scrollbar drag: press grabbed the thumb (or jumped it) and
    // bar_grab_dy is the cursor's offset into the thumb, kept during the drag.
    bool bar_drag = false;
    int bar_grab_dy = 0;

    /*Pool index of the packet under (x, y) with the panel docked and the
    current scroll applied, -1 when outside every cell.*/
    int cell_at(int x, int y) const;
    void toggle_pick(int pool_idx);
    /*true when (x, y) hit a filled chosen-bank slot (and that slot's seed
    went back); repvz SeedChooserScreen click-on-bank unpick.*/
    bool unpick_chosen_at(int x, int y);
};

/*Docked-position click rects (only meaningful in CHOOSING): panel-local
geometry + the panel's dock x (seed_chooser_panel_x0); at run time what is
drawn additionally rides the slide offset. Exposed for the harness.*/
Button seed_chooser_grid_cell(int idx, int scroll_row);
Button seed_chooser_rock_rect();
Button seed_chooser_eye_rect();
Button seed_chooser_menu_rect();
Button seed_chooser_almanac_rect();
Button seed_chooser_scroll_up_rect();
Button seed_chooser_scroll_down_rect();

/*Catalog panel dock metrics (the panel image spans
seed_chooser_panel_x0() .. +seed_chooser_panel_w() when docked).*/
int seed_chooser_panel_x0();
int seed_chooser_panel_w();

/*Chosen strip (absolute, panel-state independent): identical to the in-game
seed bank — slot i is the global plant_seed[i] rect (button.cpp), packet
size ICON_WIDTH x ICON_HEIGHT; the sun bar is drawn at (5,5) width 180 with
the initial sun at (100,17) (render_elements.cpp).*/
Button seed_chooser_chosen_slot(int i);

/*Grid geometry constants exposed for the harness's pixel-scale checks:
cells are the in-game packet size (ICON_WIDTH x ICON_HEIGHT), COLS x
VISIBLE_ROWS are on screen at once out of the whole pool.*/
int seed_chooser_grid_cols();
int seed_chooser_visible_rows();
int seed_chooser_cell_w();
int seed_chooser_cell_h();

/*Docked frame of the slim rail scrollbar (track + thumb) at the chooser's
current scroll; false when the pool fits on one screen and no bar is drawn.*/
bool seed_chooser_scrollbar_geom(const SeedChooser &sc, SDL_Rect &track,
                                 SDL_Rect &thumb);
