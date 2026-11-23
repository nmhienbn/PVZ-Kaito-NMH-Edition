#include "splitpea.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/collision/AttackPlantVsZombie.hpp"
#include "elements/elements.hpp"
#include "elements/zombies/zombie_struct.hpp"

// Splitpea.reanim (fps 12 == 5 ticks/frame) composite windows: leaves 7..23,
// back head idle 24..40 / shooting 41..53, front head idle 54..70 /
// shooting 71..87 (one sheet state paces all three).
#define SPLITPEA_ASSET_FRAMES 5
// Front head spits early in the clip; the rear head doubles up just after.
#define SPLITPEA_FRONT_FIRE_FRAME 4
#define SPLITPEA_BACK_FIRE_FRAME_1 8
#define SPLITPEA_BACK_FIRE_FRAME_2 11

static PlantAssets SPLITPEA_ASSET(SPLITPEA_ASSET_FRAMES, 87, 87, 87, 87, 0, 0);

extern int game_state;
extern Map cells;
extern Window win;
extern Elements game_characters;

SplitPea::SplitPea(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = SPLITPEA_TYPE;
    health = PLANT_HEALTH_LIMIT[SPLITPEA_TYPE];
    sec_for_prepare = 1;
    directory_num = SPLITPEA_SHEET_DIRECTORY;
    attack_range = ONE_ROW_AHEAD; // front head
}

SplitPea::~SplitPea()
{
}

/*
Rear head target: any zombie in the same lane to the LEFT of the plant
(diggers surface behind the front line).
*/
bool SplitPea::zombie_on_left(vector<Zombie *> &zombies) const
{
    for (auto &z : zombies)
        if (!z->is_charmed && z->row == row &&
            z->x_location + ZOMBIE_EXACT_LOCATION < cells[row][col].x1 + 10 &&
            z->x_location > cells[row][0].x1 - 40)
            return true;
    return false;
}

/*
Front head: one pea to the right; rear head: two peas to the left per loop.
*/
void SplitPea::fire_peas(vector<Bullet *> &bullets)
{
    if (directory_num != SPLITPEA_ATTACK_DIRECTORY)
        return;
    if (frame == SPLITPEA_FRONT_FIRE_FRAME * SPLITPEA_ASSET_FRAMES)
    {
        if (check_in_attack_range(*this, game_characters.zombies))
        {
            play_sound_effect(FIRE_PEA_MUSIC_DIRECTORY);
            // Same forward muzzle as the peashooter: mX + headX + 24.
            bullets.push_back(new Pea(row, cells[row][col].x1 + 40));
        }
    }
    if (frame == SPLITPEA_BACK_FIRE_FRAME_1 * SPLITPEA_ASSET_FRAMES ||
        frame == SPLITPEA_BACK_FIRE_FRAME_2 * SPLITPEA_ASSET_FRAMES)
    {
        if (zombie_on_left(game_characters.zombies))
        {
            play_sound_effect(FIRE_PEA_MUSIC_DIRECTORY);
            bullets.push_back(new Pea(row, cells[row][col].x1 + 5, -1));
        }
    }
}

/*
Attack loop = one volley per 90 ticks: the original mLaunchRate for shooters
is 150 cs (repvz Plant.cpp gPlantDefs) = 90 ticks at 60 fps; the 17-frame
sheet would wrap at 85 ticks with plain next_frame.
*/
void SplitPea::next_frame(const PlantAssets &assets)
{
    next_frame_capped(assets, SPLITPEA_ATTACK_DIRECTORY, 90);
}

void SplitPea::determine_appearance(bool front_target, bool back_target)
{
    if ((front_target || back_target) && directory_num == SPLITPEA_SHEET_DIRECTORY && frame == 0)
        directory_num = SPLITPEA_ATTACK_DIRECTORY;
    else if (!front_target && !back_target && directory_num == SPLITPEA_ATTACK_DIRECTORY && frame == 0)
        directory_num = SPLITPEA_SHEET_DIRECTORY;
}

void SplitPea::action()
{
    fire_peas(game_characters.bullets);
    determine_appearance(check_in_attack_range(*this, game_characters.zombies),
                         zombie_on_left(game_characters.zombies));
}

void SplitPea::display(const int &_row)
{
    if (row != _row)
        return;
    if (display_reanim(SPLITPEA_ASSET))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        next_frame(SPLITPEA_ASSET);
        return;
    }
    win.draw_png_width_scaled(SPLITPEA_DIRECTORY, cells[row][col].x1 + 5,
                              (cells[row][col].y1 + roof_tile_dy(col)) + 20, world_px(ELEMENT_WIDTH - 20));
    next_frame(SPLITPEA_ASSET);
}
