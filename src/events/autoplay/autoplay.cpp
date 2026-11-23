#include "autoplay.hpp"

#include "draw/ImageType.hpp"
#include "draw/gl/gl_loader.hpp"
#include "draw/render_elements.hpp"
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/Player/Player.hpp"
#include "elements/bullets/bullet.hpp"
#include "elements/elements.hpp"
#include "elements/elements_actions.hpp"
#include "elements/mower/mower.hpp"
#include "elements/plants/plant_type.hpp"
#include "elements/zombies/zombie.hpp"
#include "elements/zombies/zombie_type.hpp"
#include "game_param.hpp"
#include "level/level_data.hpp"
#include "music/music.hpp"
#include "reanim/reanim_plant.hpp"
#include "reanim/reanim_zombie.hpp"
#include "support.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

extern int game_state;
extern int clk;
extern int logic_ticked;
extern Level level;
extern Elements game_characters;
extern Player player;
extern Map cells;
extern Window win;

namespace autoplay
{
namespace
{
const int TICKS_PER_SEC = 60;

/*Same name tables as the scene dumper (kept local: the scene dump helpers
are file-local there).*/
const char *const PLANT_NAMES[] = {
    "peashooter", "sunflower", "wallnut", "snowpea", "potatomine",
    "cherrybomb", "repeater", "chomper", "puffshroom", "sunshroom",
    "fumeshroom", "gravebuster", "hypnoshroom", "scaredyshroom", "iceshroom",
    "doomshroom", "lilypad", "squash", "threepeater", "tanglekelp",
    "jalapeno", "spikeweed", "torchwood", "tallnut", "seashroom",
    "plantern", "cactus", "blover", "splitpea", "starfruit",
    "pumpkin", "magnetshroom", "cabbagepult", "flowerpot", "kernelpult",
    "coffeebean", "garlic", "umbrellaleaf", "marigold", "melonpult"};

const char *const ZOMBIE_NAMES[] = {
    "normal", "flag", "conehead", "buckethead", "screendoor",
    "polevault", "newspaper", "football", "dancer", "backupdancer",
    "duckytube", "snorkel", "zomboni", "bobsled", "dolphinrider",
    "jackbox", "balloon", "digger", "pogo", "bungee",
    "ladder", "catapult", "gargantuar", "imp", "zomboss", "yeti"};

const char *plant_name(int type)
{
    if (type >= 0 && type < (int)(sizeof(PLANT_NAMES) / sizeof(PLANT_NAMES[0])))
        return PLANT_NAMES[type];
    return "?";
}

const char *zombie_name(int type)
{
    if (type >= 0 && type < (int)(sizeof(ZOMBIE_NAMES) / sizeof(ZOMBIE_NAMES[0])))
        return ZOMBIE_NAMES[type];
    return "?";
}

/*Lose line (src/events/in_game/lose/lose.cpp lose_edge_x).*/
int lose_edge_x(int type)
{
    switch (type)
    {
    case GARGANTUAR_TYPE:
    case POLEVAULT_TYPE:
        return 70;
    case CATAPULT_TYPE:
    case FOOTBALL_TYPE:
    case ZOMBONI_TYPE:
        return 45;
    case DANCER_TYPE:
    case BACKUP_TYPE:
    case SNORKEL_TYPE:
        return 90;
    default:
        return 120;
    }
}

/*Sprite-sheet footprint used for the on-screen test: the 166-px zombie frame
is bottom-left anchored at x_location, roughly x_location-62 .. +104 in the
1050-px window at WORLD_SCALE=1.*/
const int ZOMBIE_BOX_LEFT = -62;
const int ZOMBIE_BOX_RIGHT = 104;

string fmt(double v, int prec = 2)
{
    ostringstream out;
    out << fixed << setprecision(prec) << v;
    return out.str();
}

double tsec(int tick) { return tick / (double)TICKS_PER_SEC; }

struct PlantMeta
{
    int id = 0;
    int type = 0;
    int row = 0, col = 0;
    int spawn_tick = 0;
    int spawn_hp = 0;
    vector<int> fire_ticks;
    vector<int> bite_ticks;
};

struct ZombieMeta
{
    int id = 0;
    int type = 0;
    int row = 0;
    int spawn_x = 0;
    int spawn_tick = 0;
    float spawn_hp = 0.0f;
    double nominal_speed = 0.0; // px/s at spawn (step_speed * 60)
    long long move_ticks = 0;
    double move_dist = 0.0;
    int death_tick = -1;
    string death_cause = "alive";
    float death_hp = 0.0f;
    int fully_on_tick = -1;
    bool enter_logged = false;
    bool full_logged = false;
    bool lose_logged = false;
};

struct BulletMeta
{
    int id = 0;
    int type = 0;
    int row = 0;
    int spawn_tick = 0;
    int spawn_x = 0;
    int prev_x = 0;
    int prev_explode = 0;
    int shooter_plant_id = -1;
    bool hit_logged = false;
};

int next_plant_id = 1;
int next_zombie_id = 1;
int next_bullet_id = 1;

unordered_map<const Plants *, PlantMeta> plant_meta;
unordered_map<const Zombie *, ZombieMeta> zombie_meta;
unordered_map<const Bullet *, BulletMeta> bullet_meta;

/*unordered_map iterates by pointer hash, so summary rows are emitted by id
to keep the file byte-identical across runs.*/
template <typename Map>
vector<const typename Map::mapped_type *> sorted_metas(const Map &m)
{
    vector<const typename Map::mapped_type *> rows;
    rows.reserve(m.size());
    for (const auto &kv : m)
        rows.push_back(&kv.second);
    sort(rows.begin(), rows.end(),
         [](const typename Map::mapped_type *a,
            const typename Map::mapped_type *b) { return a->id < b->id; });
    return rows;
}

/*Zombie speed / pea flight-speed accumulators.*/
long long pea_flight_samples = 0;
double pea_flight_dx = 0.0;
int pea_shots = 0;
int pea_hits = 0;
int plant_bites = 0;
int mower_kills = 0;
int mower_activations = 0;
int zombie_deaths = 0;
int lose_line_reached = 0;

/*Per-row mower state seen at the end of the previous tick.*/
map<int, int> mower_status;
set<int> mower_activated_rows;

ofstream g_events;

/*Rows inside one tick are buffered and written in a fixed semantic order.
The baseline game sorts the shared bullet vector by pointer, so raw emission
order can depend on process layout; sorting here keeps a fixed seed
byte-identical without touching gameplay.*/
struct EventLine
{
    int tick;
    int rank;
    int kind;
    int id;
    int col;
    string line;
};

vector<EventLine> g_event_rows;

int event_rank(const string &e)
{
    if (e == "plant_placed")
        return 10;
    if (e == "zombie_spawn")
        return 20;
    if (e == "zombie_enters_screen")
        return 30;
    if (e == "zombie_fully_on_screen")
        return 40;
    if (e == "pea_fire")
        return 50;
    if (e == "pea_hit")
        return 60;
    if (e == "plant_bite")
        return 70;
    if (e == "plant_eaten")
        return 80;
    if (e == "mower_activate")
        return 90;
    if (e == "zombie_death")
        return 100;
    if (e == "reach_lose_line")
        return 110;
    return 55;
}

int kind_rank(const string &k)
{
    if (k == "plant")
        return 10;
    if (k == "zombie")
        return 20;
    if (k == "bullet")
        return 30;
    if (k == "mower")
        return 40;
    return 50;
}

void event_row(int tick, const string &event, const string &kind, int id,
               int type, int row, int col, int x, int y,
               const string &detail = "")
{
    if (!g_events)
        return;
    string safe = detail;
    for (char &c : safe)
        if (c == ',')
            c = ';';
    ostringstream os;
    os << fmt(tsec(tick), 3) << ',' << tick << ',' << event << ',' << kind << ','
       << id << ',' << type << ',' << row << ',' << col << ',' << x << ','
       << y << ',' << safe;
    g_event_rows.push_back(
        {tick, event_rank(event), kind_rank(kind), id, col, os.str()});
}

void flush_events()
{
    if (!g_events)
        return;
    stable_sort(g_event_rows.begin(), g_event_rows.end(),
                [](const EventLine &a, const EventLine &b)
                {
                    if (a.tick != b.tick)
                        return a.tick < b.tick;
                    if (a.rank != b.rank)
                        return a.rank < b.rank;
                    if (a.kind != b.kind)
                        return a.kind < b.kind;
                    if (a.id != b.id)
                        return a.id < b.id;
                    if (a.col != b.col)
                        return a.col < b.col;
                    return a.line < b.line;
                });
    for (const EventLine &e : g_event_rows)
        g_events << e.line << '\n';
    g_event_rows.clear();
}

/*Build one track row. Empty strings leave the cell blank.*/
string track_header =
    "t,tick,kind,id,type_name,type,row,col,x,y,hp,state,anim,on_screen,fully_on_screen\n";

string track_line(int tick, const string &kind, int id,
                  const string &type_name, int type, int row, int col,
                  const string &x, const string &y, const string &hp,
                  const string &state, const string &anim, int on_screen,
                  int fully_on_screen)
{
    ostringstream os;
    os << fmt(tsec(tick), 3) << ',' << tick << ',' << kind << ',' << id << ','
       << type_name << ',' << type << ',' << row << ',' << col << ',' << x
       << ',' << y << ',' << hp << ',' << state << ',' << anim << ','
       << on_screen << ',' << fully_on_screen;
    return os.str();
}

/*---- level / entity setup -------------------------------------------------*/

Plants *add_plant(int type, int row, int col)
{
    Plants *p = init_plant(type, row, col);
    if (p == nullptr)
        return nullptr;
    game_characters.plants.push_back(p);
    cells[row][col].is_planted = true;
    if (type != SPIKEWEED_TYPE)
        cells[row][col].is_block_zombie = true;

    PlantMeta m;
    m.id = next_plant_id++;
    m.type = type;
    m.row = row;
    m.col = col;
    m.spawn_hp = p->get_health();
    plant_meta[p] = m;
    return p;
}

Zombie *add_zombie(int type, int row)
{
    Zombie *z = init_zombie(type, level.level_num);
    if (z == nullptr)
        return nullptr;
    z->row = row;
    z->y_location = cells[row][0].y1 - 50;
    z->is_moving = true;
    game_characters.zombies.push_back(z);

    ZombieMeta m;
    m.id = next_zombie_id++;
    m.type = type;
    m.row = row;
    m.spawn_x = z->x_location;
    m.spawn_tick = 0;
    m.spawn_hp = z->get_health();
    m.nominal_speed = (double)z->speed_px_per_tick() * TICKS_PER_SEC;
    zombie_meta[z] = m;
    return z;
}

/*Level + scenario setup shared by every run.*/
void setup_level(const string &scenario)
{
    /*The wave planner has its own run salt (level_data.cpp wave_run_salt,
    PVZ_WAVE_SEED) and materializes every wave's planned zombie through
    init_zombie() -- consuming the shared gameplay RNG before the scenario's
    own spawns. Pin it to the gameplay seed so a fixed PVZ_RAND_SEED gives a
    bit-identical run; an explicit PVZ_WAVE_SEED still wins.*/
    if (getenv("PVZ_WAVE_SEED") == nullptr)
    {
        const char *s = getenv("PVZ_RAND_SEED");
        _putenv_s("PVZ_WAVE_SEED",
                  (s != nullptr && s[0] != '\0') ? s : "0");
    }

    /*Dummy audio: the harness calls the real sound helpers, and SDL_mixer on
    a headless box should not fail to open a device. _putenv_s (not
    SDL_setenv) updates the CRT environment getenv() reads.*/
    _putenv_s("SDL_AUDIODRIVER", "dummy");
    init_music();

    /*Day front yard (1-4): five lanes, full grid, normal background.*/
    level.level_num = 4;
    load_level();
    level.background_directory = BACKGROUND_LV1_DIRECTORY + level.map_type;
    level.has_sun_from_sky = false;
    level.sky_sun_clk = INF;
    level.sky_sun_fallen = 0;
    /*No waves: update_new_wave_zombies() returns immediately.*/
    level.waves.clear();
    level.waves_finished = true;
    level.final_ambush_clk = 0;
    level.grave_grows.clear();
    level.grave_rises.clear();
    /*No sun economy.*/
    player.sun_count = 9999;

    /*Drop the level JSON's pre-planted props, exactly like scene_dump.*/
    for (auto *p : game_characters.plants)
        delete p;
    game_characters.plants.clear();
    for (int row = 0; row < (int)cells.size(); row++)
        for (int col = 0; col < HORIZ_BLOCK_COUNT; col++)
        {
            cells[row][col].is_planted = false;
            cells[row][col].is_block_zombie = false;
            cells[row][col].has_grave = false;
            cells[row][col].has_crater = false;
            cells[row][col].crater_clk = 0;
            cells[row][col].has_ladder = false;
        }

    if (!reanimz::ready())
        reanimz::init(win.sprite_renderer());
    if (!reanimplant::ready())
        reanimplant::init(win.sprite_renderer());

    if (scenario == "pea1")
    {
        add_plant(PEASHOOTER_TYPE, 2, 0);
        add_zombie(NORMAL_TYPE, 2);
    }
    else if (scenario == "pea_cols")
    {
        for (int row = 0; row < (int)cells.size(); row++)
        {
            add_plant(PEASHOOTER_TYPE, row, 2 * row);
            add_zombie(NORMAL_TYPE, row);
        }
    }
    else if (scenario == "walk")
    {
        add_zombie(NORMAL_TYPE, 0);
        add_zombie(CONE_TYPE, 1);
        add_zombie(BUCKET_TYPE, 2);
        add_zombie(FLAG_TYPE, 3);
        add_zombie(FOOTBALL_TYPE, 4);
    }
    else // eat
    {
        add_plant(WALLNUT_TYPE, 2, 4);
        add_zombie(NORMAL_TYPE, 2);
        add_plant(SUNFLOWER_TYPE, 3, 4);
        add_zombie(NORMAL_TYPE, 3);
    }

    /*Record the parked mowers (all inactive at setup).*/
    for (auto *m : game_characters.mowers)
        if (m != nullptr)
            mower_status[m->row] = (int)m->status;
}

/*---- rendering / screenshots ---------------------------------------------*/

bool save_png(const string &path)
{
    win.sprite_batch().flush();
    vector<unsigned char> buf((size_t)WINDOW_WIDTH * WINDOW_HEIGHT * 4);
    pvzgl::glReadPixels(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, pvzgl::PVZ_GL_RGBA,
                        pvzgl::PVZ_GL_UNSIGNED_BYTE, buf.data());
    SDL_Surface *shot = SDL_CreateRGBSurfaceWithFormat(
        0, WINDOW_WIDTH, WINDOW_HEIGHT, 32, SDL_PIXELFORMAT_ABGR8888);
    if (shot == nullptr)
    {
        SDL_Log("autoplay: SDL_CreateRGBSurfaceWithFormat failed: %s",
                SDL_GetError());
        return false;
    }
    for (int y = 0; y < WINDOW_HEIGHT; y++)
        memcpy((char *)shot->pixels + (size_t)y * shot->pitch,
               buf.data() + (size_t)(WINDOW_HEIGHT - 1 - y) * WINDOW_WIDTH * 4,
               (size_t)WINDOW_WIDTH * 4);
    const bool ok = IMG_SavePNG(shot, path.c_str()) == 0;
    if (!ok)
        SDL_Log("autoplay: IMG_SavePNG %s failed: %s", path.c_str(),
                SDL_GetError());
    SDL_FreeSurface(shot);
    return ok;
}

/*One normal gameplay frame (the scene_dump-style subset: background, lawn
entities, suns). logic_ticked = 0: this is a pure draw, no anim advance.*/
void capture_screenshot(const string &path)
{
    const int saved_logic = logic_ticked;
    const int saved_state = game_state;
    game_state = 0;
    logic_ticked = 0;
    win.clear_renderer();
    win.draw_bg(BLACK_SCREEN_DIRECTORY);
    win.draw_bg(level.background_directory);
    display_game_elements();
    display_suns(game_characters.suns);
    save_png(path);
    logic_ticked = saved_logic;
    game_state = saved_state;
}

/*---- helpers -------------------------------------------------------------*/

bool zombie_on_screen(const Zombie &z)
{
    int left = z.x_location + ZOMBIE_BOX_LEFT;
    int right = z.x_location + ZOMBIE_BOX_RIGHT;
    return right >= 0 && left <= WINDOW_WIDTH;
}

bool zombie_fully_on_screen(const Zombie &z)
{
    int left = z.x_location + ZOMBIE_BOX_LEFT;
    int right = z.x_location + ZOMBIE_BOX_RIGHT;
    return left >= 0 && right <= WINDOW_WIDTH;
}

int find_shooter(const Bullet &b)
{
    /*Peashooter muzzle is cells[row][col].x1 + 40 (Peashooter::fire_pea).*/
    for (auto *p : game_characters.plants)
    {
        if (p->get_row() != b.row || p->get_type() != PEASHOOTER_TYPE)
            continue;
        int muzzle = cells[b.row][p->get_col()].x1 + 40;
        if (abs(muzzle - b.x_location) <= 1)
            return plant_meta.count(p) ? plant_meta[p].id : -1;
    }
    return -1;
}

/*---- scenario run --------------------------------------------------------*/

struct RunResult
{
    int ticks = 0;
    bool zombie_died = false;
    int zombie_death_tick = -1;
    int shots = 0;
};

RunResult run_scenario(const string &scenario, const string &out_dir)
{
    const string dir = out_dir + "/" + scenario;
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    if (ec)
        SDL_Log("autoplay: cannot create %s: %s", dir.c_str(),
                ec.message().c_str());

    const string track_path = dir + "/track.csv";
    const string events_path = dir + "/events.csv";
    const string summary_path = dir + "/summary.txt";

    ofstream track(track_path);
    track << track_header;
    g_events.open(events_path);
    g_events << "t,tick,event,kind,id,type,row,col,x,y,detail\n";

    /*Setup happened before the log opened: emit the tick-0 placements and
    spawns here, plus the initial on-screen state. The sheet-geometry box
    starts at x-62..x+104, so every spawn already touches the 1050-px window
    (football still has a sliver to walk in).*/
    for (auto *p : game_characters.plants)
    {
        auto it = plant_meta.find(p);
        if (it == plant_meta.end())
            continue;
        const PlantMeta &m = it->second;
        event_row(0, "plant_placed", "plant", m.id, m.type, m.row, m.col,
                  cells[m.row][m.col].x1, cells[m.row][m.col].y1,
                  "hp=" + to_string(m.spawn_hp));
    }
    for (auto *z : game_characters.zombies)
    {
        auto it = zombie_meta.find(z);
        if (it == zombie_meta.end())
            continue;
        ZombieMeta &m = it->second;
        event_row(0, "zombie_spawn", "zombie", m.id, m.type, z->row, -1,
                  z->x_location, z->y_location,
                  "hp=" + fmt(m.spawn_hp) + ";speed_px_s=" +
                      fmt(m.nominal_speed));
        if (zombie_on_screen(*z))
        {
            m.enter_logged = true;
            event_row(0, "zombie_enters_screen", "zombie", m.id, m.type,
                      z->row, -1, z->x_location, z->y_location,
                      "bbox=" + to_string(z->x_location + ZOMBIE_BOX_LEFT) +
                          ".." + to_string(z->x_location + ZOMBIE_BOX_RIGHT));
        }
        if (zombie_fully_on_screen(*z))
        {
            m.full_logged = true;
            m.fully_on_tick = 0;
            event_row(0, "zombie_fully_on_screen", "zombie", m.id, m.type,
                      z->row, -1, z->x_location, z->y_location);
        }
    }

    /*Screenshots requested for these scenarios.*/
    set<int> shot_ticks;
    if (scenario == "pea1" || scenario == "walk")
        for (int s : {0, 5, 10, 15, 20, 30})
            shot_ticks.insert(s * TICKS_PER_SEC);

    /*Initial sample + t=0 screenshot (setup state, before the first tick).*/
    auto sample_track = [&](int tick)
    {
        /*Rows are keyed by kind then id (not emission order) so the file is
        byte-identical across runs: the baseline game sorts the shared bullet
        vector by pointer, so raw iteration order follows the process layout.*/
        vector<pair<string, string>> rows;
        auto key = [](int kind, int id)
        {
            char buf[32];
            snprintf(buf, sizeof buf, "%02d:%06d", kind, id);
            return string(buf);
        };
        /*Plants.*/
        for (auto *p : game_characters.plants)
        {
            auto it = plant_meta.find(p);
            if (it == plant_meta.end())
                continue;
            const PlantMeta &m = it->second;
            const Block &b = cells[m.row][m.col];
            rows.push_back(
                {key(10, m.id),
                 track_line(tick, "plant", m.id, plant_name(m.type), m.type,
                            m.row, m.col, to_string(b.x1), to_string(b.y1),
                            to_string(p->get_health()), "alive",
                            to_string(p->get_directory_num()), 1, 1)});
        }
        /*Zombies.*/
        for (auto *z : game_characters.zombies)
        {
            auto it = zombie_meta.find(z);
            if (it == zombie_meta.end())
                continue;
            const ZombieMeta &m = it->second;
            string state = z->is_moving ? "walking" : "eating";
            if (z->cold_time)
                state += "/chilled";
            rows.push_back(
                {key(20, m.id),
                 track_line(tick, "zombie", m.id, zombie_name(m.type), m.type,
                            z->row, -1, to_string(z->x_location),
                            to_string(z->y_location), fmt(z->get_health()),
                            state, to_string(z->cur_dir()),
                            zombie_on_screen(*z) ? 1 : 0,
                            zombie_fully_on_screen(*z) ? 1 : 0)});
        }
        /*Bullets.*/
        for (auto *b : game_characters.bullets)
        {
            auto it = bullet_meta.find(b);
            if (it == bullet_meta.end())
                continue;
            const BulletMeta &m = it->second;
            string state = b->explode <= BULLET_EXPLODE_TIME ? "exploding"
                                                             : "flying";
            rows.push_back(
                {key(30, m.id),
                 track_line(tick, "bullet", m.id, "pea", b->type, b->row, -1,
                            to_string(b->x_location),
                            to_string(b->y_location), "", state,
                            to_string(b->directory_num), 1, 1)});
        }
        /*Mowers.*/
        for (auto *m : game_characters.mowers)
        {
            if (m == nullptr)
                continue;
            rows.push_back(
                {key(40, m->row + 1),
                 track_line(tick, "mower", m->row + 1, "mower", 0, m->row, -1,
                            fmt(m->x, 1),
                            to_string(mower_ground_y((int)m->x, m->row)), "",
                            m->status == MOWER_ACTIVE ? "active" : "parked",
                            to_string(m->frame), 1, 1)});
        }
        sort(rows.begin(), rows.end(),
             [](const pair<string, string> &a, const pair<string, string> &b)
             { return a.first < b.first; });
        for (const auto &r : rows)
            track << r.second << '\n';
    };

    sample_track(0);
    if (shot_ticks.count(0))
        capture_screenshot(dir + "/shot_0s.png");

    /*Snapshot helpers (only pointer identity + the few gameplay fields the
    event detector needs).*/
    struct ZSnap
    {
        const Zombie *p;
        int x;
        int row;
        int type;
        float hp;
    };
    struct PSnap
    {
        const Plants *p;
        int hp;
    };

    /*--- main tick loop ---------------------------------------------------*/
    const int hard_limit = 60 * TICKS_PER_SEC;
    int tick = 0;
    int last_zombie_death = -1;
    /*D2 walk-sync diagnostics: running sum of the _ground delta fed by the
    walk animation and of the whole-pixel steps actually applied.*/
    double ground_dx_sum = 0.0;
    double applied_step_sum = 0.0;

    while (tick < hard_limit)
    {
        vector<ZSnap> zbefore;
        zbefore.reserve(game_characters.zombies.size());
        for (auto *z : game_characters.zombies)
            zbefore.push_back({z, z->x_location, z->row, z->type,
                               z->get_health()});
        vector<PSnap> pbefore;
        pbefore.reserve(game_characters.plants.size());
        for (auto *p : game_characters.plants)
            pbefore.push_back({p, p->get_health()});

        map<int, int> mower_this_tick;
        for (auto *m : game_characters.mowers)
            if (m != nullptr)
                mower_this_tick[m->row] = (int)m->status;

        /*One fixed-rate logic tick: the gameplay part of gameplay_tick()
        (music/groan/win-lose are presentation, not gameplay).*/
        handle_movements();
        handle_changes();
        ++clk;
        ++tick;

        /*Mower state after the tick: a parked mower can trip AND mow in the
        same tick, so the death-cause test must look at both snapshots.*/
        map<int, int> mower_after;
        for (auto *m : game_characters.mowers)
            if (m != nullptr)
                mower_after[m->row] = (int)m->status;

        /*Advance every plant exactly one tick through its real display path:
        the peashooter fires on a sprite frame, so this is gameplay.*/
        {
            const int saved_state = game_state;
            game_state = 0;
            logic_ticked = 1;
            for (auto *p : game_characters.plants)
                p->display(p->get_row());
            logic_ticked = 0;
            game_state = saved_state;
            win.sprite_batch().flush();
        }

        /*--- new bullets (fired by a peashooter during handle_changes) ----*/
        for (auto *b : game_characters.bullets)
        {
            if (bullet_meta.count(b))
                continue;
            BulletMeta m;
            m.id = next_bullet_id++;
            m.type = b->type;
            m.row = b->row;
            m.spawn_tick = tick;
            m.spawn_x = b->x_location;
            m.prev_x = b->x_location;
            m.prev_explode = b->explode;
            m.shooter_plant_id = find_shooter(*b);
            bullet_meta[b] = m;
            ++pea_shots;
            if (m.shooter_plant_id > 0)
                for (auto &kv : plant_meta)
                    if (kv.second.id == m.shooter_plant_id)
                    {
                        kv.second.fire_ticks.push_back(tick);
                        break;
                    }
            event_row(tick, "pea_fire", "bullet", m.id, b->type, b->row, -1,
                      b->x_location, b->y_location,
                      "shooter_plant=" + to_string(m.shooter_plant_id));
        }

        /*--- bullet hits / flight distance -------------------------------*/
        for (auto *b : game_characters.bullets)
        {
            auto it = bullet_meta.find(b);
            if (it == bullet_meta.end())
                continue;
            BulletMeta &m = it->second;
            const bool was_flying = m.prev_explode > BULLET_EXPLODE_TIME;
            const bool now_flying = b->explode > BULLET_EXPLODE_TIME;
            if (was_flying && !now_flying && !m.hit_logged)
            {
                m.hit_logged = true;
                ++pea_hits;
                event_row(tick, "pea_hit", "bullet", m.id, b->type, b->row, -1,
                          b->x_location, b->y_location,
                          "dmg=" + to_string(b->dmg));
            }
            if (was_flying && now_flying && b->x_location > m.prev_x)
            {
                pea_flight_dx += b->x_location - m.prev_x;
                ++pea_flight_samples;
            }
            m.prev_x = b->x_location;
            m.prev_explode = b->explode;
        }
        /*Forget bullets that left/expired (their pointer may be reused).*/
        {
            unordered_set<const Bullet *> live(game_characters.bullets.begin(),
                                               game_characters.bullets.end());
            for (auto it = bullet_meta.begin(); it != bullet_meta.end();)
                if (!live.count(it->first))
                    it = bullet_meta.erase(it);
                else
                    ++it;
        }

        /*--- zombie health / deaths / screen entry -----------------------*/
        unordered_set<const Zombie *> zlive(game_characters.zombies.begin(),
                                            game_characters.zombies.end());
        for (const ZSnap &s : zbefore)
        {
            if (zlive.count(s.p))
                continue;
            auto it = zombie_meta.find(s.p);
            if (it == zombie_meta.end())
                continue;
            ZombieMeta &m = it->second;
            if (m.death_tick >= 0)
                continue;
            m.death_tick = tick;
            m.death_hp = s.hp;
            if ((mower_this_tick.count(s.row) &&
                 mower_this_tick[s.row] == (int)MOWER_ACTIVE) ||
                (mower_after.count(s.row) &&
                 mower_after[s.row] == (int)MOWER_ACTIVE))
            {
                m.death_cause = "mower";
                ++mower_kills;
            }
            else
            {
                /*A pea hit on this row this tick killed it.*/
                bool pea_hit_row = false;
                for (auto *b : game_characters.bullets)
                {
                    auto bit = bullet_meta.find(b);
                    if (bit != bullet_meta.end() && bit->second.hit_logged &&
                        b->row == s.row &&
                        b->explode <= BULLET_EXPLODE_TIME)
                    {
                        pea_hit_row = true;
                        break;
                    }
                }
                m.death_cause = pea_hit_row ? "pea" : "other";
            }
            ++zombie_deaths;
            last_zombie_death = tick;
            event_row(tick, "zombie_death", "zombie", m.id, s.type, s.row, -1,
                      s.x, 0, "cause=" + m.death_cause + ";hp=" + fmt(s.hp));
        }
        for (auto *z : game_characters.zombies)
        {
            auto it = zombie_meta.find(z);
            if (it == zombie_meta.end())
            {
                ZombieMeta m;
                m.id = next_zombie_id++;
                m.type = z->type;
                m.row = z->row;
                m.spawn_x = z->x_location;
                m.spawn_tick = tick;
                m.spawn_hp = z->get_health();
                m.nominal_speed =
                    (double)z->speed_px_per_tick() * TICKS_PER_SEC;
                zombie_meta[z] = m;
                event_row(tick, "zombie_spawn", "zombie", m.id, z->type, z->row,
                          -1, z->x_location, z->y_location,
                          "hp=" + fmt(m.spawn_hp));
                it = zombie_meta.find(z);
            }
            ZombieMeta &m = it->second;
            if (!m.enter_logged && zombie_on_screen(*z))
            {
                m.enter_logged = true;
                event_row(tick, "zombie_enters_screen", "zombie", m.id,
                          z->type, z->row, -1, z->x_location, z->y_location);
            }
            if (!m.full_logged && zombie_fully_on_screen(*z))
            {
                m.full_logged = true;
                m.fully_on_tick = tick;
                event_row(tick, "zombie_fully_on_screen", "zombie", m.id,
                          z->type, z->row, -1, z->x_location, z->y_location);
            }
            if (!m.lose_logged && z->x_location <= lose_edge_x(z->type))
            {
                m.lose_logged = true;
                ++lose_line_reached;
                event_row(tick, "reach_lose_line", "zombie", m.id, z->type,
                          z->row, -1, z->x_location, z->y_location,
                          "lose_edge=" + to_string(lose_edge_x(z->type)));
            }
        }

        /*Walking distance while the zombie is in walking state. The walk
        accumulator moves 0/1 px per tick (float carry), so count every
        walking tick, not just the ticks that happened to step a pixel.*/
        for (const ZSnap &s : zbefore)
            if (zlive.count(s.p))
            {
                auto it = zombie_meta.find(s.p);
                if (it == zombie_meta.end())
                    continue;
                int dx = s.x - s.p->x_location;
                if (s.p->is_moving)
                {
                    ++it->second.move_ticks;
                    if (dx > 0)
                        it->second.move_dist += dx;
                    // By construction the ground delta equals the applied
                    // whole-pixel step over the run (fractional carry aside).
                    ground_dx_sum += s.p->last_ground_dx;
                    applied_step_sum += s.p->last_step_px;
                }
            }

        /*--- plant bites / eaten plants ----------------------------------*/
        unordered_set<const Plants *> plive(game_characters.plants.begin(),
                                            game_characters.plants.end());
        for (const PSnap &s : pbefore)
        {
            auto it = plant_meta.find(s.p);
            if (it == plant_meta.end())
                continue;
            PlantMeta &m = it->second;
            if (!plive.count(s.p))
            {
                m.bite_ticks.push_back(tick);
                ++plant_bites;
                event_row(tick, "plant_eaten", "plant", m.id, m.type, m.row, m.col,
                          cells[m.row][m.col].x1, cells[m.row][m.col].y1,
                          "hp_before=" + to_string(s.hp));
            }
            else if (s.p->get_health() < s.hp)
            {
                int lost = s.hp - s.p->get_health();
                for (int i = 0; i < lost; i++)
                    m.bite_ticks.push_back(tick);
                plant_bites += lost;
                event_row(tick, "plant_bite", "plant", m.id, m.type, m.row,
                          m.col, cells[m.row][m.col].x1,
                          cells[m.row][m.col].y1,
                          "hp=" + to_string(s.p->get_health()) +
                              ";lost=" + to_string(lost));
            }
        }

        /*--- mower activation --------------------------------------------*/
        for (auto *m : game_characters.mowers)
        {
            if (m == nullptr)
                continue;
            auto it = mower_status.find(m->row);
            bool was_active =
                it != mower_status.end() && it->second == (int)MOWER_ACTIVE;
            if (!was_active && m->status == MOWER_ACTIVE)
            {
                ++mower_activations;
                mower_activated_rows.insert(m->row);
                event_row(tick, "mower_activate", "mower", m->row + 1, 0,
                          m->row, -1, (int)m->x, mower_ground_y((int)m->x, m->row),
                          "status=active");
            }
            mower_status[m->row] = (int)m->status;
        }

        /*--- 0.5 s sampling ----------------------------------------------*/
        // Zombie animation now advances inside the logic tick (move_zombies),
        // so there is nothing to fast-forward before a render anymore.
        if (tick % 30 == 0)
            sample_track(tick);

        /*--- screenshots -------------------------------------------------*/
        if (shot_ticks.count(tick))
        {
            char name[32];
            snprintf(name, sizeof name, "/shot_%ds.png", tick / TICKS_PER_SEC);
            capture_screenshot(dir + name);
        }

        /*--- stopping rule -----------------------------------------------*/
        if (scenario == "pea1")
        {
            if (tick >= 30 * TICKS_PER_SEC && last_zombie_death >= 0)
                break;
            if (tick >= 45 * TICKS_PER_SEC) // safety: still alive at 45 s
                break;
        }
        else if (scenario == "pea_cols")
        {
            if (game_characters.zombies.empty())
                break;
        }
        else if (scenario == "eat")
        {
            if (tick >= 60 * TICKS_PER_SEC)
                break;
        }
        else // walk
        {
            if (tick >= 60 * TICKS_PER_SEC)
                break;
        }
    }

    RunResult result;
    result.ticks = tick;
    result.shots = pea_shots;
    if (!zombie_meta.empty())
    {
        /*Record the run's single/zombie deaths for the summary.*/
        for (auto &kv : zombie_meta)
            if (kv.second.death_tick >= 0)
            {
                result.zombie_died = true;
                result.zombie_death_tick = kv.second.death_tick;
                break;
            }
    }
    flush_events();
    g_events.flush();
    g_events.close();

    /*--- summary ---------------------------------------------------------*/
    ofstream sum(summary_path);
    const char *seed_env = getenv("PVZ_RAND_SEED");
    const string seed = (seed_env != nullptr && seed_env[0] != '\0')
                            ? seed_env
                            : "(time-seeded)";
    sum << "# PVZ autoplay summary\n";
    sum << "scenario: " << scenario << "\n";
    sum << "seed: " << seed << "\n";
    {
        const char *wave_seed = getenv("PVZ_WAVE_SEED");
        sum << "wave_seed: "
            << ((wave_seed != nullptr && wave_seed[0] != '\0') ? wave_seed
                                                               : "(unset)")
            << "\n";
    }
    sum << "level: " << level.level_num << " (day front yard, "
        << cells.size() << " rows, " << HORIZ_BLOCK_COUNT << " columns)\n";
    sum << "world_scale: " << WORLD_SCALE << "\n";
    sum << "ticks_per_second: " << TICKS_PER_SEC << "\n";
    sum << "ticks_run: " << result.ticks << " (" << fmt(tsec(result.ticks))
        << " s)\n";
    sum << "waves: disabled\n";
    sum << "sky_sun: disabled\n";
    sum << "hard_limit: " << hard_limit << " ticks (60 s)\n";
    sum << "\n";

    sum << "[counts]\n";
    sum << "plants_placed: " << plant_meta.size() << "\n";
    sum << "zombies_spawned: " << zombie_meta.size() << "\n";
    sum << "pea_shots: " << pea_shots << "\n";
    sum << "pea_hits: " << pea_hits << "\n";
    sum << "plant_bites: " << plant_bites << "\n";
    sum << "zombie_deaths: " << zombie_deaths << "\n";
    sum << "mower_activations: " << mower_activations << "\n";
    sum << "mower_kills: " << mower_kills << "\n";
    sum << "reach_lose_line: " << lose_line_reached << "\n";
    sum << "\n";

    auto mean_interval = [](const vector<int> &ticks) -> double
    {
        if (ticks.size() < 2)
            return 0.0;
        return (ticks.back() - ticks.front()) /
               (double)(ticks.size() - 1) / TICKS_PER_SEC;
    };

    sum << "[plants]\n";
    sum << "id,type,row,col,spawn_hp,end_hp,hp_lost,bites,first_bite_s,"
           "last_bite_s,mean_bite_interval_s,shots,first_shot_s,last_shot_s,"
           "mean_fire_interval_s\n";
    map<int, int> live_plant_hp;
    for (auto *p : game_characters.plants)
    {
        auto it = plant_meta.find(p);
        if (it != plant_meta.end())
            live_plant_hp[it->second.id] = p->get_health();
    }
    for (const PlantMeta *mp : sorted_metas(plant_meta))
    {
        const PlantMeta &m = *mp;
        int end_hp = live_plant_hp.count(m.id) ? live_plant_hp[m.id] : 0;
        double first_bite = m.bite_ticks.empty() ? -1 : tsec(m.bite_ticks.front());
        double last_bite = m.bite_ticks.empty() ? -1 : tsec(m.bite_ticks.back());
        double first_shot =
            m.fire_ticks.empty() ? -1 : tsec(m.fire_ticks.front());
        double last_shot =
            m.fire_ticks.empty() ? -1 : tsec(m.fire_ticks.back());
        sum << m.id << ',' << plant_name(m.type) << ',' << m.row << ',' << m.col
            << ',' << m.spawn_hp << ',' << end_hp << ','
            << (m.spawn_hp - end_hp) << ',' << m.bite_ticks.size() << ','
            << fmt(first_bite) << ',' << fmt(last_bite) << ','
            << fmt(mean_interval(m.bite_ticks)) << ',' << m.fire_ticks.size()
            << ',' << fmt(first_shot) << ',' << fmt(last_shot) << ','
            << fmt(mean_interval(m.fire_ticks)) << '\n';
    }
    sum << "\n";

    sum << "[zombies]\n";
    sum << "id,type,row,spawn_x,spawn_hp,spawn_speed_px_s,mean_walk_px_s,"
           "moved_px,moved_s,fully_on_screen_s,death_s,death_cause\n";
    for (const ZombieMeta *mp : sorted_metas(zombie_meta))
    {
        const ZombieMeta &m = *mp;
        double walk_speed =
            m.move_ticks > 0 ? m.move_dist / (m.move_ticks / (double)TICKS_PER_SEC)
                             : 0.0;
        sum << m.id << ',' << zombie_name(m.type) << ',' << m.row << ','
            << m.spawn_x << ',' << fmt(m.spawn_hp) << ',' << fmt(m.nominal_speed)
            << ',' << fmt(walk_speed) << ',' << fmt(m.move_dist, 0) << ','
            << fmt(m.move_ticks / (double)TICKS_PER_SEC)
            << ',' << fmt(m.fully_on_tick < 0 ? -1 : tsec(m.fully_on_tick))
            << ',' << fmt(m.death_tick < 0 ? -1 : tsec(m.death_tick)) << ','
            << m.death_cause << '\n';
    }
    sum << "\n";

    /*Aggregate walk speed per zombie type.*/
    sum << "[zombie_walk_speed_by_type]\n";
    sum << "type,count,mean_px_s,min_px_s,max_px_s\n";
    {
        map<int, vector<double>> by_type;
        for (auto &kv : zombie_meta)
            if (kv.second.move_ticks > 0)
                by_type[kv.second.type].push_back(
                    kv.second.move_dist /
                    (kv.second.move_ticks / (double)TICKS_PER_SEC));
        for (auto &kv : by_type)
        {
            /*Deterministic summation order.*/
            sort(kv.second.begin(), kv.second.end());
            double lo = kv.second.front(), hi = kv.second.front(), total = 0;
            for (double v : kv.second)
            {
                lo = min(lo, v);
                hi = max(hi, v);
                total += v;
            }
            sum << zombie_name(kv.first) << ',' << kv.second.size() << ','
                << fmt(total / kv.second.size()) << ',' << fmt(lo) << ','
                << fmt(hi) << '\n';
        }
    }
    sum << "\n";

    /*D2: the walk animation's _ground feed must equal the applied movement.*/
    sum << "[walk_sync]\n";
    sum << "ground_dx_total: " << fmt(ground_dx_sum) << "\n";
    sum << "applied_step_total: " << fmt(applied_step_sum) << "\n";
    sum << "delta_vs_applied_pct: "
        << (applied_step_sum > 0
                ? fmt(100.0 * std::fabs(ground_dx_sum - applied_step_sum) /
                          applied_step_sum)
                : string("n/a"))
        << "\n";
    sum << "\n";

    sum << "[peas]\n";
    sum << "expected_speed_px_s: " << fmt((double)PEA_DX * TICKS_PER_SEC) << "\n";
    if (pea_flight_samples > 0)
        sum << "measured_flight_speed_px_s: "
            << fmt(pea_flight_dx / pea_flight_samples * TICKS_PER_SEC) << "\n";
    else
        sum << "measured_flight_speed_px_s: n/a\n";
    sum << "flight_tick_samples: " << pea_flight_samples << "\n";
    sum << "hit_rate: "
        << (pea_shots > 0 ? fmt(100.0 * pea_hits / pea_shots, 1) + "%" : "n/a")
        << "\n";
    sum << "\n";

    sum << "[mowers]\n";
    sum << "row,activated,final_status\n";
    for (int row = 0; row < (int)cells.size(); row++)
    {
        Mower *m = nullptr;
        for (auto *candidate : game_characters.mowers)
            if (candidate != nullptr && candidate->row == row)
                m = candidate;
        string st;
        if (m != nullptr)
            st = m->status == (int)MOWER_ACTIVE ? "active" : "parked";
        else
            st = mower_activated_rows.count(row) ? "gone" : "parked";
        sum << row << ','
            << (mower_activated_rows.count(row) ? "yes" : "no") << ',' << st
            << '\n';
    }
    sum << "\n";

    sum << "[notes]\n";
    if (scenario == "pea1")
    {
        if (result.zombie_death_tick >= 0)
            sum << "- Pea measurements stop at the zombie's death (t="
                << fmt(tsec(result.zombie_death_tick))
                << " s); the empty lawn is still ticked to >=30 s so the "
                   "20 s/30 s screenshots exist.\n";
        else
            sum << "- Zombie survived the whole run.\n";
        sum << "- One peashooter row 2 col 0, one normal zombie row 2.\n";
    }
    else if (scenario == "pea_cols")
    {
        sum << "- Lane r has a peashooter at column 2*r, with one normal "
               "zombie per lane; later columns start much closer to the "
               "zombie spawn, so close lanes lose their shooter sooner.\n";
    }
    else if (scenario == "walk")
    {
        sum << "- No plants. normal/cone/bucket/flag/football in lanes 0..4 "
               "(football is the fast one).\n";
        sum << "- Mowers trip at x~199, well before the lose line "
               "(120/45), so reach_lose_line usually stays 0.\n";
    }
    else
    {
        sum << "- Wall-nut row 2 col 4 (80 clone hp = 4000 original), "
               "sunflower row 3 col 4 (6 clone hp = 300 original).\n";
        sum << "- Bite interval should read 0.5 s (BITE_CLK_COUNT=30).\n";
    }
    sum << "- Screenshots use the normal render path: black screen + level "
           "background + display_game_elements() + display_suns().\n";
    sum << "- on_screen uses the fixed sheet footprint x-62..x+104 against the "
           "1050-px window; zombies spawn with their left edge already inside, "
           "so enters_screen is t=0 while fully_on_screen waits for x+104<=1050.\n";
    sum.flush();
    sum.close();

    SDL_Log("autoplay: %s done (%d ticks, %d shots, %d hits, %d deaths)",
            scenario.c_str(), tick, pea_shots, pea_hits, zombie_deaths);
    return result;
}
} // namespace

bool run_if_requested()
{
    const char *scenario_env = getenv("PVZ_AUTOPLAY");
    if (scenario_env == nullptr || scenario_env[0] == '\0')
        return false;
    const string scenario = scenario_env;

    if (scenario != "pea1" && scenario != "pea_cols" && scenario != "walk" &&
        scenario != "eat")
    {
        SDL_Log("autoplay: unknown scenario '%s' (expected pea1, pea_cols, "
                "walk, eat)",
                scenario.c_str());
        return true; // requested: do not fall through into the normal game
    }

    const char *out_env = getenv("PVZ_AUTOPLAY_OUT");
    const string out_dir =
        (out_env != nullptr && out_env[0] != '\0') ? out_env : "./autoplay_out";

    setup_level(scenario);
    run_scenario(scenario, out_dir);
    return true;
}
} // namespace autoplay
