#include "game_param.hpp"
#include "elements/zombies/zombie_struct.hpp" // CS_VEL
#include "level/plants_registry.hpp"
#include "level/zombies_registry.hpp"
#include <nlohmann/json.hpp>
using namespace nlohmann;

ifstream param_f("./resources/dat/parameters.json");
json param_json = json::parse(param_f);

ifstream plants_json("./resources/dat/plants.json");
auto plant_json = ordered_json::parse(plants_json);

ifstream zombies_json("./resources/dat/zombies.json");
auto zombie_json = ordered_json::parse(zombies_json);

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

// Playground elements size
const int BLOCK_WIDTH = param_json["BLOCK_WIDTH"];
const int BLOCK_HEIGHT = param_json["BLOCK_HEIGHT"];
const int HORIZ_BLOCK_COUNT = param_json["HORIZ_BLOCK_COUNT"];
const int VERT_BLOCK_COUNT = param_json["VERT_BLOCK_COUNT"];

const int X_UPPER_LEFT = param_json["X_UPPER_LEFT"];
const int Y_UPPER_LEFT = param_json["Y_UPPER_LEFT"];

// Pool/fog 6-row and roof 5-row geometry (repvz GridToPixelY: 85 px rows;
// roof tiles sit at LAWN_YMIN-10 with a per-column slope step).
const int POOL_BLOCK_HEIGHT = param_json.value("POOL_BLOCK_HEIGHT", 85);
const int POOL_Y_UPPER_LEFT = param_json.value("POOL_Y_UPPER_LEFT", 80);
const int ROOF_BLOCK_HEIGHT = param_json.value("ROOF_BLOCK_HEIGHT", 85);
const int ROOF_Y_UPPER_LEFT = param_json.value("ROOF_Y_UPPER_LEFT", 70);

// Mutable: rebased per level by load_level before the grid is (re)built.
int GRID_ROWS = VERT_BLOCK_COUNT;
int GRID_Y_TOP = Y_UPPER_LEFT;
int GRID_BLOCK_H = BLOCK_HEIGHT;
bool GRID_ROOF = false;

const int ELEMENT_WIDTH = param_json["ELEMENT_WIDTH"];
const int ELEMENT_HEIGHT = param_json["ELEMENT_HEIGHT"];

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Speed properties: increase or decrease the following 9 define values to speed up or speed down entities in the game
const int FPS = param_json["FPS"];
// ticks per frame
const int TICKS_PER_FRAME = 1000 / FPS;

// Frame pacer (declared in game_param.hpp): steps the schedule by the exact
// rational period hz/tps per frame -- the Bresenham carry makes the average
// deadline land on n*hz/tps with zero long-run drift at 60 fps. A frame
// finished a whole period or more behind its deadline (hitch, blocking modal
// scene, window drag) re-anchors the schedule to `now`, dropping the backlog
// so the game never bursts catch-up frames; one call per presented frame
// means one tick per period at most.
long long FramePacer::advance(long long now, long long hz, int tps)
{
    if (next_deadline < 0)
    {
        next_deadline = now;
        carry = 0;
        return now;
    }
    long long period = hz / tps;
    carry += hz % tps;
    if (carry >= tps)
    {
        carry -= tps;
        period++;
    }
    long long deadline = next_deadline + period;
    if (now - deadline >= period)
    {
        deadline = now;
        carry = 0;
    }
    next_deadline = deadline;
    return deadline;
}
// Time appear of first wave of zombies (ZOMBIE_COUNTDOWN_FIRST_WAVE = 1800cs
// -> 18s -> 1080 ticks at 60 tps, PvZ-Portable Board.cpp:59)
const int FIRST_WAVE_CLK_COUNT = param_json["FIRST_WAVE_CLK_COUNT"];
// Time between zombies' different groups: base wait ZOMBIE_COUNTDOWN = 2500cs
// -> 25s -> 1500 ticks (PvZ-Portable Board.cpp:60) ...
const int ZOMBIE_CREATE_CLK_COUNT = param_json["ZOMBIE_CREATE_CLK_COUNT"];
// ... plus Rand(ZOMBIE_COUNTDOWN_RANGE = 600cs -> 360 ticks), so a wave waits
// 25-31 s (Board.cpp:5374)
const int ZOMBIE_CREATE_CLK_RANGE = param_json["ZOMBIE_CREATE_CLK_RANGE"];
// Wait INTO a flag wave: ZOMBIE_COUNTDOWN_BEFORE_FLAG = 4500cs -> 45 s ->
// 2700 ticks (PvZ-Portable Board.cpp:62, armed at Board.cpp:5363 when the
// wave coming next is a flag wave -- the long buildup before a huge wave)
const int ZOMBIE_BEFORE_FLAG_CLK_COUNT = param_json["ZOMBIE_BEFORE_FLAG_CLK_COUNT"];
// Zombie bite plant frequency
const int BITE_CLK_COUNT = param_json["BITE_CLK_COUNT"];
// Zombie moving frequency
const int ZOMBIE_CLK_COUNT = param_json["ZOMBIE_CLK_COUNT"];

