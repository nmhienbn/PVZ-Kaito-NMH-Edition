#pragma once

#include "draw/rsdl.hpp"

// Almanac is awarded after level 14 (Dave scene), like the original dialog
// being locked until the book is found. Shared by the selector tomb and the
// seed chooser's VIEW ALMANAC button.
inline const int ALMANAC_UNLOCK_LEVEL = 15;

// Suburban Almanac scene, a la original PvZ1: index page (plants / zombies
// entries) -> per-entry page with the reanim preview and a short
// description. Blocking scene like display_starting_screen(): runs its own
// event loop and returns when the player hits CLOSE.
void run_almanac();

// Reanim idle render of a plant, feet pinned at foot_y, centered on
// center_x (the almanac preview; also the award screen's pedestal plant).
// Returns false when the plant has no reanim skin (caller falls back to the
// static icon PNG).
bool draw_plant_preview(int type, float clock, int center_x, int foot_y);

// --- Zombie icon layout -----------------------------------------------------
// Grid thumbnails and the detail vignette follow the ORIGINAL 800x600 almanac
// geometry (see AlmanacDialog.cpp / ReanimationLawn.cpp in the reference):
// thumbnails are the in-game zombie drawn at scale 0.5 with its retail doc
// origin placed inside a 76x80 window cell; the detail zombie is the in-game
// zombie at scale 1.0 with mPosX/mPosY = (559,175) (pass x_location =
// 559+OX, y_location = 175-20 to the normal in-game draw path; the clone's
// y_location and original mPosY differ by 20).
inline constexpr float ALMANAC_ZOMBIE_THUMB_SCALE = 0.5f;

// --- Zombies grid -----------------------------------------------------------
// Original GetZombiePosition (AlmanacDialog.cpp:609-618): 5 columns of 85x80
// cells at (OX + 22 + col*85, 86 + row*80); the boss has its own slot at
// (OX + 192, 486). 26 types = 6 rows, which fit the panel without scrolling
// (the wheel/scrollbar code stays inert when everything is visible).
inline const int ALMANAC_ZOMBIE_COLS = 5;
inline const int ALMANAC_ZOMBIE_CELL_W = 76; // Almanac_ZombieWindow
inline const int ALMANAC_ZOMBIE_CELL_H = 80;
inline const int ALMANAC_ZOMBIE_CELL_DX = 85;
inline const int ALMANAC_ZOMBIE_CELL_DY = 80;
inline const int ALMANAC_ZOMBIE_GRID_X = (WINDOW_WIDTH - 800) / 2 + 22;
inline const int ALMANAC_ZOMBIE_GRID_Y = 86;
inline const int ALMANAC_ZOMBIE_VIS_ROWS = 6;

// Grid cell of a zombie entry at a scroll position (top-left of its 76x80
// window, window coords). The boss is pinned to the original (192,486) slot.
void almanac_zombie_cell(int type, int scroll_row, int &x, int &y);

// Number of hidden grid rows (0 -> no scrollbar, wheel/drag inert).
int almanac_zombie_max_scroll_row();
// True when the row holding "type" is in view at this scroll position.
bool almanac_zombie_row_visible(int type, int scroll_row);
// Zombies-page scrollbar geometry, window coords (same slim-bar idiom as the
// plants page). Returns false when the grid fits in the view.
bool almanac_zombies_scrollbar(int scroll_row, SDL_Rect &track, SDL_Rect &thumb);
// Inverse of the thumb placement, for drags: scroll row from the thumb's top.
int almanac_zombies_scroll_row_at(int thumb_top);

// Resolved layout of the detail-card preview icon (unused by the grid: the
// grid places the retail doc origin directly, see the .cpp).
struct AlmanacZombieLayout
{
    float scale;
    float anchor_x;
    float anchor_y;
    float bbox_l, bbox_t, bbox_r, bbox_b;
};

