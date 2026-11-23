#include "threepeater.hpp"
#include "draw/world_scale.hpp"
#include "elements/Map/Map.hpp"
#include "elements/elements.hpp"
#include "support.hpp"
#include "elements/Level/Level.hpp" // roof_tile_dy

// Reanim-only art (2Pak ThreePeater.reanim); the three heads share the doc:
// the 4..21 sway is every head's idle, 22..33 the shooting flare.
// The attack loop is launch-rate capped (original mLaunchRate 150 cs = 90
// ticks) instead of riding the full sheet.
static PlantAssets THREEPEATER_ASSET(5, 85, 81, 85, 85, 5, -9);
// Fire point inside the shooting segment (pea leaves as the heads snap back).
#define THREEPEATER_FIRE_FRAME 7

extern int game_state;
extern Map cells;
extern Window win;
extern Elements game_characters;

/*
Threepeater constructor
*/
ThreePeater::ThreePeater(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = THREEPEATER_TYPE;
    health = PLANT_HEALTH_LIMIT[THREEPEATER_TYPE];
    sec_for_prepare = 1;
    directory_num = THREEPEATER_SHEET_DIRECTORY;
    attack_range = NO_ATTACK; // 3 lanes handled manually
}

ThreePeater::~ThreePeater()
{
}

/*
Any uncharmed zombie ahead of the plant in the query lane.
*/
bool ThreePeater::has_zombie_ahead(Plants &plant, const int &row)
{
    if (row < 0 || row >= (int)cells.size())
        return false;
    int left_bound = cells[row][col].x2 - 40;
    for (const auto &zombie : game_characters.zombies)
        if (zombie->row == row && !zombie->is_charmed &&
            zombie->x_location + ZOMBIE_EXACT_LOCATION + 15 >= left_bound)
            return true;
    return false;
}

/*
Fire one pea per covered lane: own row plus the row above and below (edge
lanes simply lose the off-board head's pea, like the original).
*/
void ThreePeater::fire_peas(vector<Bullet *> &bullets)
{
    play_sound_effect(FIRE_PEA_MUSIC_DIRECTORY);
    // repvz Plant.cpp:4626: threepeater origin mX + 45 (= cell x1 + 45).
    int x = cells[row][col].x1 + 45;
    if (row > 0)
        bullets.push_back(new Pea(row - 1, x));
    bullets.push_back(new Pea(row, x));
    if (row + 1 < (int)cells.size())
        bullets.push_back(new Pea(row + 1, x));
}

/*
Change threepeater's appearance when any of its lanes has a zombie.
*/
void ThreePeater::determine_appearance(bool check_zombie_in_attack_range)
{
    if (check_zombie_in_attack_range)
    {
        if (directory_num == THREEPEATER_SHEET_DIRECTORY && frame == 0)
            directory_num = THREEPEATER_ATTACK_DIRECTORY;
    }
    else
    {
        if (directory_num == THREEPEATER_ATTACK_DIRECTORY && frame == 0)
            directory_num = THREEPEATER_SHEET_DIRECTORY;
    }
}

/*
Attack loop = one volley per 90 ticks (original mLaunchRate 150 cs for
shooters); wrap the attack reel there instead of riding the full sheet.
*/
void ThreePeater::next_frame(const PlantAssets &assets)
{
    next_frame_capped(assets, THREEPEATER_ATTACK_DIRECTORY, 90);
}

/*
Threepeater's action: pepper the 3 lanes.
*/
void ThreePeater::action()
{
    if (directory_num == THREEPEATER_ATTACK_DIRECTORY &&
        frame == THREEPEATER_FIRE_FRAME * THREEPEATER_ASSET.frame)
        fire_peas(game_characters.bullets);
    bool check_zombie_in_attack_range =
        has_zombie_ahead(*this, row - 1) || has_zombie_ahead(*this, row) || has_zombie_ahead(*this, row + 1);
    determine_appearance(check_zombie_in_attack_range);
}

/*
Display threepeaters in row
*/
void ThreePeater::display(const int &_row)
{
    if (row != _row)
        return;
    if (display_reanim(THREEPEATER_ASSET))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        next_frame(THREEPEATER_ASSET);
        return;
    }
    win.draw_png_width_scaled(THREEPEATER_DIRECTORY, cells[row][col].x1 + 5,
                              (cells[row][col].y2 + roof_tile_dy(col)) - 84, world_px(ELEMENT_WIDTH - 8));
    next_frame(THREEPEATER_ASSET);
}
