#include "starfruit.hpp"
#include "draw/world_scale.hpp"
#include "elements/Map/Map.hpp"
#include "elements/elements.hpp"
#include "elements/zombies/zombie_struct.hpp"
#include "game_param.hpp" // PEA_DX
#include <cstdlib>
#include "elements/Level/Level.hpp" // roof_tile_dy

// Starfruit.reanim (fps 12 == 5 ticks/frame): idle 0..24, shoot 25..42.
#define STARFRUIT_ASSET_FRAMES 5
#define STARFRUIT_FIRE_FRAME 8

static PlantAssets STARFRUIT_ASSET(STARFRUIT_ASSET_FRAMES, 62, 62, 62, 62, 5, 10);

extern int game_state;
extern Map cells;
extern Window win;
extern Elements game_characters;

Starfruit::Starfruit(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = STARFRUIT_TYPE;
    health = PLANT_HEALTH_LIMIT[STARFRUIT_TYPE];
    sec_for_prepare = 1;
    directory_num = STARFRUIT_SHEET_DIRECTORY;
    attack_range = NO_ATTACK; // targeting is star-shaped, not lane-shaped
}

Starfruit::~Starfruit()
{
}

/*
Clone approximation of the original FindStarFruitTarget: fire when a zombie
    + sits in the same lane behind the star (back-star territory), or
    + is one lane off and ahead (forward ~30 deg diagonals), or
    + is two lanes off near the star's column (vertical stars).
*/
bool Starfruit::find_target(vector<Zombie *> &zombies) const
{
    int cx = (cells[row][col].x1 + cells[row][col].x2) / 2;
    for (auto &z : zombies)
    {
        if (z->is_charmed || z->x_location > WINDOW_WIDTH || z->x_location + 140 < 0)
            continue;
        int zr = z->row;
        int dx = z->x_location + ZOMBIE_EXACT_LOCATION / 2 - cx;
        if (zr == row)
        {
            if (dx < -20) // behind: back-star
                return true;
        }
        else if (abs(zr - row) == 1)
        {
            if (dx > 10) // diagonal forward wedge
                return true;
        }
        else if (abs(zr - row) <= 2 && abs(dx) < BLOCK_WIDTH)
            return true; // vertical star corridor
    }
    return false;
}

/*
Volley of 5 stars (HEAD pacing: star axes fly at pea pace, PEA_DX px/tick,
the two 30deg diagonals split it into (cos30, sin30) components):
backward, straight down, straight up, forward-down 30deg, forward-up 30deg.
*/
void Starfruit::fire_stars(vector<Bullet *> &bullets)
{
    if (directory_num != STARFRUIT_ATTACK_DIRECTORY ||
        frame != STARFRUIT_FIRE_FRAME * STARFRUIT_ASSET_FRAMES)
        return;
    play_sound_effect(FIRE_PEA_MUSIC_DIRECTORY);
    int cx = (cells[row][col].x1 + cells[row][col].x2) / 2;
    int cy = (cells[row][col].y1 + roof_tile_dy(col)) + 25;
    const float diag_x = PEA_DX * 0.866f, diag_y = PEA_DX * 0.5f;
    bullets.push_back(new Star(row, cx - 15, cy, -PEA_DX, 0));
    bullets.push_back(new Star(row, cx, cy, 0, PEA_DX));
    bullets.push_back(new Star(row, cx, cy, 0, -PEA_DX));
    bullets.push_back(new Star(row, cx, cy, diag_x, diag_y));
    bullets.push_back(new Star(row, cx, cy, diag_x, -diag_y));
}

void Starfruit::action()
{
    fire_stars(game_characters.bullets);
    bool target = find_target(game_characters.zombies);
    if (target && directory_num == STARFRUIT_SHEET_DIRECTORY && frame == 0)
        directory_num = STARFRUIT_ATTACK_DIRECTORY;
    else if (!target && directory_num == STARFRUIT_ATTACK_DIRECTORY && frame == 0)
        directory_num = STARFRUIT_SHEET_DIRECTORY;
}

void Starfruit::display(const int &_row)
{
    if (row != _row)
        return;
    if (display_reanim(STARFRUIT_ASSET))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        next_frame(STARFRUIT_ASSET);
        return;
    }
    win.draw_png_width_scaled(STARFRUIT_DIRECTORY, cells[row][col].x1 + 8,
                              (cells[row][col].y1 + roof_tile_dy(col)) + 20, world_px(ELEMENT_WIDTH - 30));
    next_frame(STARFRUIT_ASSET);
}
