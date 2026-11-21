#include "tallnut.hpp"
#include "draw/world_scale.hpp"
#include "elements/Map/Map.hpp"
#include "elements/Level/Level.hpp" // roof_tile_dy

// Reanim-only art (2Pak Tallnut.reanim; its anim_idle marker is empty, the
// body rests static from frame 86 -- the sheet slot plays that range). 5 tpf.
static PlantAssets TALLNUT_ASSET(5, 85, 81, 85, 81, 5, 5);

extern int game_state;
extern Map cells;
extern Window win;

/*
Tall-nut constructor: twice wall-nut's health (plants.json), and tall enough
that vaulting zombies can't get over (the vault block itself needs the zombie
side -- polevault_zombie belongs to the zombie agent, so only the HP wall is
in place here).
*/
TallNut::TallNut(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = TALLNUT_TYPE;
    health = PLANT_HEALTH_LIMIT[TALLNUT_TYPE];
    directory_num = TALLNUT_SHEET_DIRECTORY;
}

TallNut::~TallNut()
{
}

/*
Tall-nut's action: nothing (pure wall).
*/
void TallNut::action()
{
}

/*
Display tall-nuts in row
*/
void TallNut::display(const int &_row)
{
    if (row != _row)
        return;
    if (display_reanim(TALLNUT_ASSET))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        Plants::next_frame(TALLNUT_ASSET);
        return;
    }
    win.draw_png_width_scaled(TALLNUT_DIRECTORY, cells[row][col].x1 + 8,
                              (cells[row][col].y2 + roof_tile_dy(col)) - 84, world_px(ELEMENT_WIDTH - 16));
    Plants::next_frame(TALLNUT_ASSET);
}
