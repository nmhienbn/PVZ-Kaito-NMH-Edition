#include "scaredyshroom.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"

// Sheet-frame pacing for the reanim segments (reanim fps 12 == 5 ticks/frame):
//   idle 0..12, shooting 13..32, scared(enter) 33..44, cower loop 45..58,
//   raise 59..71, sleep 72..89.
#define SCAREDYSHROOM_ASSET_FRAMES 5
// Puff pops out of the trembling cap early in the shooting segment.
#define SCAREDYSHROOM_FIRE_FRAME 6
// A zombie closer than this (about 1.4 tiles, also in neighbour rows)
// makes the shroom duck; charmed zombies do not scare anyone.
#define SCARED_RADIUS_PX 115
// One-shot segments: ducking (12 frames) and raising back (13 frames).
#define SCARED_ENTER_TICKS (12 * SCAREDYSHROOM_ASSET_FRAMES)
#define SCARED_RAISE_TICKS (13 * SCAREDYSHROOM_ASSET_FRAMES)

static PlantAssets SCAREDYSHROOM_ASSET(SCAREDYSHROOM_ASSET_FRAMES, 60, 70, 60, 70, 10, 22);

extern int game_state;
extern Map cells;
extern Window win;
extern Level level;

/*
Scaredy-shroom constructor: full-row spore shots, but ducks when a zombie
comes close.
*/
ScaredyShroom::ScaredyShroom(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = SCAREDYSHROOM_TYPE;
    health = PLANT_HEALTH_LIMIT[SCAREDYSHROOM_TYPE];
    sec_for_prepare = 1;
    state_ticks = 0;
    directory_num = is_night() ? SCAREDYSHROOM_SHEET_DIRECTORY : SCAREDYSHROOM_SLEEP_DIRECTORY;
    attack_range = ONE_ROW_AHEAD;
}

ScaredyShroom::~ScaredyShroom()
{
}

bool ScaredyShroom::is_night() const
{
    return level.is_night() || coffee_awake; // a coffee bean wakes it by day
}

/*
@return true while the shroom is ducked/cowering (no shooting, still edible).
*/
bool ScaredyShroom::is_down() const
{
    return directory_num == SCAREDYSHROOM_SCARED_DIRECTORY ||
           directory_num == SCAREDYSHROOM_COWER_DIRECTORY ||
           directory_num == SCAREDYSHROOM_SLEEP_DIRECTORY;
}

/*
@return true if a hostile zombie is close enough to make the shroom cower.
*/
bool ScaredyShroom::is_scared_now(const vector<Zombie *> &zombies)
{
    int head_x = cells[row][col].x2;
    for (const auto &zombie : zombies)
    {
        if (zombie->is_charmed)
            continue;
        if (abs(zombie->row - row) > 1)
            continue;
        if (abs(zombie->x_location - head_x) < SCARED_RADIUS_PX)
            return true;
    }
    return false;
}

/*
Create new spore puff; the scaredy-shroom shoots across the whole row.
*/
void ScaredyShroom::fire_puff(vector<Bullet *> &bullets)
{
    if (directory_num == SCAREDYSHROOM_ATTACK_DIRECTORY && frame == SCAREDYSHROOM_FIRE_FRAME * SCAREDYSHROOM_ASSET_FRAMES)
    {
        play_sound_effect(PUFF_MUSIC_DIRECTORY);
        bullets.push_back(new Puff(row, cells[row][col].x2 - 20, WINDOW_WIDTH));
    }
}

/*
Scaredy-shroom looks: duck when a zombie is close (scared -> cower loop),
rise back up and fight when the coast is clear. Run once per action tick.
*/
void ScaredyShroom::determine_appearance(bool check_zombie_in_attack_range, bool scared)
{
    if (directory_num == SCAREDYSHROOM_SLEEP_DIRECTORY)
    {
        // coffee bean wake-up: rise from the sleep loop straight into raising
        directory_num = SCAREDYSHROOM_SHEET_DIRECTORY;
        frame = 0;
        state_ticks = 0;
        return;
    }
    if (state_ticks > 0)
    {
        // One-shot segments: reach their end before the next transition.
        if (--state_ticks > 0)
            return;
        if (directory_num == SCAREDYSHROOM_SCARED_DIRECTORY)
        {
            directory_num = SCAREDYSHROOM_COWER_DIRECTORY;
            frame = 0;
        }
        else if (directory_num == SCAREDYSHROOM_RAISE_DIRECTORY)
        {
            directory_num = SCAREDYSHROOM_SHEET_DIRECTORY;
            frame = 0;
        }
        return;
    }
    if (scared)
    {
        if (directory_num == SCAREDYSHROOM_SHEET_DIRECTORY ||
            directory_num == SCAREDYSHROOM_ATTACK_DIRECTORY)
        {
            directory_num = SCAREDYSHROOM_SCARED_DIRECTORY;
            frame = 0;
            state_ticks = SCARED_ENTER_TICKS;
        }
        return; // already cowering
    }
    if (directory_num == SCAREDYSHROOM_COWER_DIRECTORY)
    {
        directory_num = SCAREDYSHROOM_RAISE_DIRECTORY;
        frame = 0;
        state_ticks = SCARED_RAISE_TICKS;
        return;
    }
    if (check_zombie_in_attack_range)
    {
        if (directory_num == SCAREDYSHROOM_SHEET_DIRECTORY && frame == 0)
            directory_num = SCAREDYSHROOM_ATTACK_DIRECTORY;
    }
    else if (directory_num == SCAREDYSHROOM_ATTACK_DIRECTORY && frame == 0)
        directory_num = SCAREDYSHROOM_SHEET_DIRECTORY;
}

/*
Attack loop = one spore per 90 ticks (original mLaunchRate 150 cs for
shooters); wrap the attack reel there instead of riding the full sheet.
*/
void ScaredyShroom::next_frame(const PlantAssets &assets)
{
    next_frame_capped(assets, SCAREDYSHROOM_ATTACK_DIRECTORY, 90);
}

/*
Display scaredy-shroom in row. Reanim-only art (2Pak ScaredyShroom.reanim).
*/
void ScaredyShroom::display(const int &_row)
{
    if (row != _row)
        return;
    if (display_reanim(SCAREDYSHROOM_ASSET))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        next_frame(SCAREDYSHROOM_ASSET);
        return;
    }
    win.draw_png_width_scaled(SCAREDYSHROOM_DIRECTORY, cells[row][col].x1 + 10,
                              (cells[row][col].y1 + roof_tile_dy(col)) + 30, world_px(ELEMENT_WIDTH - 30));
    next_frame(SCAREDYSHROOM_ASSET);
}
