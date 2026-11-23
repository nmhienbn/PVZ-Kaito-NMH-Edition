#include "zombie_type.hpp"
// Concrete subclass headers: only the factory table below needs them, so the
// whole set lives here instead of polluting every zombie_type.hpp includer.
#include "../boss/boss_zombie.hpp"
#include "armored/armored_zombie.hpp"    // ConeZombie / BucketZombie / DoorZombie
#include "balloon/balloon_zombie.hpp"
#include "bobsled/bobsled_zombie.hpp"
#include "bungee/bungee_zombie.hpp"
#include "catapult/catapult_zombie.hpp"
#include "dancer/dancer_zombie.hpp"
#include "digger/digger_zombie.hpp"
#include "dolphinrider/dolphinrider_zombie.hpp"
#include "duckytube/duckytube_zombie.hpp"
#include "flag/flag_zombie.hpp"
#include "football/football_zombie.hpp"
#include "gargantuar/gargantuar_zombie.hpp"
#include "imp/imp_zombie.hpp"
#include "jackbox/jackbox_zombie.hpp"
#include "ladder/ladder_zombie.hpp"
#include "newspaper/newspaper_zombie.hpp"
#include "normal/normal_zombie.hpp"
#include "pogo/pogo_zombie.hpp"
#include "polevault/polevault_zombie.hpp"
#include "snorkel/snorkel_zombie.hpp"
#include "yeti/yeti_zombie.hpp"
#include "zomboni/zomboni_zombie.hpp"
#include "zombie_factory.hpp"
#include "draw/ImageType.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "game_param.hpp"
#include "support.hpp"

/*Spawn table: type code -> constructor, built straight off the
ZOMBIE_FACTORY_X manifest. Compile-time guards: exactly one row per
ZombieType, in enum order (a forgotten registration fails the build, so the
runtime exit(1234) below can only hold off an out-of-range type code).*/
template <class T>
static Zombie *spawn_zombie()
{
    return new T;
}

using ZombieSpawnFn = Zombie *(*)();
#define ZF_SPAWN_ROW(TYPE, CLASS) spawn_zombie<CLASS>,
static ZombieSpawnFn const ZOMBIE_SPAWN_TABLE[] = {ZOMBIE_FACTORY_X(ZF_SPAWN_ROW)};
#undef ZF_SPAWN_ROW

#define ZF_TYPE_ROW(TYPE, CLASS) TYPE,
static constexpr int ZOMBIE_SPAWN_TYPES[] = {ZOMBIE_FACTORY_X(ZF_TYPE_ROW)};
#undef ZF_TYPE_ROW

constexpr bool spawn_table_in_enum_order()
{
    for (int i = 0; i < (int)(sizeof(ZOMBIE_SPAWN_TYPES) / sizeof(ZOMBIE_SPAWN_TYPES[0])); i++)
        if (ZOMBIE_SPAWN_TYPES[i] != i)
            return false;
    return true;
}
static_assert(sizeof(ZOMBIE_SPAWN_TABLE) / sizeof(ZOMBIE_SPAWN_TABLE[0]) == COUNT_ZOMBIE_TYPE,
              "ZOMBIE_FACTORY_X must register exactly one class per ZombieType");
static_assert(spawn_table_in_enum_order(),
              "ZOMBIE_FACTORY_X rows must follow the ZombieType enum order");

static int last_zombie_row = -1;

extern Map cells;
extern Level level;

/*Pool board lane rules, repvz Board::RowCanHaveZombieType (Board.cpp:
2616-2665) + Zombie::IsZombieTypePoolOnly / ZombieTypeCanGoInPool
(Zombie.cpp:8359-8375):
  + the pool-only types (snorkel / dolphin rider / ducky tube) only roll on
    the water lanes, and the zomboni and its bobsled team keep off the water;
  + the other water-capable walkers (normal / cone / bucket / flag -- they
    wear the duck tube on water, Zombie.cpp:1020-1023) may take a water lane
    only from wave 6 on: during the first five waves water lanes spawn
    pool-only types alone;
  + every remaining land type keeps off the water at all times.
wave_idx is the 0-based wave the zombie will spawn with. On boards without
water the playable list passes through untouched (the roll sequence below
then matches the classic board bit for bit).*/
vector<int> lanes_for_zombie(int type, int wave_idx)
{
    vector<int> dry, wet;
    for (int row : level.playable_rows)
        (level.row_has_water(row) ? wet : dry).push_back(row);
    if (wet.empty())
        return level.playable_rows;
    bool pool_only = type == DUCKYTUBE_TYPE || type == SNORKEL_TYPE ||
                     type == DOLPHINRIDER_TYPE;
    bool can_swim = pool_only || type == NORMAL_TYPE || type == CONE_TYPE ||
                    type == BUCKET_TYPE || type == FLAG_TYPE;
    if (pool_only)
        return wet;
    if (dry.empty())
        return level.playable_rows; // degenerate all-water board; unreachable in adventure
    if (can_swim && wave_idx >= FIRST_OPEN_POOL_WAVE)
        return level.playable_rows;
    return dry;
}

