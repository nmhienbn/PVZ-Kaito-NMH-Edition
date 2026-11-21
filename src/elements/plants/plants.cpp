#include "plants.hpp"
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp" // roof_tile_dy
#include "elements/Map/Map.hpp"
#include "elements/zombies/zombie_struct.hpp"
#include "music/music.hpp"
#include <cmath>

extern int game_state;
extern Map cells;
extern Window win;

/*
Plant constructor
*/
Plants::Plants()
{
}

/*
Plant destructor
*/
Plants::~Plants()
{
}

Plants::Plants(const int &_row, const int &_col)
{
    row = _row;
    col = _col;
    attacked_time = 0;
    frame = 0;
    attack_range = NO_ATTACK;
}

/*
Plant display: nothing
*/
void Plants::display(const int &_row)
{
}

/*
Plant geter
@return plant's type
*/
int Plants::get_type() const
{
    return type;
}

/*
Plant action virtual
*/
void Plants::action()
{
}

AttackRange Plants::get_attack_range() const
{
    return attack_range;
}

int Plants::get_directory_num() const
{
    return directory_num;
}

/*
Plant geter
@return plant's row
*/
int Plants::get_row() const
{
    return row;
}

/*
Plant geter
@return plant's column
*/
int Plants::get_col() const
{
    return col;
}

/*
Plant geter
@return plant's health (bite pool, 1 clone hit = 50 original hp)
*/
int Plants::get_health() const
{
    return health;
}

/*
Plant seter
Set plant's attacked time
*/
void Plants::set_attacked_time(const int &_attacked_time)
{
    attacked_time = _attacked_time;
}

/*
Decrease plant's health
*/
void Plants::decrease_health()
{
    --health;
}

/*
@return true if plant died (health <= 0)
*/
bool Plants::is_died()
{
    return health <= 0;
}

namespace
{
/*Flower-pot render offsets on roof maps, per row, applied to EVERY column:
the designer only hand-dragged column 0 in the F2 layout dump, but the dump
is a row translation -- every pot (and pot passenger) in the row shifts by
that amount (see roof_pot_layout_default in plants.hpp). The small nonzero
c1 entries in the dump were drag noise, not intent, so they are dropped.*/
const int ROOF_POT_ROW_LAYOUT[5][2] = {
    /* r0 */ {-3, -20},
    /* r1 */ {-7, -18},
    /* r2 */ {-4, -26},
    /* r3 */ {0, -20},
    /* r4 */ {-1, -17},
};

// A pot-passenger nudges right and up out of the pot rim (layout dump delta).
const int ROOF_PASSENGER_DX = 3, ROOF_PASSENGER_DY = -22;
} // namespace

void roof_pot_layout_default(const int &row, const int &, int &dx, int &dy)
{
    dx = dy = 0;
    if (row >= 0 && row < 5) // row translation: every column of the row, slope and flat alike
    {
        dx = ROOF_POT_ROW_LAYOUT[row][0];
        dy = ROOF_POT_ROW_LAYOUT[row][1];
    }
}

void roof_passenger_layout_default(const int &row, const int &col, int &dx, int &dy)
{
    roof_pot_layout_default(row, col, dx, dy);
    dx += ROOF_PASSENGER_DX;
    dy += ROOF_PASSENGER_DY;
}

PlantAssets::PlantAssets(int _frame, int img_width, int img_height, int game_width, int game_height, int x_alias, int y_alias)
{
    this->frame = _frame;
    this->img_width = img_width;
    this->img_height = img_height;
    this->game_width = game_width;
    this->game_height = game_height;
    this->x_alias = x_alias;
    this->y_alias = y_alias;
}

void Plants::display(const PlantAssets &assets)
{
    // PopCap reanim per-part renderer, then the PvZ2 PAM renderer (both keep
    // the sheet state machine running; only the visual changes). Missing
    // assets -> next fallback in line, ending at the sprite sheets.
    if (display_reanim(assets) || display_pam(assets))
    {
        // Blink
        // anim advances once per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        // Next frame
        next_frame(assets);
        return;
    }
    // current frame; anchored bottom-left so the sprite's bottom edge (its
    // ground contact, cell.y1 + y_alias + game_height) is the invariant
    int sframe = frame / assets.frame;
    int x_loc = cells[row][col].x1 + assets.x_alias;
    int y_bottom = cells[row][col].y1 + assets.y_alias + assets.game_height + roof_tile_dy(col);
    // F2 layout-editor adjustments (transient): offset shifts the anchor
    // wholesale; the debug scale grows the frame about its bottom-center
    // contact point. dbg == 1.0 and zero offsets in normal gameplay.
    const float dbg = rsdl_debug_scale(this);
    const int dw = (int)lroundf(assets.game_width * dbg * WORLD_SCALE);
    const int dh = (int)lroundf(assets.game_height * dbg * WORLD_SCALE);
    const int draw_x = x_loc + (assets.game_width - dw) / 2 + layout_dx;
    const int draw_yb = y_bottom + layout_dy;
    win.draw_nth_frame_bl(directory_num, sframe, assets.img_width, assets.img_height,
                          draw_x, draw_yb, dw, dh);
    // Blink: white-silhouette overlay of the SAME sheet at the SAME frame, so
    // the pose never jumps; only its alpha ramps down over the blink budget
    // (same fade the reanim flash uses, repvz mJustGotShotCounter), replacing
    // the old hard on/off pop when entering/leaving the blink.
    if (attacked_time)
    {
        int blink_dir = blink_of(directory_num);
        if (blink_dir != NULL_DIRECTORY)
        {
            win.load_texture(blink_dir);
            win.set_texture_alpha(blink_dir,
                                  all_img[blink_dir].alpha * attacked_time / MAX_TIME_BLINK);
            win.draw_nth_frame_bl(blink_dir, sframe, assets.img_width, assets.img_height,
                                  draw_x, draw_yb, dw, dh);
            win.set_texture_alpha(blink_dir, all_img[blink_dir].alpha);
        }
        // anim advances once per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
    }
    // Next frame
    next_frame(assets);
}

void Plants::next_frame(const PlantAssets &assets)
{
    // anim advances once per logic tick, not per render (auto-FPS)
    for (int _lt = logic_ticked; _lt > 0 &&
         check_status(game_state, IS_PAUSED) == false; _lt--)
        if (++frame >= assets.frame * all_img[directory_num].n_sheet)
        {
            frame = 0;
        }
}

/*
Frame advance with the attack loop capped at cap_ticks game ticks. Every
shooter launches once per attack loop; the original caps that loop by its
mLaunchRate (150 cs = 90 ticks for pea/spore shooters, 300 cs = 180 ticks for
pults) instead of letting it ride the sheet length, so grids that share one
launch rate stay in sync even with awkward reanim segment counts
(repvz Plant.cpp gPlantDefs).
*/
void Plants::next_frame_capped(const PlantAssets &assets, const int &attack_dir, const int &cap_ticks)
{
    // anim advances once per logic tick, not per render (auto-FPS); the early
    // return keeps the paused / no-tick case out of the loop entirely
    if (check_status(game_state, IS_PAUSED) || !logic_ticked)
        return;
    for (int _lt = logic_ticked; _lt > 0; _lt--)
    {
        int wrap = directory_num == attack_dir ? cap_ticks
                                               : assets.frame * all_img[directory_num].n_sheet;
        if (++frame >= wrap)
            frame = 0;
    }
}