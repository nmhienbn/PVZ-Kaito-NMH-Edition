#include "debut_data.hpp"

#include "draw/ImageType.hpp" // BACKGROUND_LV1_DIRECTORY
#include "elements/SeedPacket/SeedPacket.hpp" // level_unlock_new_plant
#include "game_param.hpp"                     // PLANT_COUNT, *_TYPE enums
#include "level_data.hpp"                     // get_lv_json, background_index_of, LEVEL_COUNT
#include "zombies_registry.hpp"               // ZOMBIE_DEFS

using namespace std;

namespace
{
/*Types the rosters never declare: flags ride every wave end, ducky tubes are
a water-lane visual variant the spawner derives from basic-class zombies.
Their adventure debuts are still well-defined, so pin them here.*/
int zombie_debut_override(int zombie_type)
{
    switch (zombie_type)
    {
    case FLAG_TYPE:
        return 2; // first flag wave, 1-2
    case DUCKYTUBE_TYPE:
        return 21; // first pool level, 3-1
    default:
        return -1; // no override: use the roster scan
    }
}
} // namespace

int plant_debut_level(int plant_type)
{
    if (plant_type < 0 || plant_type >= PLANT_COUNT)
        return 0;
    return level_unlock_new_plant[plant_type];
}

namespace
{
/*Lazy scan of the adventure rosters: debut[type] = first level listing the
zombie, 0 when never listed. Costs one pass over the 50 level JSONs, cached
forever after -- the JSONs are data, so a roster edit updates the debuts.*/
const vector<int> &zombie_debut_table()
{
    static const vector<int> table = [] {
        vector<int> debut(COUNT_ZOMBIE_TYPE, 0);
        for (int lv = 1; lv <= LEVEL_COUNT; lv++)
        {
            auto j = get_lv_json(lv);
            if (!j.count("zombies") || !j["zombies"].count("roster"))
                continue;
            for (const auto &r : j["zombies"]["roster"])
            {
                const ZombieDef *def = zombie_def_by_key(r.value("type", string()));
                if (def != NULL && debut[def->type] == 0)
                    debut[def->type] = lv;
            }
        }
        return debut;
    }();
    return table;
}
} // namespace

int zombie_debut_level(int zombie_type)
{
    int over = zombie_debut_override(zombie_type);
    if (over > 0)
        return over;
    const vector<int> &debut = zombie_debut_table();
    if (zombie_type < 0 || zombie_type >= (int)debut.size())
        return 0;
    return debut[zombie_type];
}

vector<int> level_debut_zombies(int level_num)
{
    const vector<int> &debut = zombie_debut_table();
    vector<int> out;
    for (const auto &def : ZOMBIE_DEFS)
        if (def.type < (int)debut.size() && debut[def.type] == level_num)
            out.push_back(def.type);
    return out;
}

int level_background_dir(int level_num)
{
    if (level_num < 1 || level_num > LEVEL_COUNT)
        return -1;
    auto j = get_lv_json(level_num);
    int idx = background_index_of(j.value("background", string()));
    return idx < 0 ? -1 : BACKGROUND_LV1_DIRECTORY + idx;
}

/*World packet back keyed on the plant's DEBUT world (not the level being
played): debut level -> background index -> the PvZ2-style UI_SEEDPACKETS
crop (day lawn -> MODERNDAY, night -> DARK, pool -> BEACH, fog night-pool
-> PIRATE, roof -> EIGHTIES, roof night -> FUTURE).*/
int plant_packet_bg_dir(int plant_type)
{
    switch (level_background_dir(plant_debut_level(plant_type)) - BACKGROUND_LV1_DIRECTORY)
    {
    case 3:
        return SEED_PACKET_BG_NIGHT_DIRECTORY;
    case 4:
        return SEED_PACKET_BG_POOL_DIRECTORY;
    case 5:
        return SEED_PACKET_BG_POOL_NIGHT_DIRECTORY;
    case 6:
        return SEED_PACKET_BG_ROOF_DIRECTORY;
    case 7:
        return SEED_PACKET_BG_ROOF_NIGHT_DIRECTORY;
    default: // 0-2 day frontyard, and plants without an adventure debut
        return SEED_PACKET_BG_DAY_DIRECTORY;
    }
}

bool zombie_water_only(int zombie_type)
{
    switch (zombie_type)
    {
    case DUCKYTUBE_TYPE:
    case SNORKEL_TYPE:
    case DOLPHINRIDER_TYPE:
        return true;
    default:
        return false;
    }
}