// Draw a zombie's detail-page pose (the in-game walk/idle draw for the type,
// special segment poses for the types whose almanac vignette uses one,
// Zomboss head + dome driver). x_location/y_location are the in-game zombie
// pair; scale_mul multiplies the retail skin scale. Lazily inits reanimz.
void almanac_draw_zombie_detail(int type, float clock, int x_location,
                                int y_location, float scale_mul);
// Loop length of the zombie's detail pose in 12fps clock units.
float almanac_zombie_pose_loop_length(int type);

// --- Plants grid ------------------------------------------------------------
// Original GetSeedPosition (AlmanacDialog.cpp:522-531): seed index s sits at
// (OX + 26 + (s%8)*52, 92 + (s/8)*78); packets are the in-game seed-bank
// packet (ICON_WIDTH x ICON_HEIGHT). 40 plants = 5 rows, all visible.
inline const int ALMANAC_PLANT_COLS = 8;
inline const int ALMANAC_PLANT_CELL_W = 105; // chooser/bank packet size
inline const int ALMANAC_PLANT_CELL_H = 65;
inline const int ALMANAC_PLANT_CELL_DX = 52;
inline const int ALMANAC_PLANT_CELL_DY = 78;
inline const int ALMANAC_PLANT_GRID_X = (WINDOW_WIDTH - 800) / 2 + 26;
inline const int ALMANAC_PLANT_GRID_Y = 92;
inline const int ALMANAC_PLANT_VIS_ROWS = 5;
// Slim scrollbar: 10px track hugging the grid's right edge (spanning the
// visible rows exactly), 12px square knob thumb.
inline const int ALMANAC_SCROLL_TRACK_W = 10;
inline const int ALMANAC_SCROLL_THUMB = 12;

// Number of hidden grid rows (0 -> no scrollbar, wheel/drag inert).
int almanac_plant_max_scroll_row();
// True when the row holding "type" is in view at this scroll position.
bool almanac_plant_row_visible(int type, int scroll_row);
// Top-left of a plant's packet cell at a scroll position (window coords).
void almanac_plant_cell(int type, int scroll_row, int &x, int &y);
// Plants-page scrollbar geometry, window coords. Returns false when the grid
// fits in the view (bar not drawn, wheel/drag inert).
bool almanac_plants_scrollbar(int scroll_row, SDL_Rect &track, SDL_Rect &thumb);
// Inverse of the thumb placement, for drags: scroll row from the thumb's top.
int almanac_scroll_row_at(int thumb_top);

// --- Detail-card preview backdrop (debut level) -------------------------------
// Backdrop of the subject's FIRST adventure appearance (level/debut_data.hpp),
// drawn inside the 200x200 patch as a 1:1 crop of that level's 1400x600 art
// (backdrop drawn un-panned at (0,0), so image == window coords). `sx/sy`
// picks the strip the subject belongs to: aquatics/water-only zombies get the
// pool water rows, land subjects the lawn rows under their feet (the icon
// foot lines sit at ~y156 (plants) / ~y196 (zombies) inside the patch).
// custom == false = no adventure debut (e.g. the yeti): `dir` then holds the
// stock Almanac_Ground* patch drawn whole (its art IS 200x200, so the same
// crop-call form serves both).
struct AlmanacBackdrop
{
    int dir = -1;        // image directory to draw
    int sx = 0, sy = 0;  // crop top-left for the 200x200 patch
    bool custom = false; // false -> dir is a stock Almanac_Ground* patch
};
AlmanacBackdrop almanac_plant_backdrop(int plant_type);
AlmanacBackdrop almanac_zombie_backdrop(int zombie_type);

// --- Plant grid packet -------------------------------------------------------
// The in-game seed-bank packet (105x65: packet art + portrait + sun tag +
// border + cost) at the packet's normal size, top-left (x, y).
void almanac_draw_plant_packet(int type, int x, int y);

// --- Headless page render (scene dump) ---------------------------------------
// Draws one almanac page into the current renderer without running the scene
// loop: page 0 = index, 1 = plants, 2 = zombies; `entry` selects the detail
// subject (plant/zombie type); `clock` is the reanim clock. The caller does
// the frame capture/readback.
void almanac_render_page(int page, int entry, float clock);
