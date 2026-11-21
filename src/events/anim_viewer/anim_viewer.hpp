#pragma once

#include <SDL.h>
#include <string>
#include <vector>

/*Debug animation viewer (designer tooling), dressed as the Suburban Almanac:
three pages exactly like the almanac scene -- INDEX (natives art, VIEW PLANTS /
VIEW ZOMBIES SeedChooser buttons, CLOSE), PLANTS (chooser-size seed packet grid
+ picker card) and ZOMBIES (the big 100x100 zombie windows + picker card) on
the almanac page art at the almanac's own coordinates (800x600 pages centered
in the 1050x600 window, +125). Open with F1 from anywhere (selector screen,
level list, in-game); the scene behind freezes because this is a blocking
loop. TAB cycles INDEX -> ZOMBIES -> PLANTS -> INDEX (LEFT/RIGHT on INDEX
cycles too), ESC/F1 backs out one page (subject page -> INDEX -> close),
CLOSE exits from anywhere, INDEX returns to the index page.
Browse EVERY zombie (26) and plant (40) subject, per animation LABEL (every
anim_* segment of the subject's reanim doc) plus, for the classic roster, the
exact game render states ("game: ..." entries driving reanimz::drawZombie /
the sheet hybrid). The grid pages keep one common card scale per page
(VIEWER_ZICON_SCALE for the zombie windows; the almanac seed scale for the
packets) and the almanac slim scrollbar.
Picker card (right half, the almanac card zone): the subject's state preview,
anchored bottom-center of the almanac patch position at the natural in-game
scale (big machines clip to the card zone; their full art still reads in the
grid at the fitted card scale), with the states list below it. SPACE pause,
, . step a frame, V speed 1x/0.5x/2x, UP DOWN / [ ] / PgUp PgDn switch state
(also the < > buttons or clicking a row), the "frame #" box jumps to a typed
frame (Enter commits, Esc cancels), T splits sheet-vs-reanim on the classic
sheet states. The PARTS toggle swaps the states list for the per-track parts
table of the subject's own reanim doc (plain zombie = base + flagpole docs,
skin zombies = their skin doc, plants = their plant skin doc; ALL / RESET /
wheel scroll; click a row to hide a drawn track or force-show a hidden one;
plants keep the 4 helper toggles shadow / cell box / hitbox / blink above the
list). F12 dumps the current frame to /tmp/anim_viewer.png.
*/
void display_anim_viewer();

// --- Pages (shared with the headless anim_viewer_check) -------------------------
// Mirrors the almanac scene's page flow: INDEX is the landing page, the two
// subject pages are entered with the VIEW buttons (or TAB) and return with the
// INDEX button / ESC.
enum
{
    VIEWER_PAGE_INDEX = 0,
    VIEWER_PAGE_PLANTS,
    VIEWER_PAGE_ZOMBIES
};

// --- Page buttons (mirror the almanac scene's rects; 800x600 page at offset
// (WINDOW_WIDTH-800)/2 = 125, values re-stated here so the viewer does not
// include the almanac header): INDEX (157,567,164x26), CLOSE (801,567,89x26),
// VIEW PLANTS (255,345,156x42), VIEW ZOMBIES (612,345,210x48). --------------
inline const SDL_Rect VIEWER_BTN_INDEX = {157, 567, 164, 26};
inline const SDL_Rect VIEWER_BTN_CLOSE = {801, 567, 89, 26};
inline const SDL_Rect VIEWER_BTN_VIEW_PLANTS = {255, 345, 156, 42};
inline const SDL_Rect VIEWER_BTN_VIEW_ZOMBIES = {612, 345, 210, 48};

// --- Subject grids (mirror almanac.hpp ALMANAC_ZOMBIE_*/ALMANAC_PLANT_* -----
// Zombies: 4 columns of the 100x100 ZombieBack windows at (144 + col*101,
// 92 + row*104), 4 visible rows; 26 zombies = 7 rows, so 3 rows scroll.
// Plants: chooser-size 105x65 seed packets at (147 + col*105, 92 + row*66),
// 7 visible rows; 40 plants = 10 rows, so 3 rows scroll.
inline const int VIEWER_GRID_COLS = 4;
inline const int VIEWER_ZGRID_X = 144, VIEWER_ZGRID_Y = 92;
inline const int VIEWER_ZCELL_W = 100, VIEWER_ZCELL_H = 100;
inline const int VIEWER_ZCELL_DX = 101, VIEWER_ZCELL_DY = 104;
inline const int VIEWER_ZVIS_ROWS = 4;
inline const int VIEWER_PGRID_X = 147, VIEWER_PGRID_Y = 92;
inline const int VIEWER_PCELL_W = 105, VIEWER_PCELL_H = 65;
inline const int VIEWER_PCELL_DX = 105, VIEWER_PCELL_DY = 66;
inline const int VIEWER_PVIS_ROWS = 7;
// Slim scrollbar (almanac idiom): 10px track hugging the cells' right edge
// with a 5px gap (zombies x=552, plants x=572), spanning the visible rows;
// 12px square knob thumb.
inline const int VIEWER_SCROLL_TRACK_W = 10;
inline const int VIEWER_SCROLL_THUMB = 12;

// Preview patch (mirror the almanac picker card's patch, page-local):
// zombies (643,110,200x200), plants (646,107,200x200). The state preview
// anchors bottom-center on the patch foot line at natural game scale.
inline const SDL_Rect VIEWER_ZPATCH = {643, 110, 200, 200};
inline const SDL_Rect VIEWER_PPATCH = {646, 107, 200, 200};

// Cell rect of subject idx (within its page) at a scroll row. Rows outside the
// visible window land above/below the grid area; hit-testing skips them.
void anim_viewer_grid_cell(bool zombie, int idx, int scroll_row, int &x, int &y, int &w, int &h);
// Hidden rows below the view (0 = the page fits, no scrollbar).
int anim_viewer_grid_max_scroll_row(bool zombie);
// Scrollbar track/thumb rect for a scroll row; false when the page fits.
bool anim_viewer_grid_scrollbar(bool zombie, int scroll_row, SDL_Rect &track, SDL_Rect &thumb);
// Inverse of the thumb placement: scroll row for a click at track_y.
int anim_viewer_grid_scroll_row_at(bool zombie, int track_y);

// State labels of one subject, in viewer order (the "game: ..." states first,
// then the raw reanim anim_* labels). Debug-tooling pins.
std::vector<std::string> anim_viewer_state_labels(bool zombie, int type);
