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

// --- Zombie icon layout (shared with the headless layout check) -------------
// Every zombie art in the almanac (grid windows AND the detail-card vignette)
// draws at ONE common scale, ALMANAC_ZOMBIE_DRAW_SCALE -- no per-zombie fit:
// the vignette reads the zombies at their natural relative sizes. The value
// reproduces the pre-rework look for a NORMAL zombie: the old 76x80 windows
// fit the 109x137-unit basic at ~0.57 (~78px tall); 0.55 renders it ~75px in
// the 100px glass. Arts too big for the glass at this scale (zomboss head,
// gargantuar torso, zomboni, the balloon) are NOT scaled down for it -- they
// are scissored to the cell/patch instead (crop rect, then container clip).
//
// Placement is bottom-center: the zombie's EFFECTIVE bbox (the pose-space
// crop rect for the types that crop, the union bbox of the pose otherwise --
// measured off-screen by demo/almanac_calib_dump.cpp at scale 1) x the common
// scale, centered on the target x with the bbox bottom on the foot line.
// AlmanacZombieLayout is the resolved placement; almanac_zombie_draw_args
// maps it to the reanimz call arguments (drawZombie box convention, or
// drawSkinSegment x_location/y_location for the segment poses). Same math
// feeds the real scene and demo/almanac_layout_check.cpp.
inline constexpr float ALMANAC_ZOMBIE_DRAW_SCALE = 0.55f;

struct AlmanacZombieLayout
{
    float scale;    // == ALMANAC_ZOMBIE_DRAW_SCALE for almanac placements
    float anchor_x; // bbox-center-x / foot-line anchor in window coords
    float anchor_y;
    float bbox_l, bbox_t, bbox_r, bbox_b; // expected on-screen art bounds
};

// --- Zombies grid (shared with the headless layout check) --------------------
// Big zombie windows in the ZombieBack panel (dark face spans x~141..549 in
// window coords, top y~60): 4 touching columns of 100x100 cells at
// (GRID_X + col*101, GRID_Y + row*104) -- the windows are the original 76x80
// Almanac_ZombieWindow art scaled up x1.32. 26 zombies = 7 rows; 4 rows are
// visible (y 92..504, buttons start at 567), the rest scroll in by whole rows
// via the mouse wheel or the slim scrollbar right of the grid.
inline const int ALMANAC_ZOMBIE_COLS = 4;
inline const int ALMANAC_ZOMBIE_CELL_W = 100;
inline const int ALMANAC_ZOMBIE_CELL_H = 100;
inline const int ALMANAC_ZOMBIE_CELL_DX = 101;
inline const int ALMANAC_ZOMBIE_CELL_DY = 104;
inline const int ALMANAC_ZOMBIE_GRID_X = (WINDOW_WIDTH - 800) / 2 + 19;
inline const int ALMANAC_ZOMBIE_GRID_Y = 92;
inline const int ALMANAC_ZOMBIE_VIS_ROWS = 4;

// Grid cell of a zombie entry at a scroll position (top-left of its 100x100
// window, window coords).
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

// Resolved layout of the grid icon inside the window at (cell_x, cell_y).
AlmanacZombieLayout almanac_zombie_grid_layout(int type, int cell_x, int cell_y);
// Resolved layout of the detail-card preview icon.
AlmanacZombieLayout almanac_zombie_preview_layout(int type);
// reanimz call arguments for a resolved layout.
void almanac_zombie_draw_args(int type, const AlmanacZombieLayout &layout,
                              int &x_location, int &y_location);
// Pose-space crop rect (units at scale 1, relative to the pose draw origin:
// drawSkinSegment's x/y for the segment poses, the drawZombie box's inner
// anchor (x+123, y+141) for the walk poses). True when the type renders its
// FULL pose and shows only this rect (catapult driver, gargantuar torso,
// zomboni vehicle + driver, zomboss head). The rect is also the effective
// bbox the layout fits/centers.
bool almanac_zombie_crop(int type, float &l, float &t, float &r, float &b);
// The crop rect mapped into window coords for a resolved layout (the
// pose-origin mapping inside almanac_draw_zombie_icon). false = no crop.
bool almanac_zombie_clip_rect(int type, const AlmanacZombieLayout &layout,
                              SDL_Rect &clip);

