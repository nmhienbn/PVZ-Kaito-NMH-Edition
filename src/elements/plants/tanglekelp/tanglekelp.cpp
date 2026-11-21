#include "tanglekelp.hpp"
#include "draw/world_scale.hpp"
#include "elements/Map/Map.hpp"
#include "elements/elements.hpp"
#include "support.hpp"
#include <algorithm>
#include "elements/Level/Level.hpp" // roof_tile_dy
using std::find;

// Reanim-only art (2Pak Tanglekelp.reanim: idle 0..76, grab 77..91). 5 tpf.
static PlantAssets TANGLEKELP_ASSET(5, 85, 61, 85, 61, 5, 20);
// Grab window: the vine tangle plays once, then the kelp is gone (GOTY: grab
// at 0, drag-under at 50, die at 100 ticks @100fps -- compressed here).
#define TANGLEKELP_GRAB_TICKS 70

extern int game_state;
extern Map cells;
extern Window win;
extern Elements game_characters;

/*
Tangle Kelp constructor
*/
TangleKelp::TangleKelp(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = TANGLEKELP_TYPE;
    health = PLANT_HEALTH_LIMIT[TANGLEKELP_TYPE];
    sec_for_prepare = 1;
    directory_num = TANGLEKELP_SHEET_DIRECTORY;
    attack_range = NO_ATTACK; // targeting is custom (own tile only)
    state = KELP_WAITING;
}

TangleKelp::~TangleKelp()
{
}

/*
First zombie wading onto the kelp's tile (GOTY attack rect == own tile). on the zombie's mouth point (x_location + 95, the same convention
as the engine's reached-block test), so a zombie chewing the kelp is also in
range.
*/
Zombie *TangleKelp::find_target()
{
    int left_limit = cells[row][col].x1 - 15;
    int right_limit = cells[row][col].x2;
    for (const auto &zombie : game_characters.zombies)
    {
        if (zombie->row != row || zombie->is_charmed || zombie->freeze_time)
            continue;
        int zx = zombie->x_location + 95;
        if (is_in(left_limit, zx, right_limit))
            return zombie;
    }
    return nullptr;
}

/*
Tangle Kelp's action: tangle the first pool zombie that touches it and drag
both under the water (one-shot).
*/
void TangleKelp::action()
{
    if (state == KELP_WAITING)
    {
        if (Zombie *zombie = find_target())
        {
            play_sound_effect(ZOMBIE_SPLASH_MUSIC_DIRECTORY);
            // dragged under: no corpse, no re-death animation
            auto &zombies = game_characters.zombies;
            zombies.erase(find(zombies.begin(), zombies.end(), zombie));
            delete zombie;
            state = KELP_GRABBING;
            directory_num = TANGLEKELP_GRAB_DIRECTORY;
            frame = 0;
        }
        return;
    }
    if (frame >= TANGLEKELP_GRAB_TICKS)
        health = 0; // kelp sinks down with its prey
}

/*
Display tangle kelps in row
*/
void TangleKelp::display(const int &_row)
{
    if (row != _row)
        return;
    if (display_reanim(TANGLEKELP_ASSET))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        Plants::next_frame(TANGLEKELP_ASSET);
        return;
    }
    win.draw_png_width_scaled(TANGLEKELP_DIRECTORY, cells[row][col].x1,
                              (cells[row][col].y2 + roof_tile_dy(col)) - 60, world_px(cells[row][col].x2 - cells[row][col].x1));
    Plants::next_frame(TANGLEKELP_ASSET);
}