/*A normal/cone/bucket zombie placed on a water lane wears the duck tube
(repvz Zombie.cpp:1020-1023: a plain zombie whose row is a pool row reveals
the zombie_duckytube/whitewater tracks of the body reanim). init_zombie wraps
the rows it rolls itself; spawn sites that assign the row afterwards (grave
rise, pool ambush, bungee drop) call this once the row is final. Render-only:
the tube swaps the walk/eat state slot; HP and the armor tier stay with the
original class (the cone/bucket rides armor_dir on top of the tube).*/
void wrap_in_duckytube(Zombie *z)
{
    if (z == nullptr || !level.row_has_water(z->row))
        return;
    if (z->type != NORMAL_TYPE && z->type != CONE_TYPE && z->type != BUCKET_TYPE)
        return;
    if (z->wears_duckytube())
        return;
    z->directory_num = DUCKYTUBE_WALK_DIRECTORY;
}

/*
Init a new zombie with its type.
Depend on level num
*/
Zombie *init_zombie(int _type, int level_num, const int &x_bias, int wave_idx)
{
    Zombie *res = (_type >= 0 && _type < COUNT_ZOMBIE_TYPE)
                      ? ZOMBIE_SPAWN_TABLE[_type]()
                      : nullptr;
    if (res == nullptr)
    {
        cout << "allocate new zombie error! " << _type;
        exit(1234);
    }

    res->type = _type;
    const vector<int> rows = lanes_for_zombie(_type, wave_idx);
    // Random appear row among the lanes this type may enter; avoid repeating the last one.
    if (rows.size() == 1)
    {
        res->row = rows[0];
    }
    else
    {
        int new_row = rows[rand(0, (int)rows.size() - 1)];
        while (new_row == last_zombie_row)
            new_row = rows[rand(0, (int)rows.size() - 1)];
        res->row = new_row;
        last_zombie_row = new_row;
    }
    // Retail spawn x (Zombie.cpp:161): 780 + Rand(40) board-relative. Flag
    // waves add +40 to every follower (Zombie.cpp:235); level_data passes
    // that as x_bias. Type-specific spawns override this below with their own
    // absolute x (the original switch runs after the flag-wave bump too).
    res->x_location = BOARD_OFFSET_X + 780 + rand(0, 39) + x_bias;
    switch (_type)
    {
    case POLEVAULT_TYPE:
        res->x_location = BOARD_OFFSET_X + 870 + rand(0, 9); // 800+70+Rand(10)
        break;
    case GARGANTUAR_TYPE:
        res->x_location = BOARD_OFFSET_X + 845 + rand(0, 9); // 800+45+Rand(10)
        break;
    case ZOMBONI_TYPE:
        res->x_location = BOARD_OFFSET_X + 800 + rand(0, 9); // 800+Rand(10)
        break;
    case CATAPULT_TYPE:
        res->x_location = BOARD_OFFSET_X + 825 + rand(0, 9); // 800+25+Rand(10)
        break;
    case BOBSLED_TYPE:
        res->x_location = BOARD_OFFSET_X + 880; // 800+80
        break;
    case FLAG_TYPE:
        res->x_location = BOARD_OFFSET_X + 800;
        break;
    default:
        break;
    }
    res->y_location = cells[res->row][0].y1 - 50;
    // Set init status
    res->is_moving = true;
    res->attacked_time = res->cold_time = 0;
    res->bite_time = BITE_CLK_COUNT;
    res->next_step_time = 0;

    res->init();
    // Per-zombie walking-speed roll (repvz PickRandomSpeed, called from the
    // spawn site so the virtual override of the fully-built subclass runs).
    res->pick_speed();
    // Water lane landed: any plain walker (normal/cone/bucket) boards its
    // duck tube -- same body and stats, the reanim just unhides the tube.
    wrap_in_duckytube(res);
    // Big Trouble Little Zombie (3-5): half-size zombies with quarter health
    // (repvz ZombieInitialize, Zombie.cpp:862-867: every pool /4). health_scale
    // keeps every HP-derived threshold (degrade tiers, armor drop, arm pops)
    // on the same /4 footing relative to this zombie's own max health.
    if (level.is_special(LevelSpecial::BigTrouble))
    {
        res->health_scale = 0.25f;
        res->health *= res->health_scale;
    }
    // Random first frame.
    res->frame = rand(0, ZOMBIE_FRAME * all_img[res->directory_num].n_sheet - 1);
    return res;
}

Zombie *init_zombie(int _type, int level_num, const int &x_bias)
{
    return init_zombie(_type, level_num, x_bias, FIRST_OPEN_POOL_WAVE);
}

Zombie *init_zombie(int _type, int level_num)
{
    return init_zombie(_type, level_num, 0, FIRST_OPEN_POOL_WAVE);
}