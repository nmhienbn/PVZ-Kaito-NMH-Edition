#include "seashroom.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/collision/AttackPlantVsZombie.hpp"
#include "elements/elements.hpp"

// Reanim fps 12 == 5 ticks/frame; SeaShroom.reanim: idle 0..17,
// shooting 18..31, sleep 32..49.
#define SEASHROOM_ASSET_FRAMES 5
// Spore leaves the cap around the middle of anim_shooting.
#define SEASHROOM_FIRE_FRAME 7

static PlantAssets SEASHROOM_ASSET(SEASHROOM_ASSET_FRAMES, 68, 68, 68, 68, 8, 14);

extern int game_state;
extern Map cells;
extern Window win;
extern Level level;
extern Elements game_characters;

SeaShroom::SeaShroom(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = SEASHROOM_TYPE;
    health = PLANT_HEALTH_LIMIT[SEASHROOM_TYPE];
    sec_for_prepare = 1;
    directory_num = is_night() ? SEASHROOM_SHEET_DIRECTORY : SEASHROOM_SLEEP_DIRECTORY;
    attack_range = ONE_ROW_AHEAD;
}

SeaShroom::~SeaShroom()
{
}

bool SeaShroom::is_night() const
{
    return level.is_night() || coffee_awake; // a coffee bean wakes it by day
}

/*
Create new spore puff from a sea-shroom (same short-range spore as the
puff-shroom's).
*/
void SeaShroom::fire_puff(vector<Bullet *> &bullets)
{
    if (directory_num == SEASHROOM_ATTACK_DIRECTORY && frame == SEASHROOM_FIRE_FRAME * SEASHROOM_ASSET_FRAMES)
    {
        play_sound_effect(PUFF_MUSIC_DIRECTORY);
        bullets.push_back(new Puff(row, cells[row][col].x2 - 20));
    }
}

void SeaShroom::determine_appearance(bool check_zombie_in_attack_range)
{
    if (!is_night())
        return; // sleeping in daylight
    if (directory_num == SEASHROOM_SLEEP_DIRECTORY)
    {
        // coffee bean wake-up: rise from the sleep loop to the idle reel
        directory_num = SEASHROOM_SHEET_DIRECTORY;
        frame = 0;
    }
    if (check_zombie_in_attack_range)
    {
        if (directory_num == SEASHROOM_SHEET_DIRECTORY && frame == 0)
            directory_num = SEASHROOM_ATTACK_DIRECTORY;
    }
    else
    {
        if (directory_num == SEASHROOM_ATTACK_DIRECTORY && frame == 0)
            directory_num = SEASHROOM_SHEET_DIRECTORY;
    }
}

/*
Attack loop = one spore per 90 ticks (original mLaunchRate 150 cs for
shooters); wrap the attack reel there instead of riding the full sheet.
*/
void SeaShroom::next_frame(const PlantAssets &assets)
{
    next_frame_capped(assets, SEASHROOM_ATTACK_DIRECTORY, 90);
}

/*Like the puff-shroom: spits short-range spores, but only at night and only
standing in water.*/
void SeaShroom::action()
{
    if (is_night())
    {
        fire_puff(game_characters.bullets);
        determine_appearance(check_in_attack_range(*this, game_characters.zombies));
    }
}

void SeaShroom::display(const int &_row)
{
    if (row != _row)
        return;
    if (display_reanim(SEASHROOM_ASSET))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        next_frame(SEASHROOM_ASSET);
        return;
    }
    win.draw_png_width_scaled(SEASHROOM_DIRECTORY, cells[row][col].x1 + 10,
                              (cells[row][col].y1 + roof_tile_dy(col)) + 35, world_px(ELEMENT_WIDTH - 30));
    next_frame(SEASHROOM_ASSET);
}
