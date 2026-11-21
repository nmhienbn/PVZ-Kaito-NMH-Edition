#include "gravebuster.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"

// anim_land 10 frames, anim_busting 39 frames (reanim fps 12 == 5 ticks/frame)
#define GRAVEBUSTER_ASSET_FRAMES 5
#define GRAVEBUSTER_LANDING_TICKS (10 * GRAVEBUSTER_ASSET_FRAMES)
// PvZ1 Board: GraveBuster eats a grave in 400 cs == 4 s.
#define GRAVEBUSTER_EAT_TICKS 240

static PlantAssets GRAVEBUSTER_ASSET(GRAVEBUSTER_ASSET_FRAMES, 70, 60, 70, 60, 2, 30);

extern int game_state;
extern Map cells;
extern Window win;

/*
GraveBuster constructor: drops onto the grave and chews it up.
*/
GraveBuster::GraveBuster(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = GRAVEBUSTER_TYPE;
    health = PLANT_HEALTH_LIMIT[GRAVEBUSTER_TYPE];
    directory_num = GRAVEBUSTER_LANDING_DIRECTORY;
    attack_range = NO_ATTACK;
    landing_left = GRAVEBUSTER_LANDING_TICKS;
    eat_left = GRAVEBUSTER_EAT_TICKS;
}

GraveBuster::~GraveBuster()
{
}

/*
Display grave buster in row. Reanim-only art (2Pak GraveBuster.reanim).
*/
void GraveBuster::display(const int &_row)
{
    if (row != _row)
        return;
    if (display_reanim(GRAVEBUSTER_ASSET))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        next_frame(GRAVEBUSTER_ASSET);
        return;
    }
    win.draw_png_width_scaled(GRAVEBUSTER_DIRECTORY, cells[row][col].x1 + 10,
                              (cells[row][col].y1 + roof_tile_dy(col)) + 30, world_px(ELEMENT_WIDTH - 30));
    next_frame(GRAVEBUSTER_ASSET);
}

/*
Grave Buster's action: land, chew through the grave, clear it, then wilt.
*/
void GraveBuster::action()
{
    if (landing_left > 0)
    {
        if (--landing_left == 0)
        {
            directory_num = GRAVEBUSTER_EATING_DIRECTORY;
            frame = 0;
            play_sound_effect(GRAVEBUSTER_MUSIC_DIRECTORY);
            // The chomp-start spray (PARTICLE_GRAVE_BUSTER, repvz
            // Plant.cpp:1108) is fired by the grave-eat watcher
            // (update_grave_eats in elements_actions.cpp); firing a second
            // FX here double-stacked it with the watcher's beats.
        }
        return;
    }
    if (--eat_left <= 0)
    {
        cells[row][col].has_grave = false;
        health = 0; // buster wilts away with the grave gone
        // GraveBusterDie crumble burst (Plant.cpp:1120): also fired by
        // update_grave_eats -- only one FX source per beat.
    }
}
