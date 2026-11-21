#include "plantern.hpp"
#include "draw/world_scale.hpp"
#include "elements/Map/Map.hpp"
#include "elements/elements.hpp"
#include "elements/Level/Level.hpp" // roof_tile_dy

// Plantern.reanim: idle 0..90 (fps 12 == 5 ticks/frame).
static PlantAssets PLANTERN_ASSET(5, 76, 76, 76, 76, 5, 8);

extern int game_state;
extern Map cells;
extern Window win;

Plantern::Plantern(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = PLANTERN_TYPE;
    health = PLANT_HEALTH_LIMIT[PLANTERN_TYPE];
    sec_for_prepare = 1;
    directory_num = PLANTERN_SHEET_DIRECTORY;
    attack_range = NO_ATTACK;
    play_sound_effect(PLANTERN_MUSIC_DIRECTORY);
}

Plantern::~Plantern()
{
}

void Plantern::display(const int &_row)
{
    if (row != _row)
        return;
    if (display_reanim(PLANTERN_ASSET))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        next_frame(PLANTERN_ASSET);
        return;
    }
    win.draw_png_width_scaled(PLANTERN_DIRECTORY, cells[row][col].x1 + 10,
                              (cells[row][col].y1 + roof_tile_dy(col)) + 25, world_px(ELEMENT_WIDTH - 30));
    next_frame(PLANTERN_ASSET);
}
