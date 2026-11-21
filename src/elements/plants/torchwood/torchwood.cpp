#include "torchwood.hpp"
#include "draw/world_scale.hpp"
#include "elements/Map/Map.hpp"
#include "elements/Level/Level.hpp" // roof_tile_dy

// Reanim-only art (2Pak Torchwood.reanim: idle 4..14). 5 tpf.
static PlantAssets TORCHWOOD_ASSET(5, 85, 81, 85, 81, 5, 5);

extern int game_state;
extern Map cells;
extern Window win;

/*
Torchwood constructor. All the pea conversion logic lives in the bullet side
(src/elements/bullets/pea/pea.cpp): peas crossing this column light up.
*/
Torchwood::Torchwood(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = TORCHWOOD_TYPE;
    health = PLANT_HEALTH_LIMIT[TORCHWOOD_TYPE];
    sec_for_prepare = 1;
    directory_num = TORCHWOOD_SHEET_DIRECTORY;
    attack_range = NO_ATTACK;
}

Torchwood::~Torchwood()
{
}

/*
Torchwood's action: nothing; the pea-side hook does the work.
*/
void Torchwood::action()
{
}

/*
Display torchwoods in row
*/
void Torchwood::display(const int &_row)
{
    if (row != _row)
        return;
    if (display_reanim(TORCHWOOD_ASSET))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        Plants::next_frame(TORCHWOOD_ASSET);
        return;
    }
    win.draw_png_width_scaled(TORCHWOOD_DIRECTORY, cells[row][col].x1 + 15,
                              (cells[row][col].y2 + roof_tile_dy(col)) - 78, world_px(ELEMENT_WIDTH - 30));
    Plants::next_frame(TORCHWOOD_ASSET);
}
