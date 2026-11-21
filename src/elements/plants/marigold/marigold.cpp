#include "marigold.hpp"
#include "draw/world_scale.hpp"
#include "elements/Map/Map.hpp"
#include "elements/elements.hpp"
#include "elements/Level/Level.hpp" // roof_tile_dy

// marigold.reanim: anim_idle 0..35, anim_producing 36..50 (fps 12).
static PlantAssets MARIGOLD_ASSET(2, 60, 60, 60, 60, 10, 26);
static PlantAssets MARIGOLD_PRODUCE_ASSET(5, 60, 60, 60, 60, 10, 26);
// The coin-sun pops halfway through anim_producing.
#define MARIGOLD_POP_TICK 40

extern int game_state;
extern Map cells;
extern Window win;
extern Elements game_characters;

Marigold::Marigold(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = MARIGOLD_TYPE;
    health = PLANT_HEALTH_LIMIT[MARIGOLD_TYPE];
    // First bloom 15-20 s in (original: first coin ~24 s, earlier in clones).
    sec_for_prepare = 900 + rand(0, 300);
    directory_num = MARIGOLD_DIRECTORY;
    attack_range = NO_ATTACK;
}

Marigold::~Marigold()
{
}

/*
Bloom cycle: idle -> producing (one coin-sun pops mid-segment) -> idle.
*/
void Marigold::action()
{
    if (producing())
    {
        if (frame == MARIGOLD_POP_TICK)
        {
            // silver dollar pops out of the bloom (repvz COIN_MOTION_FROM_PLANT)
            spawn_sun_from_plant(game_characters.suns, row, col, COIN_SUN_VALUE, MONEY_SILVER_COIN);
        }
        if (frame == 0) // one-shot segment wrapped back around
        {
            directory_num = MARIGOLD_DIRECTORY;
            sec_for_prepare = MARIGOLD_PRODUCE_CLK + rand(0, 100);
        }
        return;
    }
    --sec_for_prepare;
    if (sec_for_prepare <= 0 && frame == 0)
    {
        directory_num = MARIGOLD_PRODUCE_DIRECTORY;
        frame = 0;
    }
}

void Marigold::display(const int &_row)
{
    if (row != _row)
        return;
    const PlantAssets &assets = producing() ? MARIGOLD_PRODUCE_ASSET : MARIGOLD_ASSET;
    if (display_reanim(assets))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        next_frame(assets);
        return;
    }
    win.draw_png_width_scaled(MARIGOLD_DIRECTORY, cells[row][col].x1 + 15,
                              (cells[row][col].y1 + roof_tile_dy(col)) + 25, world_px(ELEMENT_WIDTH - 40));
    next_frame(assets);
}
