#include "level/level_data.hpp"
#include "draw/ImageType.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/Player/Player.hpp"
#include "elements/bowling/rolling_nut.hpp"
#include "elements/conveyor/conveyor.hpp"
#include "elements/elements.hpp"
#include "elements/plants/plant_type.hpp"
#include "elements/zombies/zombie_type.hpp"
#include "level/plants_registry.hpp"
#include "level/wave_gen.hpp"
#include "level/zombies_registry.hpp"
#include "support.hpp"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <stdexcept>

extern Level level;
extern Elements game_characters;

extern Player player;
extern Map cells;

// name -> type lookup tables, derived from the single registries
// (zombies_registry.hpp / plants_registry.hpp) so this file cannot drift
// from the wave generator, the dat loader, the selector or the almanac.
static const map<string, int> zombie_type_map = []
{
    map<string, int> m;
    for (const auto &def : ZOMBIE_DEFS)
        m.emplace(def.key, def.type);
    return m;
}();

static const map<string, int> plant_type_map = []
{
    map<string, int> m;
    for (const auto &def : PLANT_DEFS)
        m.emplace(def.key, def.type);
    return m;
}();

// background name -> index after BACKGROUND_LV1_DIRECTORY
static const map<string, int> background_map = {
    {"frontyard_1lane", 0},
    {"frontyard_3lane", 1},
    {"frontyard_full", 2},
    {"frontyard_night", 3},
    {"pool", 4},
    {"pool_night_fog", 5},
    {"roof", 6},
    {"roof_night", 7}};

int background_index_of(const string &bg_name)
{
    auto it = background_map.find(bg_name);
    return it == background_map.end() ? -1 : it->second;
}

/*Night grave columns (0-based) per grave count, from Board::PickBackground
(Board.cpp:1136+). Count 9 is the whack-a-zombie spread (any col >= 3).
Row is rolled per grave among the level's playable lanes.*/
static vector<int> grave_columns(int count)
{
    switch (count)
    {
    case 4: // levels 11-13
        return {6, 7, 8, 8};
    case 7: // levels 14, 16
        return {5, 6, 7, 7, 8, 8, 8};
    case 11: // levels 17-19
        return {4, 5, 5, 6, 6, 7, 7, 7, 8, 8, 8};
    case 13: // level 20
        return {3, 4, 4, 5, 5, 6, 6, 7, 7, 7, 8, 8, 8};
    default:
        return {};
    }
}

/*Place `count` grave stones on the map. whack: uniform free spread over
cols >= 3; otherwise the fixed per-count column pattern with random lanes.
Deterministic per PVZ_WAVE_SEED (shares the level rng salt).
Each fresh stone queues a staggered grow-in (repvz AddAGraveStone starts the
item counter at -Rand(50), then DrawGraveStone clip-reveals it over ~100cs):
clk = GRAVE_GROW_TICKS + stagger, the render turns the head start into delay.*/
static void place_graves(int count, bool whack, mt19937 &rng)
{
    vector<int> cols = whack ? vector<int>() : grave_columns(count);
    int placed = 0, guard = 500;
    while (placed < count && guard-- > 0)
    {
        int col = whack ? 3 + (int)(rng() % 6) : cols[placed];
        int row = level.playable_rows[rng() % level.playable_rows.size()];
        if (cells[row][col].has_grave)
            continue;
        cells[row][col].has_grave = true;
        level.grave_grows.push_back({row, col,
                                     GRAVE_GROW_TICKS + (int)(rng() % 50)});
        placed++;
    }
}

/*Pre-planted plants from the JSON's "pre_planted" list (roof flower pots).
Mirrors the original's CutScene::AddFlowerPots (CutScene.cpp:1050): 5-1 gets
5 columns of pots, 5-2 gets 4, 5-3..5-10 get 3. Spawned straight through
init_plant like a hand-planted pot -- no sun cost, no packet recharge -- but
the tile is flagged planted/blocked so zombies chew the pot and a main plant
can ride on top of it (the support-plant rule in support_plant.hpp).
Field absent = no-op; bad entries are logged and skipped.*/
static void place_pre_planted(const json &j)
{
    if (!j.count("pre_planted"))
        return;
    for (const auto &jp : j["pre_planted"])
    {
        auto it = plant_type_map.find(jp.value("plant", string()));
        int row = jp.value("row", -1);
        int col = jp.value("col", -1);
        if (it == plant_type_map.end() || row < 0 || row >= GRID_ROWS ||
            col < 0 || col >= HORIZ_BLOCK_COUNT)
        {
            SDL_Log("level %d: invalid pre_planted entry ignored", level.level_num);
            continue;
        }
        // The original gates each pot on CanPlantAt: never overwrite a tile
        // that is already occupied or unplantable.
        if (cells[row][col].is_planted || cells[row][col].is_water ||
            cells[row][col].has_grave || cells[row][col].has_crater)
            continue;
        game_characters.plants.push_back(init_plant(it->second, row, col));
        cells[row][col].is_planted = true;
        cells[row][col].is_block_zombie = true;
    }
}

