#include "lilypad.hpp"
#include "draw/world_scale.hpp"
#include "elements/Map/Map.hpp"
#include "elements/Level/Level.hpp" // roof_tile_dy

// Reanim-only art (2Pak Lilypad.reanim, anim_idle 0..68); the icon png is the
// legacy fallback. 5 tpf == reanim 12 fps at 60 tps.
static PlantAssets LILYPAD_ASSET(5, 85, 51, 85, 51, 0, 26);

extern int game_state;
extern Map cells;
extern Window win;

/*
Lily Pad constructor: the support plant of the pool -- a regular plant can be
planted on top of it (see support_plant.hpp).
*/
LilyPad::LilyPad(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = LILYPAD_TYPE;
    health = PLANT_HEALTH_LIMIT[LILYPAD_TYPE];
    sec_for_prepare = 1;
    directory_num = LILYPAD_SHEET_DIRECTORY;
    attack_range = NO_ATTACK;
}

LilyPad::~LilyPad()
{
}

/*
Lily Pad's action: nothing (it only carries its rider plant).
*/
void LilyPad::action()
{
}

/*
Display lily pads in row. Drawn first on its tile (planted before its rider),
so the main plant renders on top of the pad.
*/
void LilyPad::display(const int &_row)
{
    if (row != _row)
        return;
    if (display_reanim(LILYPAD_ASSET))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        next_frame(LILYPAD_ASSET);
        return;
    }
    win.draw_png_width_scaled(LILYPAD_DIRECTORY, cells[row][col].x1,
                              (cells[row][col].y2 + roof_tile_dy(col)) - 50, world_px(cells[row][col].x2 - cells[row][col].x1));
    next_frame(LILYPAD_ASSET);
}
