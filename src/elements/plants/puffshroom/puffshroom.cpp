#include "puffshroom.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"

// Sheet-frame pacing for the reanim segments (reanim fps 12 == 5 ticks/frame):
//   anim_idle 0..12, anim_shooting 13..26, anim_sleep 27..43.
#define PUFFSHROOM_ASSET_FRAMES 5
// Spore leaves the cap after the mouth-puff growl peaks inside anim_shooting.
#define PUFFSHROOM_FIRE_FRAME 7

static PlantAssets PUFFSHROOM_ASSET(PUFFSHROOM_ASSET_FRAMES, 46, 56, 46, 56, 0, 0);

extern int game_state;
extern Map cells;
extern Window win;
extern Level level;

/*
PuffShroom constructor
*/
PuffShroom::PuffShroom(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = PUFFSHROOM_TYPE;
    health = PLANT_HEALTH_LIMIT[PUFFSHROOM_TYPE];
    sec_for_prepare = 1;
    directory_num = is_night() ? PUFFSHROOM_SHEET_DIRECTORY : PUFFSHROOM_SLEEP_DIRECTORY;
    attack_range = ONE_ROW_AHEAD;
}

/*
PuffShroom destructor
*/
PuffShroom::~PuffShroom()
{
}

/*
background_map index 3 = frontyard_night (night levels). By day the shroom
stays asleep unless a coffee bean woke it: it plays anim_sleep and never fires.
*/
bool PuffShroom::is_night() const
{
    return level.is_night() || coffee_awake; // a coffee bean wakes it by day
}

/*
Create new spore puff from a puff-shroom.
*/
void PuffShroom::fire_puff(vector<Bullet *> &bullets)
{
    if (directory_num == PUFFSHROOM_ATTACK_DIRECTORY && frame == PUFFSHROOM_FIRE_FRAME * PUFFSHROOM_ASSET_FRAMES)
    {
        play_sound_effect(PUFF_MUSIC_DIRECTORY);
        bullets.push_back(new Puff(row, cells[row][col].x2 - 20));
    }
}

/*
Attack loop = one spore per 90 ticks (original mLaunchRate 150 cs for
shooters); wrap the attack reel there instead of riding the full sheet.
Note the loop outlives the 14-frame attack sheet on purpose; the reanim
sync clamps the tail frames.
*/
void PuffShroom::next_frame(const PlantAssets &assets)
{
    next_frame_capped(assets, PUFFSHROOM_ATTACK_DIRECTORY, 90);
}

/*
Change puff-shroom's appearance if it needs to attack or not.
*/
void PuffShroom::determine_appearance(bool check_zombie_in_attack_range)
{
    if (!is_night())
        return; // sleeping in daylight
    if (directory_num == PUFFSHROOM_SLEEP_DIRECTORY)
    {
        // coffee bean wake-up: rise from the sleep loop to the idle reel
        directory_num = PUFFSHROOM_SHEET_DIRECTORY;
        frame = 0;
    }
    // Attack
    if (check_zombie_in_attack_range)
    {
        if (directory_num == PUFFSHROOM_SHEET_DIRECTORY && frame == 0)
            directory_num = PUFFSHROOM_ATTACK_DIRECTORY;
    }
    // No attack
    else
    {
        if (directory_num == PUFFSHROOM_ATTACK_DIRECTORY && frame == 0)
            directory_num = PUFFSHROOM_SHEET_DIRECTORY;
    }
}

/*
Display puff-shrooms in row. Reanim-only art (2Pak Puffshroom.reanim); falls
back to the flat packet icon if the reanim parts are missing.
*/
void PuffShroom::display(const int &_row)
{
    if (row != _row)
        return;
    if (display_reanim(PUFFSHROOM_ASSET))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        next_frame(PUFFSHROOM_ASSET);
        return;
    }
    win.draw_png_width_scaled(PUFFSHROOM_DIRECTORY, cells[row][col].x1 + 10,
                              (cells[row][col].y1 + roof_tile_dy(col)) + 30, world_px(ELEMENT_WIDTH - 30));
    next_frame(PUFFSHROOM_ASSET);
}
