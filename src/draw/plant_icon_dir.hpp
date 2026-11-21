#pragma once
#include "ImageType.hpp"
#include "game_param.hpp"

/*
Seed-packet portrait (icon) image directory per PlantType, indexed by the
enum in game_param.hpp (PEASHOOTER_TYPE .. MELONPULT_TYPE). Replaces the old
"PEASHOOTER_DIRECTORY + type" contiguity hack at call sites; to point a plant
at different art, change its row here (one place) instead of hoping the enum
in image_manifest.hpp stays contiguous.

No dependency on src/level/plants_registry.hpp: that registry maps type ->
name/display while this one maps type -> ImageType, and keeping the draw side
ImageType-only avoids an include cycle.
*/
inline constexpr ImageType PLANT_ICON_DIR[PLANT_COUNT] = {
    PEASHOOTER_DIRECTORY,  // PEASHOOTER_TYPE
    SUNFLOWER_DIRECTORY,   // SUNFLOWER_TYPE
    WALLNUT_DIRECTORY,     // WALLNUT_TYPE
    SNOWPEA_DIRECTORY,     // SNOWPEA_TYPE
    POTATOMINE_DIRECTORY,  // POTATOMINE_TYPE
    CHERRYBOMB_DIRECTORY,  // CHERRYBOMB_TYPE
    REPEATER_DIRECTORY,    // REPEATER_TYPE
    CHOMPER_DIRECTORY,     // CHOMPER_TYPE
    PUFFSHROOM_DIRECTORY,  // PUFFSHROOM_TYPE
    SUNSHROOM_DIRECTORY,   // SUNSHROOM_TYPE
    FUMESHROOM_DIRECTORY,  // FUMESHROOM_TYPE
    GRAVEBUSTER_DIRECTORY, // GRAVEBUSTER_TYPE
    HYPNOSHROOM_DIRECTORY, // HYPNOSHROOM_TYPE
    SCAREDYSHROOM_DIRECTORY, // SCAREDYSHROOM_TYPE
    ICESHROOM_DIRECTORY,   // ICESHROOM_TYPE
    DOOMSHROOM_DIRECTORY,  // DOOMSHROOM_TYPE
    LILYPAD_DIRECTORY,     // LILYPAD_TYPE
    SQUASH_DIRECTORY,      // SQUASH_TYPE
    THREEPEATER_DIRECTORY, // THREEPEATER_TYPE
    TANGLEKELP_DIRECTORY,  // TANGLEKELP_TYPE
    JALAPENO_DIRECTORY,    // JALAPENO_TYPE
    SPIKEWEED_DIRECTORY,   // SPIKEWEED_TYPE
    TORCHWOOD_DIRECTORY,   // TORCHWOOD_TYPE
    TALLNUT_DIRECTORY,     // TALLNUT_TYPE
    SEASHROOM_DIRECTORY,   // SEASHROOM_TYPE
    PLANTERN_DIRECTORY,    // PLANTERN_TYPE
    CACTUS_DIRECTORY,      // CACTUS_TYPE
    BLOVER_DIRECTORY,      // BLOVER_TYPE
    SPLITPEA_DIRECTORY,    // SPLITPEA_TYPE
    STARFRUIT_DIRECTORY,   // STARFRUIT_TYPE
    PUMPKIN_DIRECTORY,     // PUMPKIN_TYPE
    MAGNETSHROOM_DIRECTORY,// MAGNETSHROOM_TYPE
    CABBAGEPULT_DIRECTORY, // CABBAGEPULT_TYPE
    FLOWERPOT_DIRECTORY,   // FLOWERPOT_TYPE
    KERNELPULT_DIRECTORY,  // KERNELPULT_TYPE
    COFFEEBEAN_DIRECTORY,  // COFFEEBEAN_TYPE
    GARLIC_DIRECTORY,      // GARLIC_TYPE
    UMBRELLALEAF_DIRECTORY,// UMBRELLALEAF_TYPE
    MARIGOLD_DIRECTORY,    // MARIGOLD_TYPE
    MELONPULT_DIRECTORY,   // MELONPULT_TYPE
};
static_assert(sizeof(PLANT_ICON_DIR) / sizeof(PLANT_ICON_DIR[0]) == PLANT_COUNT,
              "PLANT_ICON_DIR must have exactly one icon per PlantType");

// A few legacy spots outside this header's reach still compute the icon as
// PEASHOOTER_DIRECTORY + type; pin the table to that layout so any reorder in
// image_manifest.hpp breaks the build here instead of misdrawing silently.
constexpr bool plant_icon_dirs_match_contiguous_layout()
{
    for (int i = 0; i < PLANT_COUNT; i++)
        if (PLANT_ICON_DIR[i] != PEASHOOTER_DIRECTORY + i)
            return false;
    return true;
}
static_assert(plant_icon_dirs_match_contiguous_layout(),
              "PLANT_ICON_DIR drifted from the enum order in image_manifest.hpp");
