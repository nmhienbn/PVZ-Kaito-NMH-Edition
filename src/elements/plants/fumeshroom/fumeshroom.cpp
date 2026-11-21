#include "fumeshroom.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"

// Sheet-frame pacing for the reanim segments (reanim fps 12 == 5 ticks/frame):
//   anim_idle 0..58, anim_shooting 59..102, anim_sleep 103..161. The shooting
//   loop is capped at 90 ticks (original mLaunchRate 150 cs, Plant.cpp), so
//   the cloud cadence is 1.5 s, not the full sheet's 3.67 s.
#define FUMESHROOM_ASSET_FRAMES 5
// Cloud leaves the nozzle once the shoot segment opens the cap.
#define FUMESHROOM_FIRE_FRAME 10

static PlantAssets FUMESHROOM_ASSET(FUMESHROOM_ASSET_FRAMES, 64, 72, 64, 72, 8, 20);

extern int game_state;
extern Map cells;
extern Window win;
extern Level level;

/*
FumeShroom constructor: fumes soak everything ~4 tiles ahead, shields too.
*/
FumeShroom::FumeShroom(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = FUMESHROOM_TYPE;
    health = PLANT_HEALTH_LIMIT[FUMESHROOM_TYPE];
    sec_for_prepare = 1;
    directory_num = is_night() ? FUMESHROOM_SHEET_DIRECTORY : FUMESHROOM_SLEEP_DIRECTORY;
    attack_range = ONE_ROW_AHEAD_FOUR;
}

FumeShroom::~FumeShroom()
{
}

bool FumeShroom::is_night() const
{
    return level.is_night() || coffee_awake; // a coffee bean wakes it by day
}

/*
Create new spore cloud from a fume-shroom; the cloud pierces the whole row.
*/
void FumeShroom::fire_fume(vector<Bullet *> &bullets)
{
    if (directory_num == FUMESHROOM_ATTACK_DIRECTORY && frame == FUMESHROOM_FIRE_FRAME * FUMESHROOM_ASSET_FRAMES)
    {
        play_sound_effect(FUME_MUSIC_DIRECTORY);
        bullets.push_back(new Fume(row, cells[row][col].x2 + 5));
    }
}

/*
Attack loop = one cloud per 90 ticks (original mLaunchRate 150 cs); wrap the
attack reel there instead of riding the full sheet.
*/
void FumeShroom::next_frame(const PlantAssets &assets)
{
    next_frame_capped(assets, FUMESHROOM_ATTACK_DIRECTORY, 90);
}

/*
Change fume-shroom's appearance if it needs to attack or not.
*/
void FumeShroom::determine_appearance(bool check_zombie_in_attack_range)
{
    if (directory_num == FUMESHROOM_SLEEP_DIRECTORY)
    {
        // coffee bean wake-up: rise from the sleep loop to the idle reel
        directory_num = FUMESHROOM_SHEET_DIRECTORY;
        frame = 0;
    }
    if (check_zombie_in_attack_range)
    {
        if (directory_num == FUMESHROOM_SHEET_DIRECTORY && frame == 0)
            directory_num = FUMESHROOM_ATTACK_DIRECTORY;
    }
    else
    {
        if (directory_num == FUMESHROOM_ATTACK_DIRECTORY && frame == 0)
            directory_num = FUMESHROOM_SHEET_DIRECTORY;
    }
}

/*
Display fume-shrooms in row. Reanim-only art (2Pak FumeShroom.reanim).
*/
void FumeShroom::display(const int &_row)
{
    if (row != _row)
        return;
    if (display_reanim(FUMESHROOM_ASSET))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        next_frame(FUMESHROOM_ASSET);
        return;
    }
    win.draw_png_width_scaled(FUMESHROOM_DIRECTORY, cells[row][col].x1 + 10,
                              (cells[row][col].y1 + roof_tile_dy(col)) + 30, world_px(ELEMENT_WIDTH - 30));
    next_frame(FUMESHROOM_ASSET);
}
