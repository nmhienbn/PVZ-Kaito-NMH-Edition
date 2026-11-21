#include "iceshroom.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"

//   anim_idle 0..32, anim_sleep 33..62 (reanim fps 12 == 5 ticks/frame)
#define ICESHROOM_ASSET_FRAMES 5
// Short fuse before the blast (PvZ1: the shroom flashes then pops at once).
#define ICESHROOM_FUSE_TICKS 50
// The icetrap particle flash over the lawn.
#define ICESHROOM_BLAST_TICKS 30

static PlantAssets ICESHROOM_ASSET(ICESHROOM_ASSET_FRAMES, 66, 72, 66, 72, 7, 20);

extern int game_state;
extern Map cells;
extern Window win;
extern Level level;

/*
Ice-shroom constructor: freezes every zombie on the lawn.
*/
IceShroom::IceShroom(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = ICESHROOM_TYPE;
    health = PLANT_HEALTH_LIMIT[ICESHROOM_TYPE];
    sec_for_prepare = ICESHROOM_FUSE_TICKS;
    blast_left = 0;
    directory_num = is_night() ? ICESHROOM_SHEET_DIRECTORY : ICESHROOM_SLEEP_DIRECTORY;
    attack_range = NO_ATTACK;
}

IceShroom::~IceShroom()
{
}

bool IceShroom::is_night() const
{
    return level.is_night() || coffee_awake; // a coffee bean wakes it by day
}

/*
Display ice-shroom in row; the icetrap flash covers the lawn while it fades.
*/
void IceShroom::display(const int &_row)
{
    if (row != _row)
        return;
    if (!display_reanim(ICESHROOM_ASSET))
    {
        win.draw_png_width_scaled(ICESHROOM_DIRECTORY, cells[row][col].x1 + 10,
                                  (cells[row][col].y1 + roof_tile_dy(col)) + 30, world_px(ELEMENT_WIDTH - 30));
    }
    // blast flash, fading out
    if (has_exploded && blast_left > 0)
    {
        float ratio = (float)blast_left / ICESHROOM_BLAST_TICKS;
        win.set_texture_alpha(ICE_TRAP_DIRECTORY, (int)(200 * ratio));
        win.draw_png(ICE_TRAP_DIRECTORY,
                     cells[level.playable_rows.front()][0].x1,
                     cells[level.playable_rows.front()][0].y1,
                     cells[0][8].x2 - cells[0][0].x1,
                     cells[level.playable_rows.back()][8].y2 - cells[level.playable_rows.front()][0].y1);
        win.set_texture_alpha(ICE_TRAP_DIRECTORY, 255);
    }
    // anim advances per logic tick, not per render (auto-FPS)
    for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
         check_status(game_state, IS_PAUSED) == false; _lt--)
        attacked_time--;
    next_frame(ICESHROOM_ASSET);
}

/*
Fuse and blast state; returns true exactly on the blast tick.
*/
bool IceShroom::fuse_step()
{
    if (has_exploded)
        return false;
    if (directory_num == ICESHROOM_SLEEP_DIRECTORY)
    {
        // coffee bean wake-up: rise from the sleep loop, then the fuse burns
        directory_num = ICESHROOM_SHEET_DIRECTORY;
        frame = 0;
    }
    if (--sec_for_prepare > 0)
        return false;
    has_exploded = true;
    blast_left = ICESHROOM_BLAST_TICKS;
    directory_num = ICESHROOM_EXPLODE_DIRECTORY;
    frame = 0;
    play_sound_effect(ICE_MUSIC_DIRECTORY);
    return true;
}

/*
The shroom melts away once the icetrap flash is gone.
*/
void IceShroom::melt_step()
{
    if (has_exploded && --blast_left <= 0)
        health = 0;
}

