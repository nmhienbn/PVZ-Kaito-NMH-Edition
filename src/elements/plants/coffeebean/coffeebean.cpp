#include "coffeebean.hpp"
#include "draw/world_scale.hpp"
#include "elements/Map/Map.hpp"
#include "elements/elements.hpp"
#include "music/music.hpp"
#include "elements/Level/Level.hpp" // roof_tile_dy

// Coffeebean.reanim: anim_idle 0..82, anim_crumble 83..111 (fps 12).
static PlantAssets COFFEEBEAN_ASSET(5, 60, 46, 60, 46, 5, -6);
static PlantAssets COFFEEBEAN_CRUMBLE_ASSET(5, 60, 46, 60, 46, 5, -6);

// Steam ~1 s above the host (repvz mWakeUpCounter = 100 cs, Plant.cpp:4456),
// crumble in place, then the host wakes.
static const int COFFEE_BEAN_WAKE_CLK = 60;

extern int game_state;
extern Map cells;
extern Window win;
extern Elements game_characters;

/*Mushroom set that dozes in daylight (night-yard + fog + pool aquatic).*/
bool is_day_sleepy_mushroom(const int &type)
{
    switch (type)
    {
    case PUFFSHROOM_TYPE:
    case SUNSHROOM_TYPE:
    case FUMESHROOM_TYPE:
    case HYPNOSHROOM_TYPE:
    case SCAREDYSHROOM_TYPE:
    case ICESHROOM_TYPE:
    case DOOMSHROOM_TYPE:
    case SEASHROOM_TYPE:
    case MAGNETSHROOM_TYPE:
        return true;
    default:
        return false;
    }
}

CoffeeBean::CoffeeBean(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = COFFEEBEAN_TYPE;
    health = PLANT_HEALTH_LIMIT[COFFEEBEAN_TYPE];
    sec_for_prepare = 1;
    directory_num = COFFEEBEAN_DIRECTORY;
    attack_range = NO_ATTACK;
    wake_clk = COFFEE_BEAN_WAKE_CLK;
    play_sound_effect(COFFEE_MUSIC_DIRECTORY);
}

CoffeeBean::~CoffeeBean()
{
}

/*
Wake-up: find the mushroom under the bean, flip its coffee_awake flag (its
is_night() checks start passing), then let the bean wither away.
*/
void CoffeeBean::action()
{
    if (directory_num != COFFEEBEAN_CRUMBLE_DIRECTORY)
    {
        if (--wake_clk <= 0)
        {
            directory_num = COFFEEBEAN_CRUMBLE_DIRECTORY;
            frame = 0;
        }
        return;
    }
    // Crumble one-shot finished: wake the host and wilt.
    if (frame == 0)
    {
        for (auto *p : game_characters.plants)
            if (p != this && p->get_row() == row && p->get_col() == col &&
                is_day_sleepy_mushroom(p->get_type()))
            {
                p->coffee_awake = true;
                p->set_attacked_time(MAX_TIME_BLINK); // wake flash
                play_sound_effect(WAKEUP_MUSIC_DIRECTORY);
                break;
            }
        health = 0; // the bean is spent
    }
}

void CoffeeBean::display(const int &_row)
{
    if (row != _row)
        return;
    bool crumbling = directory_num == COFFEEBEAN_CRUMBLE_DIRECTORY;
    const PlantAssets &assets = crumbling ? COFFEEBEAN_CRUMBLE_ASSET : COFFEEBEAN_ASSET;
    if (display_reanim(assets))
    {
        next_frame(assets);
        return;
    }
    win.draw_png_width_scaled(COFFEEBEAN_DIRECTORY, cells[row][col].x1 + 20,
                              (cells[row][col].y1 + roof_tile_dy(col)) + 5, world_px(ELEMENT_WIDTH - 50));
    next_frame(assets);
}
