#pragma once
#include "game_param.hpp" // enum PlantType
#include <string>
using namespace std;

/*
Single source of truth for plant name <-> type, mirroring
zombies_registry.hpp: the level loader, the dat-file loader (plants.json is
keyed by these names), the LevelSelector reward icon, and the almanac display
names all derive from this table. Add/rename a plant HERE only.

    type    = PlantType value (game_param.hpp enum)
    key     = name used in level JSONs and plants.json
    display = almanac display name
*/
struct PlantDef
{
    int type;
    const char *key;
    const char *display;
};

// One row per PlantType, in enum order (PEASHOOTER_TYPE = 0 ..
// MELONPULT_TYPE = PLANT_COUNT-1).
inline constexpr PlantDef PLANT_DEFS[] = {
    {PEASHOOTER_TYPE, "peashooter", "Peashooter"},
    {SUNFLOWER_TYPE, "sunflower", "Sunflower"},
    {WALLNUT_TYPE, "wallnut", "Wall-nut"},
    {SNOWPEA_TYPE, "snowpea", "Snow Pea"},
    {POTATOMINE_TYPE, "potato_mine", "Potato Mine"},
    {CHERRYBOMB_TYPE, "cherrybomb", "Cherry Bomb"},
    {REPEATER_TYPE, "repeater", "Repeater"},
    {CHOMPER_TYPE, "chomper", "Chomper"},
    {PUFFSHROOM_TYPE, "puffshroom", "Puff-shroom"},
    {SUNSHROOM_TYPE, "sunshroom", "Sun-shroom"},
    {FUMESHROOM_TYPE, "fumeshroom", "Fume-shroom"},
    {GRAVEBUSTER_TYPE, "gravebuster", "Grave Buster"},
    {HYPNOSHROOM_TYPE, "hypnoshroom", "Hypno-shroom"},
    {SCAREDYSHROOM_TYPE, "scaredyshroom", "Scaredy-shroom"},
    {ICESHROOM_TYPE, "iceshroom", "Ice-shroom"},
    {DOOMSHROOM_TYPE, "doomshroom", "Doom-shroom"},
    {LILYPAD_TYPE, "lilypad", "Lily Pad"},
    {SQUASH_TYPE, "squash", "Squash"},
    {THREEPEATER_TYPE, "threepeater", "Threepeater"},
    {TANGLEKELP_TYPE, "tanglekelp", "Tangle Kelp"},
    {JALAPENO_TYPE, "jalapeno", "Jalapeno"},
    {SPIKEWEED_TYPE, "spikeweed", "Spikeweed"},
    {TORCHWOOD_TYPE, "torchwood", "Torchwood"},
    {TALLNUT_TYPE, "tallnut", "Tall-nut"},
    {SEASHROOM_TYPE, "seashroom", "Sea-shroom"},
    {PLANTERN_TYPE, "plantern", "Plantern"},
    {CACTUS_TYPE, "cactus", "Cactus"},
    {BLOVER_TYPE, "blover", "Blover"},
    {SPLITPEA_TYPE, "splitpea", "Split Pea"},
    {STARFRUIT_TYPE, "starfruit", "Starfruit"},
    {PUMPKIN_TYPE, "pumpkin", "Pumpkin"},
    {MAGNETSHROOM_TYPE, "magnetshroom", "Magnet-shroom"},
    {CABBAGEPULT_TYPE, "cabbagepult", "Cabbage-pult"},
    {FLOWERPOT_TYPE, "flowerpot", "Flower Pot"},
    {KERNELPULT_TYPE, "kernelpult", "Kernel-pult"},
    {COFFEEBEAN_TYPE, "coffeebean", "Coffee Bean"},
    {GARLIC_TYPE, "garlic", "Garlic"},
    {UMBRELLALEAF_TYPE, "umbrellaleaf", "Umbrella Leaf"},
    {MARIGOLD_TYPE, "marigold", "Marigold"},
    {MELONPULT_TYPE, "melonpult", "Melon-pult"},
};
inline constexpr int PLANT_DEF_COUNT = sizeof(PLANT_DEFS) / sizeof(PLANT_DEFS[0]);
static_assert(PLANT_DEF_COUNT == PLANT_COUNT,
              "PLANT_DEFS must have exactly one row per PlantType");
constexpr bool plant_defs_cover_all_types()
{
    for (int i = 0; i < PLANT_DEF_COUNT; i++)
        if (PLANT_DEFS[i].type != i)
            return false;
    return true;
}
static_assert(plant_defs_cover_all_types(),
              "PLANT_DEFS rows must be in enum order, one per type");

// nullptr when the name/type is unknown; callers decide the failure policy.
inline const PlantDef *plant_def_by_key(const string &key)
{
    for (const auto &def : PLANT_DEFS)
        if (key == def.key)
            return &def;
    return nullptr;
}

inline const PlantDef *plant_def_by_type(int type)
{
    for (const auto &def : PLANT_DEFS)
        if (def.type == type)
            return &def;
    return nullptr;
}