/*
Run-constant salt for the wave generator: waves differ between runs but are
reproducible within a run (and deterministically when PVZ_WAVE_SEED is set).
*/
static uint32_t wave_run_salt()
{
    static const uint32_t salt = [] {
        if (const char *env = getenv("PVZ_WAVE_SEED"))
            return (uint32_t)atoi(env);
        return (uint32_t)chrono::high_resolution_clock::now().time_since_epoch().count();
    }();
    return salt;
}

string level_file_path(int lv_id)
{
    return string(LEVELS_DIRECTORY) + "level" + to_string(lv_id) + ".json";
}

json get_lv_json(int lv_id)
{
    ifstream myfile(level_file_path(lv_id));
    return json::parse(myfile);
}

/*Zombie roster declared in the level JSON ("zombies"."roster"), as zombie-type
ids. validate_level guarantees the block and its names, so this only runs on
known-good files. The preview wants the declared roster, not the spawned
counts: a type the wave generator skipped this run still belongs in the
credits strip (repvz SeedChooserScreen builds its preview from
mSeedChooserScreen's PickToolZombies list, i.e. the level's full zombie
table).*/
vector<int> get_level_roster_types()
{
    vector<int> types;
    auto j = get_lv_json(level.level_num);
    if (!j.count("zombies") || !j["zombies"].count("roster"))
        return types;
    for (const auto &r : j["zombies"]["roster"])
    {
        auto it = zombie_type_map.find(r.value("type", string()));
        if (it != zombie_type_map.end() && it->second != FLAG_TYPE &&
            find(types.begin(), types.end(), it->second) == types.end())
            types.push_back(it->second);
    }
    return types;
}

/*Strongest zombie of a level's JSON roster, for the selector's no-debut cell
fallback: the highest declared "cost", ties keeping the FIRST declaration --
except ZOMBOSS, who outranks any cost tie (the boss is his level's star: at
5-10 the equally-costed gargantuar is declared first but zomboss must still
win). -1 when the level declares no roster (no-wave specials like 4-5), so
the cell stays a bare backdrop. Unknown names are skipped: the selector reads
JSONs straight, without validate_level.*/
int level_roster_strongest_zombie(int level_num)
{
    auto j = get_lv_json(level_num);
    if (!j.count("zombies") || !j["zombies"].count("roster"))
        return -1;
    int best_type = -1, best_cost = -1;
    for (const auto &r : j["zombies"]["roster"])
    {
        auto it = zombie_type_map.find(r.value("type", string()));
        if (it == zombie_type_map.end())
            continue;
        const int cost = r.value("cost", 0);
        if (cost > best_cost || (cost == best_cost && it->second == ZOMBOSS_TYPE))
        {
            best_type = it->second;
            best_cost = cost;
        }
    }
    return best_type;
}

/*Reset all things of level.*/
void reset_level()
{
    level.reset();
    game_characters.reset();

    for (size_t y = 0; y < cells.size(); y++)
    {
        for (int x = 0; x < HORIZ_BLOCK_COUNT; x++)
        {
            cells[y][x].is_planted = false;
            cells[y][x].is_block_zombie = false;
            cells[y][x].has_grave = false;
            cells[y][x].has_crater = false;
            cells[y][x].crater_clk = 0;
            cells[y][x].is_water = false;
            cells[y][x].has_ladder = false;
        }
    }
}

/*Reset all things of level.*/
bool is_belt_mode(const string &mode)
{
    return mode == "conveyor" || mode == "bowling";
}

/*Seed chooser picks per level number (this run only). A restart reloads the
level and keeps the same seeds via this table instead of re-asking.*/
static map<int, vector<int>> saved_seed_choices;

/*Plant seed packets of the level (resolved from the seeds mode).
Belt modes use the conveyor instead of the packet column.*/
static void init_seed_packets()
{
    player.seed_packets.clear();
    if (is_belt_mode(level.seed_mode))
        return;
    for (int type : level.seed_plants)
        player.seed_packets.push_back(SeedPacket(type));
}

