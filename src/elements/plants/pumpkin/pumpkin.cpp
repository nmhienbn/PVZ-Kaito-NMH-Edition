#include "pumpkin.hpp"
#include "draw/world_scale.hpp"
#include "elements/Map/Map.hpp"
#include "elements/elements.hpp"
#include "elements/Level/Level.hpp" // roof_tile_dy

// Pumpkin.reanim has no label windows: one static shell pose (frames 0..24
// hold the same look); the skin entry calibrates on 0..24.
static PlantAssets PUMPKIN_ASSET(5, 82, 82, 82, 82, 2, 6);

namespace reanimplant
{
// Posted so the skin draw can swap Pumpkin_front for the damage-stage art
// (pumpkin_damage1/3.png) by hp thirds (original: 4000 hp -> damage1 at 2/3,
// damage3 at 1/3).
void setPumpkinHealth(int hp, int hpMax);
} // namespace reanimplant

extern int game_state;
extern Map cells;
extern Window win;

Pumpkin::Pumpkin(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = PUMPKIN_TYPE;
    health = PLANT_HEALTH_LIMIT[PUMPKIN_TYPE];
    sec_for_prepare = 1;
    directory_num = PUMPKIN_SHEET_DIRECTORY;
    attack_range = NO_ATTACK;
}

Pumpkin::~Pumpkin()
{
}

void Pumpkin::display(const int &_row)
{
    if (row != _row)
        return;
    reanimplant::setPumpkinHealth(health, PLANT_HEALTH_LIMIT[PUMPKIN_TYPE]);
    if (display_reanim(PUMPKIN_ASSET))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        next_frame(PUMPKIN_ASSET);
        return;
    }
    win.draw_png_width_scaled(PUMPKIN_DIRECTORY, cells[row][col].x1 - 3,
                              (cells[row][col].y1 + roof_tile_dy(col)) + 10, world_px(ELEMENT_WIDTH - 6));
    next_frame(PUMPKIN_ASSET);
}
