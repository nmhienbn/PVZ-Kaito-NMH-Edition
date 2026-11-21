#include "doomshroom.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"

// Sheet-frame pacing for the reanim segments (reanim fps 12 == 5 ticks/frame):
//   anim_idle 0..20, anim_explode 21..43, anim_sleep 44..64.
#define DOOMSHROOM_ASSET_FRAMES 5
// PvZ1 Board: Doom-shroom flashes for ~1 s before the blast.
#define DOOMSHROOM_FUSE_TICKS 50
// anim_explode is 23 reanim frames -> 115 ticks.
#define DOOMSHROOM_EXPLODE_TICKS (23 * DOOMSHROOM_ASSET_FRAMES)

static PlantAssets DOOMSHROOM_ASSET(DOOMSHROOM_ASSET_FRAMES, 84, 80, 84, 80, -2, 12);

extern int game_state;
extern Map cells;
extern Window win;
extern Level level;

/*
Doom-shroom constructor: blows a 3x5 crater into the lawn (the crater tile
stays unplantable for the rest of the level).
*/
DoomShroom::DoomShroom(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = DOOMSHROOM_TYPE;
    health = PLANT_HEALTH_LIMIT[DOOMSHROOM_TYPE];
    sec_for_prepare = DOOMSHROOM_FUSE_TICKS;
    directory_num = is_night() ? DOOMSHROOM_SHEET_DIRECTORY : DOOMSHROOM_SLEEP_DIRECTORY;
    attack_range = THREE_X_FIVE;
}

DoomShroom::~DoomShroom()
{
}

bool DoomShroom::is_night() const
{
    return level.is_night() || coffee_awake; // a coffee bean wakes it by day
}

/*
Fuse: true on the blast tick. Caller applies the 3x5 damage and the crater.
*/
bool DoomShroom::fuse_blow()
{
    if (has_exploded || !is_night())
        return false;
    if (directory_num == DOOMSHROOM_SLEEP_DIRECTORY)
    {
        // coffee bean wake-up: rise from the sleep loop, then the fuse burns
        directory_num = DOOMSHROOM_SHEET_DIRECTORY;
        frame = 0;
    }
    if (--sec_for_prepare > 0)
        return false;
    has_exploded = true;
    directory_num = DOOMSHROOM_EXPLODE_DIRECTORY;
    frame = 0;
    play_sound_effect(DOOM_MUSIC_DIRECTORY);
    return true;
}

/*
Dies once the explosion segment has played out (frame runs past its end
because next_frame here does not wrap after the blast).
*/
void DoomShroom::aftermath()
{
    if (has_exploded && frame >= DOOMSHROOM_EXPLODE_TICKS)
        health = 0;
}

/*
Display doom-shroom in row. Reanim-only art (2Pak DoomShroom.reanim).
*/
void DoomShroom::display(const int &_row)
{
    if (row != _row)
        return;
    if (!display_reanim(DOOMSHROOM_ASSET))
        win.draw_png_width_scaled(DOOMSHROOM_DIRECTORY, cells[row][col].x1 + 10,
                                  (cells[row][col].y1 + roof_tile_dy(col)) + 30, world_px(ELEMENT_WIDTH - 30));
    if (has_exploded)
    {
        // let the explosion segment run to its end instead of wrapping
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && check_status(game_state, IS_PAUSED) == false; _lt--)
            ++frame;
    }
    else
    {
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        next_frame(DOOMSHROOM_ASSET);
    }
}