/*Resolve the seeds entry into concrete plant types.
Modes:
    + fixed: exactly the listed seeds.
    + all_unlocked: every plant whose unlock level is reached.
    + conveyor: packets drop from the conveyor belt (no sun, no recharge).
    + bowling: belt dispenses Wall-nut/Explode-o-nut that are thrown, not planted.
Belt modes read the weighted seeds from jseeds[mode]; seed_plants still lists
the available types. Levels missing the weight block fall back to plain seeds.
*/
static void resolve_level_seeds(const json &jseeds)
{
    string mode = jseeds["mode"];
    level.seed_mode = mode;
    if ((mode == "conveyor" || mode == "bowling") && jseeds.count(mode))
    {
        for (const auto &js : jseeds[mode]["seeds"])
        {
            int type = plant_type_map.at(js["plant"].get<string>());
            level.belt_seeds.push_back({type, js["weight"]});
            level.seed_plants.push_back(type);
        }
    }
    else if (mode == "all_unlocked")
    {
        // The seed chooser picks the packets before the game starts; a
        // restart reuses the pick saved from the chooser. Left empty
        // otherwise so no 40-packet bar overflows the screen pre-chooser.
        auto it = saved_seed_choices.find(level.level_num);
        if (it != saved_seed_choices.end())
            level.seed_plants = it->second;
    }
    else if (mode == "conveyor")
    {
        for (int i = 0; i < PLANT_COUNT; i++)
            if (level_unlock_new_plant[i] <= level.level_num)
                level.seed_plants.push_back(i);
    }
    else // fixed, bowling without a weight block
    {
        for (const auto &jname : jseeds["fixed"])
            level.seed_plants.push_back(plant_type_map.at(jname.get<string>()));
    }
}
/*Playable lanes of a level: centered on the board row count, e.g. for a
    5-row board: rows = 1 -> [2]; rows = 3 -> [1..3]; rows = 5 -> [0..4].
*/
static void init_playable_rows(int row_cnt)
{
    int row0 = (GRID_ROWS - row_cnt) / 2;
    for (int row = row0; row < row0 + row_cnt; row++)
        level.playable_rows.push_back(row);
}

/*Rebase the runtime grid geometry to the level's stage and row count, and
rebuild the tile map. repvz GridToPixelY (Board.cpp:9242):
  + pool/fog: 6 rows of 85 px from y 80;
  + roof/roof_night: 5 rows of 85 px from y 70 (LAWN_YMIN - 10) -- the
    per-column slope step is a plant-placement offset (roof_tile_dy), the
    tile band itself stays flat so zombie lane code keeps one row base;
  + otherwise: classic 5 rows of 100 px from y 80.
Rows listed in the JSON's water_rows become water tiles (all 9 columns).*/
static void setup_grid(const json &j)
{
    int row_cnt = j.value("rows", VERT_BLOCK_COUNT);
    string bg = j.value("background", string());
    GRID_ROOF = (bg == "roof" || bg == "roof_night");
    if (GRID_ROOF)
    {
        GRID_ROWS = VERT_BLOCK_COUNT;
        GRID_Y_TOP = ROOF_Y_UPPER_LEFT;
        GRID_BLOCK_H = ROOF_BLOCK_HEIGHT;
    }
    else if (row_cnt >= 6)
    {
        GRID_ROWS = 6;
        GRID_Y_TOP = POOL_Y_UPPER_LEFT;
        GRID_BLOCK_H = POOL_BLOCK_HEIGHT;
    }
    else
    {
        GRID_ROWS = VERT_BLOCK_COUNT;
        GRID_Y_TOP = Y_UPPER_LEFT;
        GRID_BLOCK_H = BLOCK_HEIGHT;
    }
    if ((int)cells.size() != GRID_ROWS ||
        (GRID_ROWS > 0 && cells[0][0].y1 != GRID_Y_TOP))
        cells = create_a_collection_of_blocks();

    level.water_rows.assign(GRID_ROWS, false);
    if (j.count("water_rows"))
    {
        for (int row : j["water_rows"])
        {
            if (row < 0 || row >= GRID_ROWS)
                continue;
            level.water_rows[row] = true;
            for (int x = 0; x < HORIZ_BLOCK_COUNT; x++)
                cells[row][x].is_water = true;
        }
    }
    level.fog_column = j.value("fog_column", -1);
}

