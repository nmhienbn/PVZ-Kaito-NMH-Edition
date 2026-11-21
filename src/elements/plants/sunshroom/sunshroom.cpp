#include "sunshroom.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"

// Sheet-frame pacing for the reanim segments (reanim fps 12 == 5 ticks/frame).
#define SUNSHROOM_ASSET_FRAMES 5
// PvZ1 Board: Sun-shroom grows up after mStateCountdown 12000 cs == 120 s.
#define SUNSHROOM_GROW_TICKS 7200
// Grow-up segment length in ticks (17 reanim frames @ 5 ticks/frame).
#define SUNSHROOM_GROW_ANIM 85
// Small suns are worth 15; the grown shroom makes regular suns.
#define SMALL_SUN_VALUE 15

static PlantAssets SUNSHROOM_ASSET(SUNSHROOM_ASSET_FRAMES, 50, 62, 50, 62, 15, 25);

extern int game_state;
extern Map cells;
extern Window win;
extern Level level;

/*
Sun-shroom constructor: small at first, grows up after 120 s of night time.
*/
SunShroom::SunShroom(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = SUNSHROOM_TYPE;
    health = PLANT_HEALTH_LIMIT[SUNSHROOM_TYPE];
    // First sun 3-12.5 s after planting, same as every sun maker
    // (repvz Plant.cpp:172: mLaunchCounter = RandRangeInt(300, mLaunchRate / 2) cs).
    sec_for_prepare = 180 + rand(0, 569);
    grow_clock = SUNSHROOM_GROW_TICKS;
    grow_anim_left = 0;
    directory_num = is_night() ? SUNSHROOM_SHEET_DIRECTORY : SUNSHROOM_SLEEP_DIRECTORY;
    attack_range = NO_ATTACK;
}

SunShroom::~SunShroom()
{
}

bool SunShroom::is_night() const
{
    return level.is_night() || coffee_awake; // a coffee bean wakes it by day
}

/*
Idle/bounce appearance: the happy bounce starts as a lead-in for the drop --
the original flashes for its last 100 cs of the launch countdown
(repvz Plant.cpp:1021), so the happy segment starts ~2 s ahead. Directory
swaps only at a frame-0 wrap so a segment never cuts mid-play.
*/
const int SUNSHROOM_GLOW_LEAD = 120;

void SunShroom::determine_appearance()
{
    if (directory_num == SUNSHROOM_GROW_DIRECTORY || directory_num == SUNSHROOM_SLEEP_DIRECTORY)
        return; // grow-up segment plays once; sleeping loop stays until woken
    int idle_dir = is_grown ? SUNSHROOM_BIG_DIRECTORY : SUNSHROOM_SHEET_DIRECTORY;
    int happy_dir = is_grown ? SUNSHROOM_BIG_HAPPY_DIRECTORY : SUNSHROOM_HAPPY_DIRECTORY;
    if (frame == 0)
        directory_num = sec_for_prepare <= SUNSHROOM_GLOW_LEAD ? happy_dir : idle_dir;
}

/*
Produce a sun: 15 sun while small, the regular 25 after growing up. The drop
is timer-authoritative like the original's mLaunchCounter
(repvz Plant.cpp:1027) -- not gated on a happy-segment frame, which used to
stretch the interval past 23.5-25 s.
*/
void SunShroom::gen_sun(vector<Sun> &suns)
{
    if (directory_num == SUNSHROOM_SLEEP_DIRECTORY)
    {
        // coffee bean wake-up: rise from the sleep loop to the small idle
        directory_num = SUNSHROOM_SHEET_DIRECTORY;
        frame = 0;
    }
    if (directory_num == SUNSHROOM_GROW_DIRECTORY)
        return; // mid-grow-up
    --sec_for_prepare;
    determine_appearance();
    if (sec_for_prepare <= 0)
    {
        // Re-arm the moment the counter runs out: 23.5-25 s
        // (repvz Plant.cpp:1027: RandRangeInt(2350, 2500) cs = 1410-1500 ticks).
        sec_for_prepare = 1410 + rand(0, 89);
        // 15 sun while small, the regular 25 after growing up (repvz
        // Plant.cpp:1030 for the small/large switch).
        spawn_sun_from_plant(suns, row, col, is_grown ? NORMAL_SUN_VALUE : SMALL_SUN_VALUE);
    }
}

/*
Display sun-shroom in row. Reanim-only art (2Pak SunShroom.reanim).
*/
void SunShroom::display(const int &_row)
{
    if (row != _row)
        return;
    if (display_reanim(SUNSHROOM_ASSET))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        next_frame(SUNSHROOM_ASSET);
        return;
    }
    win.draw_png_width_scaled(SUNSHROOM_DIRECTORY, cells[row][col].x1 + 10,
                              (cells[row][col].y1 + roof_tile_dy(col)) + 30, world_px(ELEMENT_WIDTH - 30));
    next_frame(SUNSHROOM_ASSET);
}

/*
Growth timer: after 120 s of night the shroom plays its grow-up segment once
and then produces regular suns.
*/
void SunShroom::grow_step()
{
    if (!is_night() || is_grown)
        return;
    if (grow_clock > 0)
    {
        --grow_clock;
        if (grow_clock == 0)
        {
            play_sound_effect(PLANTGROW_MUSIC_DIRECTORY);
            directory_num = SUNSHROOM_GROW_DIRECTORY;
            frame = 0;
            grow_anim_left = SUNSHROOM_GROW_ANIM;
        }
    }
    else if (grow_anim_left > 0 && --grow_anim_left == 0)
    {
        is_grown = true;
        directory_num = SUNSHROOM_BIG_DIRECTORY;
        frame = 0;
    }
}
