#pragma once

#include <vector>

/*First-appearance ("debut") lookups shared by every preview surface: the
adventure level-selector cells, the almanac preview backdrops and the seed
packet backdrops. Adventure levels are numbered 1..LEVEL_COUNT; level JSONs
in resources/levels/ are the single source of truth (rosters + background),
so a data edit there is reflected everywhere for free.*/

/*Level where the plant first becomes available (award table
level_unlock_new_plant). 0 when the type never unlocks in adventure.*/
int plant_debut_level(int plant_type);

/*First adventure level whose declared zombie roster ("zombies"."roster")
lists the type. 0 when the type never debuts in adventure.*/
int zombie_debut_level(int zombie_type);

/*Zombie types whose debut level IS this one, in roster-declaration order.*/
std::vector<int> level_debut_zombies(int level_num);

/*PvZ2-style packet-back art directory (SEED_PACKET_BG_*) of the world the
plant FIRST debuts in -- the chooser catalog dresses each seed for its home
world, wherever the current level is played. Day for plants without an
adventure debut.*/
int plant_packet_bg_dir(int plant_type);

/*Backdrop image directory of a level (BACKGROUND_LV1_DIRECTORY + map index
of its JSON "background"); -1 for an unknown level/background name.*/
int level_background_dir(int level_num);

/*True for zombies only ever placed in a pool lane (ducky tube family,
snorkel, dolphin rider): the almanac draws these over the water strip;
everything else stands on the lawn, including pool/fog zombies the game also
puts on land.*/
bool zombie_water_only(int zombie_type);
