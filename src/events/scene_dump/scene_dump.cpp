#include "scene_dump.hpp"

#include "draw/gl/gl_loader.hpp"
#include "draw/render_elements.hpp"
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/Player/Player.hpp"
#include "elements/bullets/pea/pea.hpp"
#include "elements/elements.hpp"
#include "elements/mower/mower.hpp"
#include "elements/plants/plant_type.hpp"
#include "elements/sun/sun.hpp"
#include "elements/zombies/zombie.hpp"
#include "elements/zombies/zombie_type.hpp"
#include "events/almanac/almanac.hpp"
#include "game_param.hpp"
#include "level/level_data.hpp"
#include "reanim/reanim_plant.hpp"
#include "reanim/reanim_zombie.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

extern Level level;
extern Elements game_characters;
extern Map cells;
extern Window win;
extern Player player;
extern int game_state;

namespace scene_dump
{
bool g_capture_active = false;

namespace
{
/*One measured draw call's union box (screen px).*/
struct Box
{
    float x0 = 1e30f, y0 = 1e30f, x1 = -1e30f, y1 = -1e30f;
    bool any = false;

    void add(float x, float y)
    {
        x0 = min(x0, x);
        y0 = min(y0, y);
        x1 = max(x1, x);
        y1 = max(y1, y);
        any = true;
    }
    int ix() const { return (int)floorf(x0 + 0.5f); }
    int iy() const { return (int)floorf(y0 + 0.5f); }
    int iw() const { return max(0, (int)ceilf(x1) - ix()); }
    int ih() const { return max(0, (int)ceilf(y1) - iy()); }
};

Box g_box; // box of the draw call currently being measured

/*Entity report row (also the cyan bbox + yellow cross source).*/
struct Rec
{
    const char *kind; // plant | zombie | bullet | mower | sun
    int type;
    int row;
    int col; // -1 when the entity is not grid-planted
    int x, y; // logical anchor (cell origin / x_location / sun position)
    Box box;
};

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

/*Pick the level whose background/grid drives the scene. Default is a plain
day front yard with all five lanes (1-4: frontyard_full); the optional
PVZ_SCENE values map to the first level of each variant. The classic grid is
identical to level 1: x1 = 260 + col*80, y1 = 80 + row*100.*/
int level_for_scene(const string &scene)
{
    if (scene == "night")
        return 11; // 2-1: frontyard_night, 5 rows
    if (scene == "pool")
        return 21; // 3-1: pool, 6 rows of 85
    if (scene == "roof")
        return 41; // 5-1: roof, sloped
    return 4;      // 1-4: day front yard, 5 rows, full grid
}

/*Place a plant through the normal factory and wire the tile flags the same
way plant_new_plant does (minus sun cost / recharge / dirt FX).*/
Plants *add_plant(int type, int row, int col, vector<string> &skipped)
{
    Plants *p = init_plant(type, row, col);
    if (p == nullptr)
    {
        char buf[96];
        snprintf(buf, sizeof buf, "%s(r%d,c%d)", plant_name(type), row, col);
        skipped.push_back(buf);
        return nullptr;
    }
    game_characters.plants.push_back(p);
    cells[row][col].is_planted = true;
    if (type != SPIKEWEED_TYPE)
        cells[row][col].is_block_zombie = true;
    return p;
}

/*Potato Mine spawns unarmed (state PLANTED, 900-tick fuse). Run its own
per-tick state machine forward -- no gameplay logic, just next_frame -- until
it reports ARMED, so the dump shows the armed idle art. Returns false if the
type did not arm in the budget.*/
bool arm_potato_mine(Plants *p)
{
    PotatoMine *pm = dynamic_cast<PotatoMine *>(p);
    if (pm == nullptr)
        return false;
    // next_frame only needs assets.frame (the sheet wrap divisor); a dummy
    // with frame 1 is enough to count the fuse down and cross the recover
    // sheet's wrap. n_sheet still comes from the real directory metadata.
    PlantAssets dummy(1, 1, 1, 1, 1, 0, 0);
    const int saved_logic = logic_ticked;
    const int saved_state = game_state;
    game_state = 0; // next_frame gates on IS_PAUSED
    logic_ticked = 1;
    for (int i = 0; i < 5000 && !pm->is_armed(); i++)
        pm->next_frame(dummy);
    logic_ticked = saved_logic;
    game_state = saved_state;
    return pm->is_armed();
}

/*Spawn a zombie through the normal factory, then pin its lane/position the
way a spawn site does. Walking art is the constructor's default state.*/
Zombie *add_zombie(int type, int row, int x, vector<string> &skipped)
{
    Zombie *z = init_zombie(type, level.level_num);
    if (z == nullptr)
    {
        char buf[96];
        snprintf(buf, sizeof buf, "%s(r%d,x%d)", zombie_name(type), row, x);
        skipped.push_back(buf);
        return nullptr;
    }
    z->row = row;
    z->x_location = x;
    z->y_location = cells[row][0].y1 - 50;
    z->is_moving = true;
    game_characters.zombies.push_back(z);
    return z;
}

/*Run the per-tick animation state machines without any gameplay tick: the
render path (Plants::display / Zombie::display) advances its own anim once
per unit of logic_ticked, so a display pass with logic_ticked = ticks ages
every entity by exactly that many ticks without moving anything.*/
void advance_animation(int ticks)
{
    const int saved_logic = logic_ticked;
    const int saved_state = game_state;
    game_state = 0;
    logic_ticked = ticks;
    win.clear_renderer(); // this pass is throwaway; the real frame redraws below
    for (auto *p : game_characters.plants)
        p->display(p->get_row());
    for (auto *z : game_characters.zombies)
        z->display(z->row);
    logic_ticked = 0;
    game_state = saved_state;
    logic_ticked = saved_logic;
}

/*Draw the actual frame through the normal gameplay renderer (background +
shadows + mowers + plants + zombies + bullets, then suns).*/
void draw_scene_frame()
{
    win.clear_renderer();
    win.draw_bg(BLACK_SCREEN_DIRECTORY);
    win.draw_bg(level.background_directory);
    display_game_elements();
    display_suns(game_characters.suns);
}

/*Second, invisible pass over the same entities with logic_ticked == 0 (so
no anim advances): the empty scissor keeps every quad off the framebuffer
while the pushQuad hook still reports it, giving the exact drawn bbox of the
frame that is already on screen.*/
void capture_bounds(vector<Rec> &recs)
{
    const int saved_logic = logic_ticked;
    logic_ticked = 0;
    SDL_Rect empty = {0, 0, 0, 0};
    win.sprite_batch().setClipRect(&empty); // flushes the real frame first

    for (auto *m : game_characters.mowers)
    {
        if (m == nullptr)
            continue;
        capture_begin();
        m->display();
        Rec r{"mower", 0, m->row, -1, (int)lroundf(m->x), cells[m->row][0].y1, g_box};
        capture_end();
        recs.push_back(r);
    }
    for (int row = 0; row < (int)cells.size(); row++)
    {
        for (auto *p : game_characters.plants)
        {
            if (p->get_row() != row)
                continue;
            capture_begin();
            p->display(row);
            Rec r{"plant", p->get_type(), row, p->get_col(),
                  cells[row][p->get_col()].x1, cells[row][p->get_col()].y1, g_box};
            capture_end();
            recs.push_back(r);
        }
        for (auto *z : game_characters.zombies)
        {
            if (z->row != row)
                continue;
            capture_begin();
            z->display(row);
            Rec r{"zombie", z->type, row, -1, z->x_location,
                  cells[row][0].y1, g_box};
            capture_end();
            recs.push_back(r);
        }
    }
    for (auto *b : game_characters.bullets)
    {
        capture_begin();
        b->display();
        Rec r{"bullet", b->type, b->row, -1, b->x_location, b->y_location, g_box};
        capture_end();
        recs.push_back(r);
    }
    if (!game_characters.suns.empty())
    {
        capture_begin();
        display_suns(game_characters.suns);
        const Sun &s = game_characters.suns.front();
        Rec r{"sun", 0, s.final_row, s.final_col, s.x_location, s.y_location, g_box};
        capture_end();
        recs.push_back(r);
    }

    win.sprite_batch().setClipRect(nullptr); // flush the clipped quads away
    logic_ticked = saved_logic;
}

/*1 px translucent white rect (the grid lines; Window::draw_rect has no
alpha channel).*/
void grid_line(float x, float y, float w, float h)
{
    win.sprite_batch().fillRect(x, y, w, h, 255, 255, 255, 128);
}

void draw_cross(int x, int y)
{
    const int arm = 4;
    win.draw_line(x - arm, y, x + arm, y, YELLOW);
    win.draw_line(x, y - arm, x, y + arm, YELLOW);
}

void draw_overlay(const vector<Rec> &recs)
{
    // Lawn grid cell borders (white, 50% alpha).
    for (int row = 0; row < (int)cells.size(); row++)
        for (int col = 0; col < HORIZ_BLOCK_COUNT; col++)
        {
            const Block &b = cells[row][col];
            grid_line((float)b.x1, (float)b.y1, 1.0f, (float)(b.y2 - b.y1 + 1));
            grid_line((float)b.x2, (float)b.y1, 1.0f, (float)(b.y2 - b.y1 + 1));
            grid_line((float)b.x1, (float)b.y1, (float)(b.x2 - b.x1 + 1), 1.0f);
            grid_line((float)b.x1, (float)b.y2, (float)(b.x2 - b.x1 + 1), 1.0f);
        }

    // Original PvZ plant hitbox: cell origin + (10, 0), 60x80.
    for (auto *p : game_characters.plants)
    {
        const Block &b = cells[p->get_row()][p->get_col()];
        win.draw_rect(b.x1 + 10, b.y1, 60, 80, GREEN);
    }

    // Original PvZ zombie body + attack rects, anchored on the row's cell y1
    // (the original mPosY), fixed 42x115 / 20x115 at +36 / +50 from x.
    const RGB ORANGE(255, 128, 0);
    for (auto *z : game_characters.zombies)
    {
        const int row_y1 = cells[z->row][0].y1;
        win.draw_rect(z->x_location + 36, row_y1, 42, 115, RED);
        win.draw_rect(z->x_location + 50, row_y1, 20, 115, ORANGE);
    }

    // Actual drawn sprite bbox (union of drawn part quads) in cyan.
    for (const Rec &r : recs)
    {
        if (!r.box.any)
            continue;
        win.draw_rect(r.box.ix(), r.box.iy(), r.box.iw(), r.box.ih(), CYAN);
    }

    // Logical anchor cross in yellow.
    for (const Rec &r : recs)
        draw_cross(r.x, r.y);
}

/*glReadPixels + IMG_SavePNG, same pixel-format fix as the anim viewer's F12
dump (ABGR8888 little-endian keeps R,G,B,A byte order for the vendored
IMG_SavePNG, which ignores masks).*/
bool save_png(const char *path)
{
    win.sprite_batch().flush();
    vector<unsigned char> buf((size_t)WINDOW_WIDTH * WINDOW_HEIGHT * 4);
    pvzgl::glReadPixels(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, pvzgl::PVZ_GL_RGBA,
                        pvzgl::PVZ_GL_UNSIGNED_BYTE, buf.data());
    SDL_Surface *shot = SDL_CreateRGBSurfaceWithFormat(
        0, WINDOW_WIDTH, WINDOW_HEIGHT, 32, SDL_PIXELFORMAT_ABGR8888);
    if (shot == nullptr)
    {
        SDL_Log("scene_dump: SDL_CreateRGBSurfaceWithFormat failed: %s", SDL_GetError());
        return false;
    }
    for (int y = 0; y < WINDOW_HEIGHT; y++)
        memcpy((char *)shot->pixels + (size_t)y * shot->pitch,
               buf.data() + (size_t)(WINDOW_HEIGHT - 1 - y) * WINDOW_WIDTH * 4,
               (size_t)WINDOW_WIDTH * 4);
    const bool ok = IMG_SavePNG(shot, path) == 0;
    if (!ok)
        SDL_Log("scene_dump: IMG_SavePNG %s failed: %s", path, SDL_GetError());
    SDL_FreeSurface(shot);
    return ok;
}

void write_report(const string &path, const string &scene, int lv, int ticks,
                  const vector<Rec> &recs, const vector<string> &skipped)
{
    ofstream out(path);
    if (!out)
        return;
    out << "# pvz scene dump\n";
    out << "# scene=" << scene << " level=" << lv << " ticks=" << ticks
        << " world_scale=" << WORLD_SCALE << "\n";
    out << "# plant hitbox = green (cell.x1+10, cell.y1, 60, 80)\n";
    out << "# zombie body = red (x+36, row.y1, 42, 115); attack = orange "
           "(x+50, row.y1, 20, 115)\n";
    out << "# drawn sprite bbox = cyan; logical anchor cross = yellow; grid = "
           "white 50%\n";
    out << "# type row col/x anchor=(x,y) bbox=(x,y,w,h) scale\n";
    for (const Rec &r : recs)
    {
        out << r.kind << ' ';
        if (strcmp(r.kind, "plant") == 0)
            out << plant_name(r.type) << " type=" << r.type;
        else if (strcmp(r.kind, "zombie") == 0)
            out << zombie_name(r.type) << " type=" << r.type;
        else
            out << "type=" << r.type;
        out << " row=" << r.row << ' ';
        if (r.col >= 0)
            out << "col=" << r.col;
        else
            out << "x=" << r.x;
        out << " anchor=(" << r.x << ',' << r.y << ')';
        if (r.box.any)
            out << " bbox=(" << r.box.ix() << ',' << r.box.iy() << ','
                << r.box.iw() << ',' << r.box.ih() << ')';
        else
            out << " bbox=(none)";
        out << " scale=" << WORLD_SCALE << '\n';
    }
    out << "# skipped:";
    if (skipped.empty())
        out << " none";
    else
        for (const string &s : skipped)
            out << ' ' << s;
    out << '\n';
}
} // namespace

void capture_begin()
{
    g_box = Box();
    g_capture_active = true;
}

void capture_end()
{
    g_capture_active = false;
}

void capture_quad(const float xs[4], const float ys[4])
{
    if (!g_capture_active)
        return;
    for (int i = 0; i < 4; i++)
        g_box.add(xs[i], ys[i]);
}

bool run_if_requested()
{
    const char *out_env = getenv("PVZ_SCENE_DUMP");
    if (out_env == nullptr || out_env[0] == '\0')
        return false;
    const string out_path = out_env;

    const char *scene_env = getenv("PVZ_SCENE");
    const string scene = (scene_env != nullptr && scene_env[0] != '\0') ? scene_env : "day";
    int ticks = 30;
    if (const char *t = getenv("PVZ_SCENE_TICKS"))
        ticks = atoi(t);
    ticks = max(0, min(ticks, 100000));

    SDL_Log("scene_dump: scene=%s ticks=%d out=%s", scene.c_str(), ticks, out_path.c_str());

    // Almanac page renders bypass the level/entity setup: they only need the
    // reanim assets and a fully unlocked player (so the plants grid is shown
    // in full, like the live scene after the book is found).
    if (scene == "almanac_index" || scene == "almanac_plants" ||
        scene == "almanac_zombies")
    {
        int entry = 0;
        if (const char *e = getenv("PVZ_ALMANAC_ENTRY"))
            entry = atoi(e);
        if (!reanimz::ready())
            reanimz::init(win.sprite_renderer());
        if (!reanimplant::ready())
            reanimplant::init(win.sprite_renderer());
        player.unlocked_level = 99;
        const int page = scene == "almanac_plants" ? 1
                         : scene == "almanac_zombies" ? 2
                                                      : 0;
        almanac_render_page(page, entry, 0.0f);
        const bool ok = save_png(out_path.c_str());
        SDL_Log("scene_dump: almanac page=%d entry=%d %s", page, entry,
                ok ? "saved" : "SAVE FAILED");
        return true;
    }

    // Level setup through the normal loader (grid geometry + background +
    // mowers). No menus, no save data, no seed chooser.
    level.level_num = level_for_scene(scene);
    load_level();
    level.background_directory = BACKGROUND_LV1_DIRECTORY + level.map_type;
    level.sky_sun_clk = INF; // no sky sun in the dump
    level.sky_sun_fallen = 0;

    // Drop the level JSON's pre-planted props (roof pots on 5-1): the dump
    // scene is exactly the entity list below.
    for (auto *p : game_characters.plants)
        delete p;
    game_characters.plants.clear();
    for (int row = 0; row < (int)cells.size(); row++)
        for (int col = 0; col < HORIZ_BLOCK_COUNT; col++)
        {
            cells[row][col].is_planted = false;
            cells[row][col].is_block_zombie = false;
        }

    // PopCap reanim skins (the normal gameplay look); the render paths fall
    // back to the sprite sheets if the assets are missing.
    if (!reanimz::ready())
        reanimz::init(win.sprite_renderer());
    if (!reanimplant::ready())
        reanimplant::init(win.sprite_renderer());

    vector<string> skipped;

    if (scene == "plants")
    {
        // Reference peashooter in column 0 of every lane, then every plant
        // type the game supports across columns 1..8, row-major (40 types fit
        // the 5x8 block exactly).
        for (int row = 0; row < (int)cells.size(); row++)
            add_plant(PEASHOOTER_TYPE, row, 0, skipped);
        for (int t = 0; t < PLANT_COUNT; t++)
        {
            int row = t / (HORIZ_BLOCK_COUNT - 1);
            int col = t % (HORIZ_BLOCK_COUNT - 1) + 1;
            if (row < (int)cells.size())
                add_plant(t, row, col, skipped);
        }
    }
    else if (scene == "zombies")
    {
        // Every zombie type, five per lane at the standard dump x slots.
        // Day has five lanes; the 26th type spills onto a sixth row slot if
        // the level ever has one, otherwise it is parked on lane 0 off the
        // right edge so it still appears in the report.
        static const int zx[5] = {300, 440, 580, 720, 860};
        for (int t = 0; t < COUNT_ZOMBIE_TYPE; t++)
        {
            int row = t / 5;
            int x = zx[t % 5];
            if (row >= (int)cells.size())
            {
                row = 0;
                x = 1000;
            }
            add_zombie(t, row, x, skipped);
        }
    }
    else
    {
    // Row 0: one of every classic front-yard plant + an armed Potato Mine.
    add_plant(PEASHOOTER_TYPE, 0, 0, skipped);
    add_plant(SUNFLOWER_TYPE, 0, 1, skipped);
    add_plant(WALLNUT_TYPE, 0, 2, skipped);
    add_plant(SNOWPEA_TYPE, 0, 3, skipped);
    Plants *pm = add_plant(POTATOMINE_TYPE, 0, 4, skipped);
    if (pm != nullptr && !arm_potato_mine(pm))
        skipped.push_back("potatomine(not armed)");
    add_plant(REPEATER_TYPE, 0, 5, skipped);
    add_plant(CHOMPER_TYPE, 0, 6, skipped);
    add_plant(TALLNUT_TYPE, 0, 7, skipped);

    // Row 1: four walkers spread across the lane.
    add_zombie(NORMAL_TYPE, 1, 300, skipped);
    add_zombie(CONE_TYPE, 1, 450, skipped);
    add_zombie(BUCKET_TYPE, 1, 600, skipped);
    add_zombie(FLAG_TYPE, 1, 750, skipped);

    // Row 2: the armored / special walkers.
    add_zombie(NEWSPAPER_TYPE, 2, 300, skipped);
    add_zombie(DOOR_TYPE, 2, 450, skipped);
    add_zombie(FOOTBALL_TYPE, 2, 600, skipped);
    add_zombie(POLEVAULT_TYPE, 2, 750, skipped);

    // Row 3: a pea in flight over c2, a resting sky sun, the parked mowers
    // (already created by load_level) and one walker at x=800.
    game_characters.bullets.push_back(new Pea(3, cells[3][2].x1 + 40));
    {
        Sun s;
        s.final_row = 3;
        s.final_col = 3; // display_suns pins a sky sun to cells[0][col].x1 == 500
        s.x_location = 500;
        s.y_location = 380;
        s.wait_seconds = 0;
        s.value = NORMAL_SUN_VALUE;
        game_characters.suns.push_back(s);
    }
    add_zombie(NORMAL_TYPE, 3, 800, skipped);

    // Row 4: peashooter at c3 + a normal zombie parked exactly where the
    // game's own reach test first locks it into the eating state.
    add_plant(PEASHOOTER_TYPE, 4, 3, skipped);
    {
        Zombie *z = init_zombie(NORMAL_TYPE, level.level_num);
        if (z == nullptr)
            skipped.push_back("normal(r4,eating)");
        else
        {
            z->row = 4;
            z->y_location = cells[4][0].y1 - 50;
            z->is_moving = true;
            int x = cells[4][3].x2 + 120;
            z->x_location = x;
            while (x > cells[4][3].x1 && !has_zombie_reached_plant(*z))
                z->x_location = --x;
            game_characters.zombies.push_back(z);
        }
    }
    }

    // Age the animations by PVZ_SCENE_TICKS (default 30) without a gameplay
    // tick, then draw the real frame + overlay.
    advance_animation(ticks);
    draw_scene_frame();
    vector<Rec> recs;
    capture_bounds(recs);
    draw_overlay(recs);

    const bool ok = save_png(out_path.c_str());
    write_report(out_path + ".txt", scene, level.level_num, ticks, recs, skipped);
    SDL_Log("scene_dump: %s (%d entities, %d skipped)",
            ok ? "saved" : "SAVE FAILED", (int)recs.size(), (int)skipped.size());
    return true;
}
} // namespace scene_dump
