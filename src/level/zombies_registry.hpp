#pragma once
#include "game_param.hpp" // enum ZombieType
#include <string>
using namespace std;

/*
Single source of truth for zombie name <-> type. Every name table in the
codebase derives from this one: the wave generator and the level loader
(zombie_type_from_name / zombie_type_name), the dat-file loader
(zombies.json is keyed by these names), the LevelSelector icon fallback, and
the almanac display names. Add/rename a zombie HERE only.

    type    = ZombieType value (game_param.hpp enum)
    key     = name used in level JSONs and zombies.json
    display = almanac display name
*/
struct ZombieDef
{
    int type;
    const char *key;
    const char *display;
};

// One row per ZombieType, in enum order (the static_assert below enforces it:
// YETI_TYPE stays the appended last entry, see the note in game_param.hpp).
inline constexpr ZombieDef ZOMBIE_DEFS[] = {
    {NORMAL_TYPE, "normal", "Zombie"},
    {FLAG_TYPE, "flag", "Flag Zombie"},
    {CONE_TYPE, "conehead", "Conehead Zombie"},
    {BUCKET_TYPE, "buckethead", "Buckethead Zombie"},
    {DOOR_TYPE, "door", "Screen Door Zombie"},
    {POLEVAULT_TYPE, "polevault", "Pole Vaulting Zombie"},
    {NEWSPAPER_TYPE, "newspaper", "Newspaper Zombie"},
    {FOOTBALL_TYPE, "football", "Football Zombie"},
    {DANCER_TYPE, "dancer", "Dancing Zombie"},
    {BACKUP_TYPE, "backup", "Backup Dancer"},
    {DUCKYTUBE_TYPE, "duckytube", "Ducky Tube Zombie"},
    {SNORKEL_TYPE, "snorkel", "Snorkel Zombie"},
    {ZOMBONI_TYPE, "zomboni", "Zomboni"},
    {BOBSLED_TYPE, "bobsled", "Zombie Bobsled Team"},
    {DOLPHINRIDER_TYPE, "dolphinrider", "Dolphin Rider Zombie"},
    {JACKBOX_TYPE, "jackbox", "Jack-in-the-Box Zombie"},
    {BALLOON_TYPE, "balloon", "Balloon Zombie"},
    {DIGGER_TYPE, "digger", "Digger Zombie"},
    {POGO_TYPE, "pogo", "Pogo Zombie"},
    {BUNGEE_TYPE, "bungee", "Bungee Zombie"},
    {LADDER_TYPE, "ladder", "Ladder Zombie"},
    {CATAPULT_TYPE, "catapult", "Catapult Zombie"},
    {GARGANTUAR_TYPE, "gargantuar", "Gargantuar"},
    {IMP_TYPE, "imp", "Imp"},
    {ZOMBOSS_TYPE, "zomboss", "Dr. Zomboss"},
    {YETI_TYPE, "yeti", "Zombie Yeti"},
};
inline constexpr int ZOMBIE_DEF_COUNT = sizeof(ZOMBIE_DEFS) / sizeof(ZOMBIE_DEFS[0]);
static_assert(ZOMBIE_DEF_COUNT == COUNT_ZOMBIE_TYPE,
              "ZOMBIE_DEFS must have exactly one row per ZombieType");
constexpr bool zombie_defs_cover_all_types()
{
    for (int i = 0; i < ZOMBIE_DEF_COUNT; i++)
        if (ZOMBIE_DEFS[i].type != i)
            return false;
    return true;
}
static_assert(zombie_defs_cover_all_types(),
              "ZOMBIE_DEFS rows must be in enum order, one per type");

// nullptr when the name/type is unknown; callers decide the failure policy.
inline const ZombieDef *zombie_def_by_key(const string &key)
{
    for (const auto &def : ZOMBIE_DEFS)
        if (key == def.key)
            return &def;
    return nullptr;
}

inline const ZombieDef *zombie_def_by_type(int type)
{
    for (const auto &def : ZOMBIE_DEFS)
        if (def.type == type)
            return &def;
    return nullptr;
}
