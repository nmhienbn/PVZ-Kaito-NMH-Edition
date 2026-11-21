#include "hypnoshroom.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"

//   anim_idle 0..58, anim_sleep 59..78 (reanim fps 12 == 5 ticks/frame)
#define HYPNOSHROOM_ASSET_FRAMES 5

static PlantAssets HYPNOSHROOM_ASSET(HYPNOSHROOM_ASSET_FRAMES, 56, 66, 56, 66, 12, 20);

extern int game_state;
extern Map cells;
extern Window win;
extern Level level;

/*
Hypno-shroom constructor: the bite that would eat it turns the zombie around
instead (the trance hook lives in ZombieVsPlant.cpp::try_hypno_charm).
*/
HypnoShroom::HypnoShroom(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = HYPNOSHROOM_TYPE;
    health = PLANT_HEALTH_LIMIT[HYPNOSHROOM_TYPE];
    directory_num = is_night() ? HYPNOSHROOM_SHEET_DIRECTORY : HYPNOSHROOM_SLEEP_DIRECTORY;
    attack_range = NO_ATTACK;
}

HypnoShroom::~HypnoShroom()
{
}

bool HypnoShroom::is_night() const
{
    return level.is_night() || coffee_awake; // a coffee bean wakes it by day
}

/*
Display hypno-shroom in row. Reanim-only art (2Pak HypnoShroom.reanim).
*/
void HypnoShroom::display(const int &_row)
{
    if (row != _row)
        return;
    if (display_reanim(HYPNOSHROOM_ASSET))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        next_frame(HYPNOSHROOM_ASSET);
        return;
    }
    win.draw_png_width_scaled(HYPNOSHROOM_DIRECTORY, cells[row][col].x1 + 10,
                              (cells[row][col].y1 + roof_tile_dy(col)) + 30, world_px(ELEMENT_WIDTH - 30));
    next_frame(HYPNOSHROOM_ASSET);
}

/*
Hypno-shroom's action: nothing on its own; the trance fires when it is bitten.
*/
void HypnoShroom::action()
{
    if (is_night() && directory_num == HYPNOSHROOM_SLEEP_DIRECTORY)
    {
        // coffee bean wake-up: rise from the sleep loop to the idle reel
        directory_num = HYPNOSHROOM_SHEET_DIRECTORY;
        frame = 0;
    }
}
