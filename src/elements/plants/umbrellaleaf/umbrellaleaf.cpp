#include "umbrellaleaf.hpp"
#include "draw/world_scale.hpp"
#include "elements/Map/Map.hpp"
#include "elements/elements.hpp"
#include "music/music.hpp"
#include "elements/Level/Level.hpp" // roof_tile_dy

// Umbrellaleaf.reanim: anim_idle 0..37, anim_block 38..53 (fps 12, 5 tpf).
static PlantAssets UMBRELLALEAF_ASSET(2, 74, 74, 74, 74, 3, 12);
static PlantAssets UMBRELLALEAF_BLOCK_ASSET(5, 74, 74, 74, 74, 3, 12);

extern int game_state;
extern Map cells;
extern Window win;
extern Elements game_characters;

/*True when (row, col) sits inside some umbrella leaf's 3x3 cover.*/
bool umbrella_covers(const int &row, const int &col)
{
    for (auto *p : game_characters.plants)
    {
        if (p->get_type() != UMBRELLALEAF_TYPE)
            continue;
        if (abs(p->get_row() - row) <= 1 && abs(p->get_col() - col) <= 1)
            return true;
    }
    return false;
}

/*umbrella_covers + flashes the block animation on the covering leaf (called
when the deflect actually happens).*/
bool umbrella_protects(const int &row, const int &col)
{
    for (auto *p : game_characters.plants)
    {
        if (p->get_type() != UMBRELLALEAF_TYPE)
            continue;
        if (abs(p->get_row() - row) <= 1 && abs(p->get_col() - col) <= 1)
        {
            static_cast<UmbrellaLeaf *>(p)->flash_block();
            return true;
        }
    }
    return false;
}

UmbrellaLeaf::UmbrellaLeaf(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = UMBRELLALEAF_TYPE;
    health = PLANT_HEALTH_LIMIT[UMBRELLALEAF_TYPE];
    sec_for_prepare = 1;
    directory_num = UMBRELLALEAF_DIRECTORY;
    attack_range = NO_ATTACK;
}

UmbrellaLeaf::~UmbrellaLeaf()
{
}

void UmbrellaLeaf::flash_block()
{
    directory_num = UMBRELLALEAF_BLOCK_DIRECTORY;
    frame = 0;
    block_anim = UMBRELLA_BLOCK_TICKS;
}

void UmbrellaLeaf::action()
{
    if (block_anim > 0 && --block_anim == 0)
    {
        directory_num = UMBRELLALEAF_DIRECTORY; // back to the idle sway
        frame = 0;
    }
}

void UmbrellaLeaf::display(const int &_row)
{
    if (row != _row)
        return;
    bool blocking = directory_num == UMBRELLALEAF_BLOCK_DIRECTORY;
    const PlantAssets &assets = blocking ? UMBRELLALEAF_BLOCK_ASSET : UMBRELLALEAF_ASSET;
    if (display_reanim(assets))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        next_frame(assets);
        return;
    }
    win.draw_png_width_scaled(UMBRELLALEAF_DIRECTORY, cells[row][col].x1 + 8,
                              (cells[row][col].y1 + roof_tile_dy(col)) + 24, world_px(ELEMENT_WIDTH - 26));
    next_frame(assets);
}