/*Stale-warning hook for level specials: every LevelSpecial the loader can
parse now has its gameplay wired up (whack, big_trouble, vasebreaker, storm,
bungee blitz in materialize_wave_plans, boss via boss_fight.cpp), so a level
load logs nothing. Unknown names are a hard validation error
(validate_level), this only ever sees valid enums. Kept as the single place
to hang a notice on when a future special lands before its gameplay.*/
static void warn_unimplemented_special(LevelSpecial)
{
}

/*
Turn generated wave plans into zombie instances.
    + First wave starts 18 s after "Ready Set Plant".
    + Later waves wait 25-31 s (shortened by the health-based early trigger).
    + Flag waves wait 45 s instead -- the original's long buildup into a
      huge wave (ZOMBIE_COUNTDOWN_BEFORE_FLAG, Board.cpp:5360-5363: the check
      runs after SpawnZombieWave()'s mCurrentWave++ at Board.cpp:4949, so it
      arms the wait for the flag wave coming NEXT, not a breather after one).
*/
static void materialize_wave_plans(const vector<WavePlan> &plans, mt19937 &rng)
{
    level.wave_count = plans.size();
    level.final_wave = level.wave_count - 1;
    level.waves.assign(level.wave_count, Wave());
    level.count_zombie_type.assign(COUNT_ZOMBIE_TYPE, 0);
    // Wave pacing by special (reference figures converted 100 cs -> 60 t):
    // + Challenge levels open with a 2 s first wave (repvz Challenge.cpp:
    //   218-239: whack / little-trouble / storm / bungee challenges set
    //   mZombieCountDown = 200 cs); bowling's dig phase hands over the same
    //   way.
    // + Whack runs a fixed 20 s wave clock (Challenge::WhackAZombieSpawning's
    //   2000 cs) -- flag or not.
    // + Big Trouble Little Zombie (3-5) waves come every 7.5 s (Board.cpp:
    //   5368: 750 cs).
    // + Bowling (1-5) is excluded from the flag-wave 45 s breather
    //   (Board.cpp:5360): its final wave fires on the normal timer.
    const bool whack = level.is_special(LevelSpecial::Whack);
    const bool fast_start = whack ||
                            level.is_special(LevelSpecial::BigTrouble) ||
                            level.is_special(LevelSpecial::Storm) ||
                            level.is_special(LevelSpecial::Bungee) ||
                            level.seed_mode == "bowling";
    const bool bowling = level.seed_mode == "bowling";
    for (int i = 0; i < level.wave_count; i++)
    {
        Wave &wave = level.waves[i];
        wave.has_flag = plans[i].has_flag;
        // First wave on level 1-2 before the first adventure completion waits
        // 50 s so the player can set up (repvz Board.cpp:1257-1260:
        // IsFirstTimeAdventureMode && mLevel == 2 -> ZOMBIE_COUNTDOWN * 2 =
        // 5000cs -> 3000 ticks). The clone has no mHasFinishedAdventure flag;
        // its proxy is unlocked_level: finishing 5-10 bumps it past
        // LEVEL_COUNT (player_data.cpp update_unlocked_level).
        wave.delay_time =
            (i == 0) ? (level.level_num == 2 && player.unlocked_level <= LEVEL_COUNT ? 3000 : (fast_start ? 120 : FIRST_WAVE_CLK_COUNT))
                     : whack ? 1200
            : (plans[i].has_flag && !bowling) ? ZOMBIE_BEFORE_FLAG_CLK_COUNT
            : level.is_special(LevelSpecial::BigTrouble) ? 450
            : ZOMBIE_CREATE_CLK_COUNT + (int)(rng() % ZOMBIE_CREATE_CLK_RANGE);

        // Bungee Blitz (5-5): flag waves are exactly five bungee drops instead
        // of the usual parade (Challenge::BungeeBlitz rule).
        bool blitz_flag = level.is_special(LevelSpecial::Bungee) && plans[i].has_flag;

        // Whack-a-Zombie (2-5) ships NO wave-plan zombies: every riser is
        // created fresh by the burst clock (elements_actions.cpp
        // whack_update, the WhackAZombieSpawning port) -- the tiny plans
        // (~32 zombies) would leave the yard empty next to the original's
        // ~100+ risers.
        // Flag zombie leads the parade, the rest trails behind.
        int x_bias_zombie = 0;
        if (!whack)
        {
        if (wave.has_flag && !blitz_flag)
        {
            wave.add_zombie(init_zombie(FLAG_TYPE, level.level_num));
            x_bias_zombie = 40; // flag-wave followers: +40 (Zombie.cpp:235)
        }
        if (blitz_flag)
        {
            for (int j = 0; j < 5; j++)
            {
                wave.add_zombie(init_zombie(BUNGEE_TYPE, level.level_num));
                level.count_zombie_type[BUNGEE_TYPE]++;
            }
        }
        else
        for (const auto &spawn : plans[i].spawns)
        {
            if (spawn.type == FLAG_TYPE)
                continue;
            for (int j = 0; j < spawn.count; j++)
            {
                Zombie *zombie = init_zombie(spawn.type, level.level_num, x_bias_zombie, i);
                level.count_zombie_type[spawn.type]++;
                wave.add_zombie(zombie);
                // Bobsled team: the four riders always arrive together, same
                // lane, packed at the lane end behind the lead (original:
                // only on ice; here the roster pairs them with the zomboni).
                // Only the LEADER wears the sled armor (300-pt helm,
                // Zombie.cpp:553-560 -- followers spawn with the parent and
                // get no helm); trim the mates back to the bare 270-pt body
                // tier (the tier under the leader's top).
                if (spawn.type == BOBSLED_TYPE)
                {
                    for (int b = 1; b < 4; b++)
                    {
                        Zombie *mate = init_zombie(BOBSLED_TYPE, level.level_num, x_bias_zombie + b * 45, i);
                        mate->row = zombie->row;
                        mate->y_location = cells[mate->row][0].y1 - 50;
                        mate->spawn_set_health(*next(ZOMBIE_HEALTH_LIMIT[BOBSLED_TYPE].rbegin()));
                        level.count_zombie_type[BOBSLED_TYPE]++;
                        wave.add_zombie(mate);
                    }
                }
            }
        }
        }

        // Count total zombies - for progress bar.
        level.zombie_count += wave.zombie_count();
    }
}