// Draw a zombie's almanac pose at a resolved layout (the scene's own icon
// path; lazily inits reanimz on the game renderer). The pose is the walk
// reanim for most types; the recipe swaps in a fixed segment pose (pole
// vaulter standing with his pole, balloon idle -- the only frames with the
// balloon, catapult driver vignette, gargantuar torso vignette) or the 2Pak
// disco doc for the dancer; Zomboss renders the head-only pose + dome driver
// overlay. `contain`, when given, scissored the draw to that rect (grid cell
// / preview patch); cropped types are additionally scissored to their mapped
// crop rect.
void almanac_draw_zombie_icon(int type, float clock,
                              const AlmanacZombieLayout &layout,
                              const SDL_Rect *contain = nullptr);
// Loop length of the zombie's almanac pose in 12fps clock units (walk loop by
// default, the pose segment's otherwise); 0 when unavailable.
float almanac_zombie_pose_loop_length(int type);

// --- Plants grid (shared with the headless layout check) ---------------------
// Chooser-sized seed packets (105x65, the seed bank / pre-game chooser size
// ICON_WIDTH x ICON_HEIGHT) in 4 columns inside the PlantBack panel (wood
// frame at x 21..443, top edge y~91, bottom edge y~557): cells touch like
// the seed bank does, 7 rows fit (y 92..553), the remaining rows scroll in
// by whole rows via the mouse wheel or the slim scrollbar just right of the
// grid. 40 plants / 4 cols = 10 rows, so the scroll range is 0..3.
inline const int ALMANAC_PLANT_COLS = 4;
inline const int ALMANAC_PLANT_CELL_W = 105; // chooser packet size
inline const int ALMANAC_PLANT_CELL_H = 65;
inline const int ALMANAC_PLANT_CELL_DX = 105;
inline const int ALMANAC_PLANT_CELL_DY = 66;
inline const int ALMANAC_PLANT_GRID_X = (WINDOW_WIDTH - 800) / 2 + 22;
inline const int ALMANAC_PLANT_GRID_Y = 92;
inline const int ALMANAC_PLANT_VIS_ROWS = 7;
// One natural-pixel scale for every almanac packet: each plant's icon art
// draws at this exact factor (no per-plant sizing) so the grid reads at a
// uniform zoom, bottom-anchored inside the icon box. 1.0 is the seed bank /
// pre-game chooser proportion (SEED_ICON_SCALE reference, world_scale.hpp):
// large arts clamp into the box (same containment as the old world-height
// fit), small arts draw 1:1 instead of being shrunk to a grid-wide fit.
inline constexpr float ALMANAC_SEED_SCALE = 1.0f;
// Portrait clamp box inside a cell: 4px side inset, 3px top inset, 6px
// bottom margin -- the lily pad is the exception, bottom flush (margin 0).
inline const int ALMANAC_ICON_SIDE_INSET = 4;
inline const int ALMANAC_ICON_TOP_INSET = 3;
inline const int ALMANAC_ICON_BOTTOM_MARGIN = 6;
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
// The portrait's clamp box inside the cell at (cell_x, cell_y): centered in
// the cell, its bottom edge the icon's anchor line (cell bottom minus the
// margin). The portrait draws at the one shared ALMANAC_SEED_SCALE, so no
// part can leave the cell.
void almanac_plant_icon_box(int type, int cell_x, int cell_y, int &bx, int &by,
                            int &bw, int &bh);
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

// --- Plant grid icon ---------------------------------------------------------
// Packet portrait at the shared almanac seed scale: center-bottom anchored in
// the cell's clamp box (see almanac_plant_icon_box).
void almanac_draw_plant_icon(int type, int cell_x, int cell_y);
