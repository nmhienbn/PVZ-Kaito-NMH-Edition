#pragma once

// Suburban Almanac copy: names + one-or-two-line descriptions, indexed by
// PlantType / ZombieType (enum order in game_param.hpp). English, original
// PvZ flavor; kept here instead of LawnStrings so the clone stays
// self-contained.

const char *almanac_plant_name(int type);
const char *almanac_plant_desc(int type);
const char *almanac_zombie_name(int type);
const char *almanac_zombie_desc(int type);
