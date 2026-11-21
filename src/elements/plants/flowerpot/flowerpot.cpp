#include "flowerpot.hpp"
#include "draw/world_scale.hpp"
#include "elements/Map/Map.hpp"
#include "elements/Level/Level.hpp" // roof_tile_dy

// Reanim-only art (2Pak Pot.reanim, anim_idle 0..19). The pot hugs the bottom
// of its tile so the rider plant above keeps its usual footing.
static PlantAssets FLOWERPOT_ASSET(2, 60, 40, 60, 34, 10, 58);

extern int game_state;
extern Map cells;
extern Window win;

FlowerPot::FlowerPot(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = FLOWERPOT_TYPE;
    health = PLANT_HEALTH_LIMIT[FLOWERPOT_TYPE];
    sec_for_prepare = 1;
    directory_num = FLOWERPOT_DIRECTORY; // icon dir carries the idle pacing
    attack_range = NO_ATTACK;
}

FlowerPot::~FlowerPot()
{
}

/*
Flower Pot's action: nothing (it only carries its rider plant).
*/
void FlowerPot::action()
{
}

/*
Display flower pots in row. Drawn before its rider (planted first), so the
main plant renders standing inside the pot.
*/
void FlowerPot::display(const int &_row)
{
    if (row != _row)
        return;
    if (display_reanim(FLOWERPOT_ASSET))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        next_frame(FLOWERPOT_ASSET);
        return;
    }
    win.draw_png_width_scaled(FLOWERPOT_DIRECTORY, cells[row][col].x1 + 15,
                              (cells[row][col].y2 + roof_tile_dy(col)) - 45, world_px(cells[row][col].x2 - cells[row][col].x1 - 30));
    next_frame(FLOWERPOT_ASSET);
}