/*
Validate one level JSON up front: gather EVERY problem (missing required
key, zombie/plant name unknown to the registries, row/col/water_rows/
fog_column out of bounds, unknown background, ...) into one message that
names the file, log it once, then fail via an exception carrying the full
list. This replaces the old mix of silent `value()` defaults, silent
out-of-bounds skips and blind `.at()` crashes. Called at the top of
read_level; also reusable offline to sweep the files in resources/levels/.
*/
void validate_level(const json &j, const string &path)
{
    vector<string> errors;
    auto bad = [&](const string &msg) { errors.push_back(msg); };

    for (const char *key : {"name", "rows", "background", "sun", "seeds", "zombies"})
        if (!j.count(key))
            bad(string("missing required key '") + key + "'");

    int rows = j.count("rows") && j["rows"].is_number_integer()
                   ? j["rows"].get<int>()
                   : -1;
    if (j.count("rows") && (rows < 1 || rows > 6))
        bad("'rows' must be an integer in [1, 6], got " +
            (j["rows"].is_number_integer() ? to_string(rows) : j["rows"].dump()));

    string bg = j.count("background") && j["background"].is_string()
                    ? j["background"].get<string>()
                    : string();
    if (j.count("background") && !background_map.count(bg))
        bad("unknown background '" + (bg.empty() ? j["background"].dump() : bg) + "'");

    if (j.count("special"))
    {
        LevelSpecial sp;
        if (!j["special"].is_string() ||
            !parse_level_special(j["special"].get<string>(), sp))
            bad("unknown special " + j["special"].dump());
    }

    // Effective board row count, mirroring setup_grid: roof stages stay at
    // VERT_BLOCK_COUNT, boards asking for >= 6 rows get the 6-row geometry.
    int grid_rows = bg == "roof" || bg == "roof_night" ? VERT_BLOCK_COUNT
                    : rows >= 6 ? 6
                                : VERT_BLOCK_COUNT;

    if (j.count("water_rows"))
    {
        if (!j["water_rows"].is_array())
            bad("'water_rows' must be an array of row indexes");
        else
            for (const auto &jr : j["water_rows"])
                if (!jr.is_number_integer() || jr.get<int>() < 0 || jr.get<int>() >= grid_rows)
                    bad("water_rows entry " + jr.dump() + " out of bounds for a " +
                        to_string(grid_rows) + "-row board");
    }
    if (j.count("graves") && (!j["graves"].is_number_integer() || j["graves"].get<int>() < 0))
        bad("'graves' must be a non-negative integer");
    if (j.count("fog_column"))
    {
        int fc = j["fog_column"].is_number_integer() ? j["fog_column"].get<int>() : -2;
        if (fc < -1 || fc >= HORIZ_BLOCK_COUNT)
            bad("'fog_column' must be -1 (no fog) or a column in [0, " +
                to_string(HORIZ_BLOCK_COUNT) + ")");
    }

    if (j.count("sun"))
    {
        const json &jsun = j["sun"];
        if (!jsun.count("sky") || !jsun["sky"].is_boolean())
            bad("'sun.sky' must be a boolean");
        if (!jsun.count("initial") || !jsun["initial"].is_number_integer() ||
            jsun["initial"].get<int>() < 0)
            bad("'sun.initial' must be a non-negative integer");
    }

    auto check_plant_name = [&](const json &v, const string &ctx) {
        if (!v.is_string() || !plant_def_by_key(v.get<string>()))
            bad(ctx + ": unknown plant name " + v.dump());
    };
    auto check_zombie_name = [&](const json &v, const string &ctx) {
        if (!v.is_string() || !zombie_def_by_key(v.get<string>()))
            bad(ctx + ": unknown zombie name " + v.dump());
    };

    if (j.count("seeds"))
    {
        const json &js = j["seeds"];
        if (!js.count("mode") || !js["mode"].is_string())
            bad("'seeds.mode' missing or not a string");
        else
        {
            string mode = js["mode"];
            static const set<string> modes = {"fixed", "all_unlocked", "conveyor", "bowling"};
            if (!modes.count(mode))
                bad("unknown seeds.mode '" + mode + "'");
            if (mode == "fixed")
            {
                if (!js.count("fixed") || !js["fixed"].is_array())
                    bad("seeds.mode 'fixed' requires a 'seeds.fixed' array of plant names");
                else
                    for (const auto &v : js["fixed"])
                        check_plant_name(v, "seeds.fixed");
            }
            if ((mode == "conveyor" || mode == "bowling") && js.count(mode))
            {
                const json &jb = js[mode];
                if (!jb.count("seeds") || !jb["seeds"].is_array())
                    bad("'seeds." + mode + ".seeds' must be an array");
                else
                    for (const auto &e : jb["seeds"])
                    {
                        if (e.count("plant"))
                            check_plant_name(e["plant"], string("seeds.") + mode);
                        else
                            bad("seeds." + mode + " entry without 'plant'");
                        if (!e.count("weight") || !e["weight"].is_number_integer() ||
                            e["weight"].get<int>() < 1)
                            bad("seeds." + mode + " entry weight must be a positive integer");
                    }
            }
        }
    }

    if (j.count("zombies"))
    {
        const json &jz = j["zombies"];
        // 0 waves is legitimate for no-wave specials (vasebreaker 4-5).
        if (!jz.count("total_waves") || !jz["total_waves"].is_number_integer() ||
            jz["total_waves"].get<int>() < 0)
            bad("'zombies.total_waves' must be a non-negative integer");
        if (!jz.count("roster") || !jz["roster"].is_array() ||
            (jz["total_waves"].is_number_integer() && jz["total_waves"].get<int>() > 0 &&
             jz["roster"].empty()))
            bad("'zombies.roster' must be an array (non-empty when total_waves > 0)");
        else
            for (const auto &pick : jz["roster"])
            {
                if (pick.count("type"))
                    check_zombie_name(pick["type"], "zombies.roster");
                else
                    bad("zombies.roster entry without 'type'");
                if (pick.count("cost") &&
                    (!pick["cost"].is_number_integer() || pick["cost"].get<int>() < 1))
                    bad("zombies.roster: cost must be a positive integer, got " + pick["cost"].dump());
                if (pick.count("weight") &&
                    (!pick["weight"].is_number_integer() || pick["weight"].get<int>() < 0))
                    bad("zombies.roster: weight must be a non-negative integer, got " + pick["weight"].dump());
                if (pick.count("first_wave") &&
                    (!pick["first_wave"].is_number_integer() || pick["first_wave"].get<int>() < 1))
                    bad("zombies.roster: first_wave must be a 1-based wave index, got " + pick["first_wave"].dump());
            }
        if (jz.count("intro_force") && !jz["intro_force"].is_null())
        {
            if (jz["intro_force"].count("type"))
                check_zombie_name(jz["intro_force"]["type"], "zombies.intro_force");
            else
                bad("zombies.intro_force without 'type'");
        }
    }

    if (j.count("reward") && !j["reward"].is_null())
    {
        if (j["reward"].count("plant") && !j["reward"]["plant"].is_null())
            check_plant_name(j["reward"]["plant"], "reward");
        if (j["reward"].count("zombie") && !j["reward"]["zombie"].is_null())
            check_zombie_name(j["reward"]["zombie"], "reward");
    }
    if (j.count("display") && j["display"].count("zombie"))
        check_zombie_name(j["display"]["zombie"], "display");

    if (j.count("pre_planted"))
    {
        if (!j["pre_planted"].is_array())
            bad("'pre_planted' must be an array");
        else
            for (const auto &jp : j["pre_planted"])
            {
                if (jp.count("plant"))
                    check_plant_name(jp["plant"], "pre_planted");
                else
                    bad("pre_planted entry without 'plant'");
                int row = jp.value("row", -1);
                int col = jp.value("col", -1);
                if (row < 0 || row >= grid_rows || col < 0 || col >= HORIZ_BLOCK_COUNT)
                    bad("pre_planted row/col out of bounds: row=" + to_string(row) +
                        " col=" + to_string(col) + " on a " + to_string(grid_rows) + "x" +
                        to_string(HORIZ_BLOCK_COUNT) + " board");
            }
    }

    if (!errors.empty())
    {
        string msg = path + ": " + to_string(errors.size()) + " invalid level definition(s):";
        for (const string &e : errors)
            msg += "\n  - " + e;
        SDL_Log("%s", msg.c_str());
        throw runtime_error(msg);
    }
}
/*Read level's information from its JSON file: name, rows, background, sun,
seeds, zombies (wave-generator spec). Invalid files are rejected by
validate_level before anything is touched.*/
void read_level()
{
    auto j = get_lv_json(level.level_num);
    validate_level(j, level_file_path(level.level_num));

    level.level_name = j.value("name", string("Level"));
    setup_grid(j);
    init_playable_rows(j["rows"]);
    level.map_type = background_map.at(j["background"].get<string>());
    level.has_sun_from_sky = j["sun"]["sky"];
    // JSON "initial" is the standard 50-sun stake of repvz Board.cpp:1434-1436
    // (the else branch of the mSunMoney init chain). Level 1-1 upgrades to
    // 150 only while the first adventure campaign is still on level 1
    // (repvz Board.cpp:1430-1433: IsFirstTimeAdventureMode && mLevel == 1;
    // LawnApp.cpp:2600-2602: first-time == adventure not finished). Replay
    // proxies: mLevel == 1 ~ unlocked_level == 1 (next uncleared level);
    // adventure finished ~ unlocked_level > LEVEL_COUNT (update_unlocked_level
    // bumps past it after 5-10), so unlocked_level > 1 means 50.
    player.sun_count = j["sun"]["initial"];
    if (level.level_num == 1 && player.unlocked_level <= 1)
        player.sun_count = 150;
    // First sky sun 4.25-7 s in (repvz Board.cpp:1454: RandRangeInt(425,700)
    // centiseconds = 255-420 ticks at 60 tps); later intervals in
    // elements_actions.cpp follow Board.cpp:5430.
    level.sky_sun_clk = 255 + rand(0, 165);
    level.sky_sun_fallen = 0;
    // JSON "special" string -> enum, validated above so this parse cannot fail.
    level.special = LevelSpecial::None;
    parse_level_special(j.value("special", string()), level.special);
    warn_unimplemented_special(level.special);
    // Big Trouble Little Zombie (3-5): mini zombies animate faster. The
    // original's rate is exactly 2x (UpdateAnimSpeed: rate ~ 1/mScaleZombie
    // with mScaleZombie=0.5, Zombie.cpp:6734). The JSON can tune/override it;
    // big_trouble defaults to 2x, other levels 1x.
    level.zombie_time_mul = j.value("zombie_time_mul",
                                    level.is_special(LevelSpecial::BigTrouble) ? 2.0f : 1.0f);
    resolve_level_seeds(j["seeds"]);
    // Bowling is throw-only: no shovel, no planted seeds.
    level.has_shovel = (level.level_num >= 5 || player.unlocked_level > 4) &&
                       level.seed_mode != "bowling";

    WaveGenSpec spec = wave_spec_from_json(j["zombies"]);
    // Replay wave budget (Board.cpp:642-645): a finished adventure ramps
    // the per-wave points faster (w*2/5 + 1). Proxy for
    // mHasFinishedAdventure: unlocked_level past LEVEL_COUNT (finishing 5-10
    // bumps it, player_data.cpp update_unlocked_level). Level 1-5 (the
    // bowling tutorial) keeps the first-run budget, like the original.
    spec.replay_budget = player.unlocked_level > LEVEL_COUNT && level.level_num != 5;
    // Replay wave count (Board.cpp:583-586): the same finished adventure also
    // bumps the WAVE COUNT -- under 10 becomes 20, anything else +10 -- except
    // the mini-boss flag levels 1-10/2-10/3-10 (LawnApp.cpp:2141-2147) and
    // the whack special (fixed 8, Board.cpp:576-579); vasebreaker is waveless
    // (total_waves 0). This was the "too few zombies on every level" root
    // cause: the replay POINTS were ported but not the wave bump, so a
    // replayed 1-1 kept its 4 first-run waves (~6 zombies instead of ~106).
    if (spec.replay_budget &&
        level.level_num != 10 && level.level_num != 20 && level.level_num != 30 &&
        spec.total_waves > 0 && !level.is_special(LevelSpecial::Whack))
        spec.total_waves = spec.total_waves < 10 ? 20 : spec.total_waves + 10;
    // First-time 1-1 carries no flag parade (repvz Board.cpp:527, IsFlagWave's
    // level-1 first-run suppression); a replayed 1-1 (flag_waves true in the
    // JSON) runs the full 20-wave flag cadence like the original.
    if (level.level_num == 1 && player.unlocked_level <= LEVEL_COUNT)
        spec.flag_waves = false;
    // Grave-spawned zombies wear cones when the roster knows coneheads.
    for (const auto &pick : spec.roster)
        if (pick.type == CONE_TYPE)
            level.graves_have_cones = true;
    mt19937 rng(wave_run_salt() + (uint32_t)level.level_num * 7919u);
    // Night grave stones (Board::PickBackground whack/night branch).
    if (int grvs = j.value("graves", 0))
        place_graves(grvs, level.is_special(LevelSpecial::Whack), rng);
    place_pre_planted(j);
    vector<WavePlan> plans = generate_wave_plans(spec, rng);
    if (getenv("PVZ_DUMP_WAVES"))
        dump_wave_plans(cout, level.level_num, level.level_name, plans);
    materialize_wave_plans(plans, rng);

    // Reset cur wave
    level.cur_wave = 0;
    level.waves_finished = false;
    level.next_wave_clk = level.waves.empty() ? 0 : level.waves[0].delay_time;
    level.next_wave_clk_start = level.next_wave_clk;
}

