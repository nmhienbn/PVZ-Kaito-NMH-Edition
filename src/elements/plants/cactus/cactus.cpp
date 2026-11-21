#include "cactus.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/collision/AttackPlantVsZombie.hpp"
#include "elements/elements.hpp"
#include "elements/zombies/zombie_struct.hpp"

// Cactus.reanim (fps 12 == 5 ticks/frame): idle 0..33, shooting 34..51,
// rise 52..66, idlehigh 67..81, shootinghigh 82..95, lower 96..108.
#define CACTUS_ASSET_FRAMES 5
#define CACTUS_FIRE_FRAME 7

static PlantAssets CACTUS_ASSET(CACTUS_ASSET_FRAMES, 80, 90, 80, 90, 6, -4);

extern int game_state;
extern Map cells;
extern Window win;
extern Elements game_characters;

Cactus::Cactus(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = CACTUS_TYPE;
    health = PLANT_HEALTH_LIMIT[CACTUS_TYPE];
    sec_for_prepare = 1;
    directory_num = CACTUS_SHEET_DIRECTORY;
    attack_range = ONE_ROW_AHEAD;
}

Cactus::~Cactus()
{
}

/*
A flying balloon zombie ahead of the cactus counts as a stretch target (the
clone approximates the original's attack rect, repvz Plant.cpp GetPlantAttackRect
default `Rect(mX + 60, mY, BOARD_WIDTH, mHeight)`, with its own row and column,
and a popped balloon no longer flies, so the stretch drops with it).
*/
bool Cactus::balloon_in_row(vector<Zombie *> &zombies) const
{
    for (auto &z : zombies)
        if (!z->is_charmed && z->type == BALLOON_TYPE && z->is_flying() &&
            z->row == row && z->x_location > cells[row][col].x1 &&
            z->x_location < WINDOW_WIDTH)
            return true;
    return false;
}

/*One spike per attack loop, pea fire-point timing.*/
void Cactus::fire_spike(vector<Bullet *> &bullets)
{
    if (frame != CACTUS_FIRE_FRAME * CACTUS_ASSET_FRAMES)
        return;
    if (directory_num == CACTUS_ATTACK_DIRECTORY)
    {
        play_sound_effect(FIRE_PEA_MUSIC_DIRECTORY);
        bullets.push_back(new Spike(row, cells[row][col].x2 - 10));
    }
    else if (directory_num == CACTUS_ATTACK_HIGH_DIRECTORY)
    {
        play_sound_effect(FIRE_PEA_MUSIC_DIRECTORY);
        bullets.push_back(new Spike(row, cells[row][col].x2 - 10, -30));
    }
}

/*State machine: LOW <-> (RISING -> HIGH -> LOWERING) while a balloon is in
the lane; between those, swap idle/attack sheets like a peashooter.*/
void Cactus::determine_appearance(bool ground_target, bool balloon_target)
{
    bool high = directory_num == CACTUS_IDLE_HIGH_DIRECTORY ||
                directory_num == CACTUS_ATTACK_HIGH_DIRECTORY;
    // repvz Plant.cpp:1742-1748 (UpdateCactus, STATE_CACTUS_HIGH): a high
    // cactus lowers the moment its flying-target scan comes up empty --
    // ground zombies do not hold it up. Don't wait out the shooting loop,
    // so the pop reaction reads as immediate; restarting the frame counter
    // keeps the sprite-sheet fallback in range.
    if (high && !balloon_target)
    {
        directory_num = CACTUS_LOWER_DIRECTORY;
        frame = 0;
        return;
    }
    if (frame != 0)
        return; // one-shot transitions happen on loop boundaries
    if (directory_num == CACTUS_RISE_DIRECTORY)
    {
        directory_num = balloon_target ? CACTUS_IDLE_HIGH_DIRECTORY : CACTUS_LOWER_DIRECTORY;
        return;
    }
    if (directory_num == CACTUS_LOWER_DIRECTORY)
    {
        directory_num = CACTUS_SHEET_DIRECTORY;
        return;
    }
    if (balloon_target &&
        (directory_num == CACTUS_SHEET_DIRECTORY || directory_num == CACTUS_ATTACK_DIRECTORY))
    {
        directory_num = CACTUS_RISE_DIRECTORY;
        return;
    }
    if (high)
    {
        directory_num = CACTUS_ATTACK_HIGH_DIRECTORY;
        return;
    }
    // Low state: ground work like a peashooter.
    if (ground_target && directory_num == CACTUS_SHEET_DIRECTORY)
        directory_num = CACTUS_ATTACK_DIRECTORY;
    else if (!ground_target && directory_num == CACTUS_ATTACK_DIRECTORY)
        directory_num = CACTUS_SHEET_DIRECTORY;
}

void Cactus::action()
{
    fire_spike(game_characters.bullets);
    determine_appearance(check_in_attack_range(*this, game_characters.zombies),
                         balloon_in_row(game_characters.zombies));
}

void Cactus::display(const int &_row)
{
    if (row != _row)
        return;
    if (display_reanim(CACTUS_ASSET))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        next_frame(CACTUS_ASSET);
        return;
    }
    win.draw_png_width_scaled(CACTUS_DIRECTORY, cells[row][col].x1 + 5,
                              (cells[row][col].y1 + roof_tile_dy(col)) + 5, world_px(ELEMENT_WIDTH - 20));
    next_frame(CACTUS_ASSET);
}