// Announcer appear time
const int ANNOUNCER_CLK_COUNT = param_json["ANNOUNCER_CLK_COUNT"];
// Sun change color frequency
const int SUN_CHANGE_COLOR_CLK_COUNT = param_json["SUN_CHANGE_COLOR_CLK_COUNT"];
// Max time blink when attack
const int MAX_TIME_BLINK = param_json["MAX_TIME_BLINK"];
// Max time being cold (10s; snow-pea chill, repvz ApplyChill(false) = 1000cs)
const int MAX_COLD_TIME = param_json["MAX_COLD_TIME"];
// Chill left by the ice-shroom blast once the freeze lets go (repvz
// ApplyChill(true) = 2000cs = 20s -> 1200 ticks at 60 tps)
const int ICE_SHROOM_COLD_TIME = param_json["ICE_SHROOM_COLD_TIME"];
// Max time that a sun can appear (450 ticks = CS_TICKS(750), repvz
// Coin::GetDisappearTime Coin.cpp:1495-1515)
const int SUN_APPEAR_LIMIT = param_json["SUN_APPEAR_LIMIT"];

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Initial sun count and normal sun value
const int NORMAL_SUN_VALUE = param_json["NORMAL_SUN_VALUE"];
const int INF = param_json["INF"];

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

// Moving speed of some elements
const int ZOMBIE_INIT_X = WINDOW_WIDTH;
const int ZOMBIE_DX = param_json["ZOMBIE_DX"];
// PEA_DX is px/tick for every straight shot (pea/snow pea/puff/spike/star
// axes). parameters.json holds the original 3.33 px/cs; CS_VEL converts it to
// the clone's 60 tps.
const float PEA_DX = CS_VEL(param_json["PEA_DX"]);
const int SUN_DY = param_json["SUN_DY"];
const int CLICKED_SUN_D = param_json["CLICKED_SUN_D"];

// *********************** PLANT_PARAM ***********************

/*The dat tables are indexed by name, not by file order: each plants.json /
zombies.json key resolves to its PlantType/ZombieType slot through the
registries (level/plants_registry.hpp, level/zombies_registry.hpp), so
re-sorting the JSONs can no longer silently reshuffle the arrays. An unknown,
duplicated or missing key is a hard startup error naming the file and key.*/

static void dat_fail(const string &file, const string &msg)
{
    cerr << file << ": " << msg << '\n';
    abort();
}

// plants.json objects indexed by PlantType; gaps are rejected up front.
static vector<const ordered_json *> index_plants_dat(const ordered_json &plants)
{
    vector<const ordered_json *> by_type(PLANT_COUNT, nullptr);
    for (const auto &it : plants.items())
    {
        const PlantDef *def = plant_def_by_key(it.key());
        if (!def)
            dat_fail("plants.json", "unknown plant key '" + it.key() + "'");
        if (by_type[def->type])
            dat_fail("plants.json", "duplicate plant key '" + it.key() + "'");
        by_type[def->type] = &it.value();
    }
    for (const auto &def : PLANT_DEFS)
        if (!by_type[def.type])
            dat_fail("plants.json", string("missing plant key '") + def.key + "'");
    return by_type;
}
static const vector<const ordered_json *> PLANTS_DAT = index_plants_dat(plant_json);

const vector<int> PLANT_SUN_COST = [](const vector<const ordered_json *> &by)
{
    vector<int> v(PLANT_COUNT);
    for (int i = 0; i < PLANT_COUNT; i++)
        v[i] = (*by[i])["cost"];
    return v;
}(PLANTS_DAT);

const vector<int> PLANT_LOADING_TIME = [](const vector<const ordered_json *> &by)
{
    vector<int> v(PLANT_COUNT);
    for (int i = 0; i < PLANT_COUNT; i++)
        v[i] = (*by[i])["loading_time"];
    return v;
}(PLANTS_DAT);

const vector<int> PLANT_HEALTH_LIMIT = [](const vector<const ordered_json *> &by)
{
    vector<int> v(PLANT_COUNT);
    for (int i = 0; i < PLANT_COUNT; i++)
        v[i] = (*by[i])["health"];
    return v;
}(PLANTS_DAT);

// Arming delay in original centiseconds (only the potato mine has one).
const vector<int> PLANT_ARMED_TIME = [](const vector<const ordered_json *> &by)
{
    vector<int> v(PLANT_COUNT);
    for (int i = 0; i < PLANT_COUNT; i++)
        v[i] = by[i]->value("armed_time", 0);
    return v;
}(PLANTS_DAT);

// Aquatic plants (lilypad / tanglekelp / seashroom ...) are the only ones
// plantable directly onto water tiles.
const vector<bool> PLANT_IS_AQUATIC = [](const vector<const ordered_json *> &by)
{
    vector<bool> v(PLANT_COUNT);
    for (int i = 0; i < PLANT_COUNT; i++)
        v[i] = by[i]->value("aquatic", false);
    return v;
}(PLANTS_DAT);

// *********************** ZOMBIE_HEALTH_LIMIT ***********************

// Health/tier values are float: the effective scale is 20 original damage
// points = 1 pea per pool point (repvz ZombieInitialize: 270 body etc.), and
// exact ports land on .5 steps (270 -> 13.5, 1370 -> 68.5, ...).
const vector<set<float>> ZOMBIE_HEALTH_LIMIT = [](const ordered_json &zombies)
{
    vector<bool> seen(COUNT_ZOMBIE_TYPE, false);
    vector<set<float>> v(COUNT_ZOMBIE_TYPE);
    for (const auto &it : zombies.items())
    {
        const ZombieDef *def = zombie_def_by_key(it.key());
        if (!def)
            dat_fail("zombies.json", "unknown zombie key '" + it.key() + "'");
        if (seen[def->type])
            dat_fail("zombies.json", "duplicate zombie key '" + it.key() + "'");
        seen[def->type] = true;
        set<float> health_limit(it.value().begin(), it.value().end());
        v[def->type] = health_limit;
    }
    for (const auto &def : ZOMBIE_DEFS)
        if (!seen[def.type])
            dat_fail("zombies.json", string("missing zombie key '") + def.key + "'");
    return v;
}(zombie_json);