/*
Reset level.
Read level.
Random zombie appear order.
Init mower.
*/
void load_level()
{
    reset_level();
    read_level();
    // cout << level.level_num << '\n';

    init_seed_packets();
    if (is_belt_mode(level.seed_mode))
    {
        // First-packet delay per board, repvz Challenge::StartLevel's
        // mConveyorBeltCounter (cs -> 60 tps ticks): bowling 400cs -> 240,
        // bungee blitz / big trouble / storm 200cs -> 120, the boss fight
        // (and column, absent here) 1000cs -> 600; plain conveyor levels
        // (1-10, 2-10, 3-10) keep this clone's deliberate 2 s warm-up.
        // The boss belt also runs its cadence at x0.875
        // (Challenge.cpp:1636-1640 aConveyorSpeedMultiplier).
        int first_clk = 120;
        float speed_mul = 1.0f;
        if (level.seed_mode == "bowling")
            first_clk = 240;
        if (level.is_special(LevelSpecial::Boss))
        {
            first_clk = 600;
            speed_mul = 0.875f;
        }
        belt.init(level.belt_seeds, level.seed_mode == "bowling",
                  first_clk, speed_mul);
    }
    else
        belt.reset();
    reset_rolling_nuts();
    init_mower(game_characters.mowers, level.playable_rows.front(), level.playable_rows.back());
    player.is_shoveling = false;
    level.zombie_has_coming = false;
    // Tutorial advice script (1-1/1-2/1-3); inactive on other levels.
    level.advice.init(level.level_num);
}

vector<int> unlocked_plant_types()
{
    vector<int> types;
    for (int i = 0; i < PLANT_COUNT; i++)
        if (level_unlock_new_plant[i] <= player.unlocked_level)
            types.push_back(i);
    return types;
}

void set_level_seeds(const vector<int> &seeds)
{
    saved_seed_choices[level.level_num] = seeds;
    level.seed_plants = seeds;
    init_seed_packets();
}
