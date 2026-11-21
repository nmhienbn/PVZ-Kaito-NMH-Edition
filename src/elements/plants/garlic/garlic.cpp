#include "garlic.hpp"
#include "draw/world_scale.hpp"
#include "elements/Map/Map.hpp"
#include "elements/Level/Level.hpp" // roof_tile_dy
#include "elements/zombies/zombie_struct.hpp"

// Garlic.reanim: anim_idle 138..234 (fps 12). The icon dir paces stage 1
// (long chew); the two hurt stages share the 97-frame idle window.
static PlantAssets GARLIC_ASSET(2, 64, 64, 64, 64, 8, 22);
static PlantAssets GARLIC_HURT_ASSET(2, 64, 64, 64, 64, 8, 22);

extern int game_state;
extern Map cells;
extern Window win;
extern Level level;

Garlic::Garlic(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = GARLIC_TYPE;
    health = PLANT_HEALTH_LIMIT[GARLIC_TYPE]; // 8 bites (orig 400 health at 4 dmg/4 cs chews)
    sec_for_prepare = 1;
    directory_num = GARLIC_DIRECTORY;
    attack_range = NO_ATTACK;
}

Garlic::~Garlic()
{
}

/*Pick the look for the remaining bite count: 3 stages at the 2/3 and 1/3 marks.*/
int Garlic::stage_dir() const
{
    if (health > GARLIC_STAGE2_HEALTH)
        return GARLIC_DIRECTORY;
    if (health > GARLIC_STAGE3_HEALTH)
        return GARLIC_HURT1_DIRECTORY;
    return GARLIC_HURT2_DIRECTORY;
}

const PlantAssets &Garlic::stage_assets() const
{
    return health > GARLIC_STAGE2_HEALTH ? GARLIC_ASSET : GARLIC_HURT_ASSET;
}

void Garlic::action()
{
}

// ---- The yucky beat (repvz Zombie.cpp:4851-4858 + UpdateYuckyFace,
// 4738-4824, 100 cs/s) -----------------------------------------------
// Tick figures below are the original's centisecond counts x0.6 (60 tps
// clone): face/gag at 70 -> 42 ticks, divert at 170 -> 102, face clear at
// 270 -> 162 (the clear would only unhide the gross-out face image, which
// this art set does not ship).
#define GARLIC_YUCKY_CHEW_TICKS 42 // still chewing, no face yet
#define GARLIC_YUCKY_DIVERT_TICKS 102
#define GARLIC_YUCKY_DONE_TICKS 162

namespace
{
struct YuckyZombie
{
    Zombie *zombie;
    int ticks; // 0 up to GARLIC_YUCKY_DONE_TICKS
};
vector<YuckyZombie> yucky_zombies;

YuckyZombie *yucky_of(const Zombie *zombie)
{
    for (auto &yz : yucky_zombies)
        if (yz.zombie == zombie)
            return &yz;
    return nullptr;
}

bool yucky_row_free(const Zombie &zombie, int new_row)
{
    if (new_row < 0 || new_row >= (int)cells.size())
        return false;
    // Original diverts only to a same-terrain row (UpdateYuckyFace's pool-row
    // parity check): a land zombie never gags into the pool and back.
    return level.row_has_water(new_row) == level.row_has_water(zombie.row);
}
} // namespace

void garlic_bite(Zombie &zombie)
{
    if (yucky_of(&zombie))
        return; // already gagging: keep the timer going (if (!mYuckyFace))
    yucky_zombies.push_back({&zombie, 0});
}

bool garlic_gagging(const Zombie *zombie)
{
    const YuckyZombie *yz = yucky_of(zombie);
    return yz && yz->ticks >= GARLIC_YUCKY_CHEW_TICKS &&
           yz->ticks < GARLIC_YUCKY_DIVERT_TICKS;
}

void update_garlic_yucky(vector<Zombie *> &zombies)
{
    for (int i = (int)yucky_zombies.size() - 1; i >= 0; i--)
    {
        Zombie *z = yucky_zombies[i].zombie;
        bool alive = false;
        for (auto *candidate : zombies)
            if (candidate == z)
            {
                alive = true;
                break;
            }
        if (!alive || ++yucky_zombies[i].ticks >= GARLIC_YUCKY_DONE_TICKS)
        {
            yucky_zombies.erase(yucky_zombies.begin() + i);
            continue;
        }
        if (yucky_zombies[i].ticks >= GARLIC_YUCKY_CHEW_TICKS &&
            yucky_zombies[i].ticks < GARLIC_YUCKY_DIVERT_TICKS)
        {
            // StopEating + hold: no bites, no walking during the gag face.
            z->is_moving = false;
            if (z->bite_time < 2)
                z->bite_time = 2;
            continue;
        }
        if (yucky_zombies[i].ticks == GARLIC_YUCKY_DIVERT_TICKS)
        {
            // SetRow: up/down at random among the lanes this zombie may enter
            // (no divert possible stays put and chews on).
            bool can_up = yucky_row_free(*z, z->row - 1);
            bool can_down = yucky_row_free(*z, z->row + 1);
            if (can_up && can_down)
                z->row += rand(0, 1) ? -1 : 1;
            else if (can_up)
                z->row--;
            else if (can_down)
                z->row++;
            if (can_up || can_down)
                z->y_location = cells[z->row][0].y1 - 50;
            // StartWalkAnim(20): walking resumes from here.
            z->is_moving = true;
            z->bite_time = BITE_CLK_COUNT;
        }
    }
}

void Garlic::display(const int &_row)
{
    if (row != _row)
        return;
    directory_num = stage_dir();
    const PlantAssets &assets = stage_assets();
    if (display_reanim(assets))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        next_frame(assets);
        return;
    }
    win.draw_png_width_scaled(GARLIC_DIRECTORY, cells[row][col].x1 + 10,
                              (cells[row][col].y1 + roof_tile_dy(col)) + 28, world_px(ELEMENT_WIDTH - 30));
    next_frame(assets);
}
