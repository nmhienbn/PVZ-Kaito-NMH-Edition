#include "layout_editor.hpp"
#include "draw/render_elements.hpp"
#include "draw/rsdl.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/Player/Player.hpp"
#include "elements/bullets/bullet.hpp"
#include "elements/button/button.hpp"
#include "elements/conveyor/conveyor.hpp"
#include "elements/elements.hpp"
#include "elements/plants/support_plant.hpp"
#include "elements/sun/sun.hpp"
#include "level/plants_registry.hpp"
#include "level/wave_gen.hpp"
#include <SDL_mixer.h>
#include <cmath>
#include <cstdio>
#include <functional>
#include <vector>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

extern int game_state;
extern bool quit;
extern int clk;
extern Level level;
extern Elements game_characters;
extern Player player;
extern Map cells;
extern Window win;

namespace layout_editor
{
void open_editor(); // nested blocking scene, run by the filter below

namespace
{
bool g_editor_active = false;

// Live redraw for the scene under the editor, registered per frame by
// game.cpp. Null means the scene cannot be re-rendered from here (selector
// screen, seed chooser, award cutscenes: full-scene blockers owning their
// own frame loop) and the editor replays the frozen last-presented frame
// instead -- the picture is pixel-accurate but frozen.
std::function<void()> g_redraw;

// Swallow F2 keydowns everywhere and open the editor right from the event
// filter: every scene in the game (menus, seed chooser, credit pan, sod
// roll, READY-SET-PLANT, gameplay) pumps SDL events once per frame, so this
// fires in all of them without any scene-side hook. The scene that picked
// the event up resumes when the editor closes.
int SDLCALL f2_filter(void *, SDL_Event *event)
{
    if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_F2 &&
        !event->key.repeat)
    {
        if (g_editor_active)
            return 1; // the editor's own loop uses F2 as its exit key
        // Modal in-level sub-menus (restart/leave dialog) keep F2 inert --
        // the same gate the F1 anim viewer uses.
        if (check_status(game_state, IS_RESTART) || check_status(game_state, IS_LEAVE))
            return 0;
        open_editor();
        return 0;
    }
    return 1;
}

enum class ObjKind
{
    NONE,
    PLANT,
    ZOMBIE,
    MOWER,
    SEED_PACKET,
    SUN,
    BULLET,
};

/*The scene class under the editor, from game_state flags: GAMEPLAY = the
level loop proper (all groups), SURVEY = the intro pan / READY-SET-PLANT
(mowers + seed bank exist), CHOOSER = the seed chooser (seed bank only),
MENU = everything else (no board objects at all).*/
enum class SceneClass
{
    MENU,
    SURVEY,
    CHOOSER,
    GAMEPLAY,
};

SceneClass scene_class()
{
    if (check_status(game_state, IS_GAME_STARTED))
        return SceneClass::GAMEPLAY;
    if (check_status(game_state, IS_CHOOSING_SEEDS))
        return SceneClass::CHOOSER;
    if (check_status(game_state, IS_LEVEL_CHOSEN))
        return SceneClass::SURVEY;
    return SceneClass::MENU;
}

/*Which object groups the current scene may enumerate. Plants/zombies are
gameplay-only: load_level() pre-plants roof flower pots BEFORE the seed
chooser runs, so hit-testing them from a frozen chooser/menu frame (where the
camera pan renders the lawn content off-window) selected invisible objects --
the "click picks a flower pot that should be at negative coords" bug.*/
bool group_enumerated(ObjKind kind, SceneClass scene)
{
    switch (scene)
    {
    case SceneClass::GAMEPLAY:
        return true; // plants, zombies, mowers, seed packets, ...
    case SceneClass::SURVEY:
        return kind == ObjKind::MOWER || kind == ObjKind::SEED_PACKET;
    case SceneClass::CHOOSER:
        return kind == ObjKind::SEED_PACKET;
    default:
        return false;
    }
}

// A bbox fully outside the window can never be a click target.
bool on_screen(int x, int y, int w, int h)
{
    return !(x + w < 0 || y + h < 0 || x > WINDOW_WIDTH || y > WINDOW_HEIGHT);
}

struct Rect
{
    int x = 0, y = 0, w = 0, h = 0;
};

struct Selection
{
    ObjKind kind = ObjKind::NONE;
    void *ptr = NULL; // live object address; re-validated every frame
};

Selection sel;

// Editor-session scale factors, keyed by live object address. scale=1 means
// no override. Each frame these are pushed into the rsdl debug-scale
// registry; every selectable render path (plants, zombies, mowers, seed
// packets, suns) reads it and scales for real. Bullets stay measure-only.
struct ScaleEntry
{
    ObjKind kind;
    void *ptr;
    float scale;
};
std::vector<ScaleEntry> g_scales;

float &scale_of(ObjKind kind, void *ptr)
{
    for (auto &e : g_scales)
        if (e.kind == kind && e.ptr == ptr)
            return e.scale;
    g_scales.push_back({kind, ptr, 1.0f});
    return g_scales.back().scale;
}

/*Per-kind transient layout offsets (px, render-only). Plants/Zombies/Mowers/
Suns carry them as live-object fields; seed packets keep them in the
SeedPacket layout registry (see SeedPacket.hpp). Bullets have none.*/
void remember_dirty(ObjKind kind, void *ptr); // defined with g_dirty below
void get_layout_offset(ObjKind kind, const void *ptr, int &dx, int &dy)
{
    switch (kind)
    {
    case ObjKind::PLANT:
        dx = ((const Plants *)ptr)->layout_dx;
        dy = ((const Plants *)ptr)->layout_dy;
        return;
    case ObjKind::ZOMBIE:
        dx = ((const Zombie *)ptr)->layout_dx;
        dy = ((const Zombie *)ptr)->layout_dy;
        return;
    case ObjKind::MOWER:
        dx = ((const Mower *)ptr)->layout_dx;
        dy = ((const Mower *)ptr)->layout_dy;
        return;
    case ObjKind::SUN:
        dx = ((const Sun *)ptr)->layout_dx;
        dy = ((const Sun *)ptr)->layout_dy;
        return;
    case ObjKind::SEED_PACKET:
        seed_packet_layout_get((const SeedPacket *)ptr, dx, dy);
        return;
    default:
        dx = dy = 0;
        return;
    }
}

void set_layout_offset(ObjKind kind, void *ptr, int dx, int dy)
{
    switch (kind)
    {
    case ObjKind::PLANT:
        ((Plants *)ptr)->layout_dx = dx;
        ((Plants *)ptr)->layout_dy = dy;
        break;
    case ObjKind::ZOMBIE:
        ((Zombie *)ptr)->layout_dx = dx;
        ((Zombie *)ptr)->layout_dy = dy;
        break;
    case ObjKind::MOWER:
        ((Mower *)ptr)->layout_dx = dx;
        ((Mower *)ptr)->layout_dy = dy;
        break;
    case ObjKind::SUN:
        ((Sun *)ptr)->layout_dx = dx;
        ((Sun *)ptr)->layout_dy = dy;
        break;
    case ObjKind::SEED_PACKET:
        seed_packet_layout_set((const SeedPacket *)ptr, dx, dy);
        break;
    default:
        return;
    }
    remember_dirty(kind, ptr);
}

// Grow a bbox about its bottom-center contact point (the pivot the plant/
// zombie/mower render paths scale around).
Rect scale_about_bottom_center(const Rect &r, float s)
{
    int nw = (int)lroundf(r.w * s), nh = (int)lroundf(r.h * s);
    return {r.x + (r.w - nw) / 2, r.y + r.h - nh, nw, nh};
}

// Grow a bbox about its center (the pivot SeedPacket::display scales around;
// also how display_suns scales the sun icon).
Rect scale_about_center(const Rect &r, float s)
{
    int nw = (int)lroundf(r.w * s), nh = (int)lroundf(r.h * s);
    return {r.x + (r.w - nw) / 2, r.y + (r.h - nh) / 2, nw, nh};
}

// Spawn-default render offsets (the roof bake in init_plant): R and the
// on-close restore must return an edited object to its DEFAULTS, not to
// zero — roof flower pots and their passengers carry tuned offsets from
// spawn, so zeroing them on an editor roundtrip would strip the bake.
void default_layout_offset(ObjKind kind, void *ptr, int &dx, int &dy)
{
    dx = dy = 0;
    if (kind != ObjKind::PLANT || !level.is_roof())
        return;
    Plants *p = (Plants *)ptr;
    if (p->get_type() == FLOWERPOT_TYPE)
        roof_pot_layout_default(p->get_row(), p->get_col(), dx, dy);
    else
    {
        Plants *pot = find_support_plant(game_characters.plants, p->get_row(),
                                         p->get_col());
        if (pot != NULL && pot->get_type() == FLOWERPOT_TYPE)
            roof_passenger_layout_default(p->get_row(), p->get_col(), dx, dy);
    }
}

// R (reset) target state; restore-all on close uses the same default-aware
// path via restore_all_edits below.
struct DirtyEntry
{
    ObjKind kind;
    void *ptr;
};
std::vector<DirtyEntry> g_dirty;

void remember_dirty(ObjKind kind, void *ptr)
{
    for (auto &e : g_dirty)
        if (e.kind == kind && e.ptr == ptr)
            return;
    g_dirty.push_back({kind, ptr});
}

// Same "is this pointer still a live object of this kind" test as
// valid_selection, reusable for the edit-state tables.
bool ptr_live(ObjKind kind, const void *ptr)
{
    switch (kind)
    {
    case ObjKind::PLANT:
        for (auto &plant : game_characters.plants)
            if (plant == ptr)
                return true;
        return false;
    case ObjKind::ZOMBIE:
        for (auto &zombie : game_characters.zombies)
            if (zombie == ptr)
                return true;
        return false;
    case ObjKind::MOWER:
        for (auto &mower : game_characters.mowers)
            if (mower == ptr)
                return true;
        return false;
    case ObjKind::SEED_PACKET:
        for (auto &packet : player.seed_packets)
            if (&packet == ptr)
                return true;
        return false;
    case ObjKind::SUN:
        for (auto &sun : game_characters.suns)
            if (&sun == ptr)
                return true;
        return false;
    case ObjKind::BULLET:
        for (auto &bullet : game_characters.bullets)
            if (bullet == ptr)
                return true;
        return false;
    default:
        return false;
    }
}

// Drop edit-state entries whose object no longer exists (level reset, mower
// used up, zombie died): stale addresses would otherwise collide with a
// future allocation at the same address. Dropping also clears the rsdl /
// packet-registry residue so a same-address reuse starts at identity.
void prune_edit_state()
{
    for (auto it = g_scales.begin(); it != g_scales.end();)
        if (ptr_live(it->kind, it->ptr))
            ++it;
        else
        {
            rsdl_clear_debug_scale(it->ptr);
            it = g_scales.erase(it);
        }
    for (auto it = g_dirty.begin(); it != g_dirty.end();)
        if (ptr_live(it->kind, it->ptr))
            ++it;
        else
        {
            if (it->kind == ObjKind::SEED_PACKET)
                seed_packet_layout_clear((const SeedPacket *)it->ptr);
            it = g_dirty.erase(it);
        }
}

// Push every recorded scale into the rsdl debug registry; every selectable
// render path reads it there (plants/zombies/mowers/packets/suns).
void apply_debug_scales()
{
    for (auto &e : g_scales)
        if (e.scale != 1.0f)
            rsdl_set_debug_scale(e.ptr, e.scale);
}

// On editor close every edit reverts: offsets zero out on the live objects /
// packet registry, scale overrides drop with the session scale table (the
// close path also clears the rsdl registry they were pushed into).
void restore_all_edits()
{
    for (auto &e : g_dirty)
        if (ptr_live(e.kind, e.ptr))
        {
            int dx, dy;
            default_layout_offset(e.kind, e.ptr, dx, dy);
            set_layout_offset(e.kind, e.ptr, dx, dy);
        }
    g_dirty.clear();
    g_scales.clear();
    seed_packet_layout_clear_all();
}

// Fallback scene redraw when no live callback was registered: replay the
// last presented frame (frozen for the editor session).
void replay_last_frame()
{
    const GlTexture &shot = win.sprite_batch().lastFrame();
    if (shot.id != 0)
        win.sprite_batch().drawQuad(shot, 0, 0, (float)shot.w, (float)shot.h,
                                    0.0f, 0.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT,
                                    0.0f, false, true); // flipV: snapshot is GL-origin
}

bool g_show_grid = true;

// Lawn tile grid for live-redraw level scenes (same per-column slope step as
// the blink bands, so roof tiles line up with plant placement).
void draw_grid_overlay()
{
    for (int r = 0; r < (int)cells.size(); r++)
        for (int c = 0; c < (int)cells[r].size(); c++)
        {
            const Block &b = cells[r][c];
            win.draw_rect(b.x1, b.y1 + roof_tile_dy(c), b.x2 - b.x1, b.y2 - b.y1, CYAN);
        }
}

bool valid_selection()
{
    if (sel.kind == ObjKind::NONE || sel.ptr == NULL)
        return false;
    // The scene may have changed under the selection (e.g. a picker opened
    // over the frozen frame): a group the current scene does not enumerate
    // is not a valid selection anymore.
    if (!group_enumerated(sel.kind, scene_class()))
        return false;
    return ptr_live(sel.kind, sel.ptr);
}

int packet_index_of(const SeedPacket *p)
{
    for (int i = 0; i < (int)player.seed_packets.size(); i++)
        if (&player.seed_packets[i] == p)
            return i;
    return -1;
}

// Lane-surface line under a mower lane: the shared mower_ground_y() the
// renderer itself anchors to (tuned parked ground on roof, flat y1+97
// elsewhere), so the editor rect always matches the art. The slope delta is
// 0 at the parked anchor, so this rect is exact there.
Rect mower_rect(const Mower *m)
{
    Rect base{(int)m->x, mower_ground_y((int)m->x, m->row) - MOWER_HEIGHT,
              MOWER_WIDTH, MOWER_HEIGHT};
    Rect r = scale_about_bottom_center(base, scale_of(ObjKind::MOWER, (void *)m));
    r.x += m->layout_dx;
    r.y += m->layout_dy;
    return r;
}

Rect plant_rect(const Plants *p)
{
    const Block &b = cells[p->get_row()][p->get_col()];
    Rect base{b.x1, b.y1, b.x2 - b.x1, b.y2 - b.y1};
    Rect r = scale_about_bottom_center(base, scale_of(ObjKind::PLANT, (void *)p));
    r.x += p->layout_dx;
    r.y += p->layout_dy;
    return r;
}

Rect zombie_rect(const Zombie *z)
{
    Rect base{z->zrect_left(), z->zrect_top(),
              z->zrect_right() - z->zrect_left(), z->zrect_bottom() - z->zrect_top()};
    Rect r = scale_about_bottom_center(base, scale_of(ObjKind::ZOMBIE, (void *)z));
    r.x += z->layout_dx;
    r.y += z->layout_dy;
    return r;
}

Rect packet_rect(const SeedPacket *p)
{
    int i = packet_index_of(p);
    if (i < 0 || i >= (int)plant_seed.size())
        return {};
    const Button &b = plant_seed[i];
    Rect base{b.x1, b.y1, b.x2 - b.x1, b.y2 - b.y1};
    Rect r = scale_about_center(base, scale_of(ObjKind::SEED_PACKET, (void *)p));
    int dx, dy;
    seed_packet_layout_get(p, dx, dy);
    r.x += dx;
    r.y += dy;
    return r;
}

// The bbox display_suns actually draws (pop grow-in and the small-sun factor
// included), mirroring its F2 layout offset / debug-scale handling.
Rect sun_rect(const Sun *s)
{
    int w = s->value >= NORMAL_SUN_VALUE ? SUN_WIDTH : SUN_WIDTH / 2;
    if (s->from_plant && s->pop_age < 18)
        w = w * (40 + (60 * s->pop_age) / 18) / 100;
    // Sky suns render on their column's left border (display_suns re-pins
    // x_location every frame); plant-popped suns keep the arc's x.
    int bx = s->from_plant || cells.empty() || s->final_col < 0 ||
                     s->final_col >= (int)cells[0].size()
                 ? s->x_location
                 : cells[0][s->final_col].x1;
    Rect r = scale_about_center({bx, s->y_location, w, w},
                                scale_of(ObjKind::SUN, (void *)s));
    r.x += s->layout_dx;
    r.y += s->layout_dy;
    return r;
}

Rect bullet_rect(const Bullet *b)
{
    return {b->x_location, b->y_location, b->width, b->height};
}

Rect selection_rect()
{
    switch (sel.kind)
    {
    case ObjKind::PLANT:
        return plant_rect((Plants *)sel.ptr);
    case ObjKind::ZOMBIE:
        return zombie_rect((Zombie *)sel.ptr);
    case ObjKind::MOWER:
        return mower_rect((Mower *)sel.ptr);
    case ObjKind::SEED_PACKET:
        return packet_rect((SeedPacket *)sel.ptr);
    case ObjKind::SUN:
        return sun_rect((Sun *)sel.ptr);
    case ObjKind::BULLET:
        return bullet_rect((Bullet *)sel.ptr);
    default:
        return {};
    }
}

const char *bullet_type_name(int t)
{
    static const char *names[] = {"pea", "snowpea", "puff", "fume",
                                  "spike", "star", "lobbed"};
    return (t >= 0 && t < BULLET_TYPE_NUM) ? names[t] : "bullet";
}

string selection_id()
{
    switch (sel.kind)
    {
    case ObjKind::PLANT:
    {
        int t = ((Plants *)sel.ptr)->get_type();
        const PlantDef *def = plant_def_by_type(t);
        return def ? def->key : "plant_" + to_string(t);
    }
    case ObjKind::ZOMBIE:
        return zombie_type_name(((Zombie *)sel.ptr)->type);
    case ObjKind::MOWER:
        return "row_" + to_string(((Mower *)sel.ptr)->row);
    case ObjKind::SEED_PACKET:
    {
        int t = (int)((SeedPacket *)sel.ptr)->plant_name;
        const PlantDef *def = plant_def_by_type(t);
        return def ? def->key : "packet_" + to_string(t);
    }
    case ObjKind::SUN:
    {
        const Sun *s = (const Sun *)sel.ptr;
        return string("sun") + (s->from_plant ? "_plant_" : "_sky_") +
               to_string(s->value) + "@r" + to_string(s->final_row) + "c" +
               to_string(s->final_col);
    }
    case ObjKind::BULLET:
        return string(bullet_type_name(((Bullet *)sel.ptr)->type)) + "@row" +
               to_string(((Bullet *)sel.ptr)->row);
    default:
        return "none";
    }
}

const char *kind_name(ObjKind kind)
{
    switch (kind)
    {
    case ObjKind::PLANT:
        return "plant";
    case ObjKind::ZOMBIE:
        return "zombie";
    case ObjKind::MOWER:
        return "mower";
    case ObjKind::SEED_PACKET:
        return "seed_packet";
    case ObjKind::SUN:
        return "sun";
    case ObjKind::BULLET:
        return "bullet";
    default:
        return "none";
    }
}

// Current engine-side anchor for the dump: zombies use their sprite origin
// (x_location/y_location, the same fields the renderer reads), mowers their
// float x plus the lane surface line, plants/packets the rect origin.
void anchor_of(ObjKind kind, void *ptr, const Rect &r, float &x, float &y)
{
    if (kind == ObjKind::ZOMBIE)
    {
        const Zombie *z = (const Zombie *)ptr;
        x = (float)z->x_location;
        y = (float)z->y_location;
        return;
    }
    if (kind == ObjKind::MOWER)
    {
        const Mower *m = (const Mower *)ptr;
        x = m->x;
        y = (float)r.y;
        return;
    }
    x = (float)r.x;
    y = (float)r.y;
}

void pick_nearest(int mx, int my)
{
    const SceneClass scene = scene_class();
    float best = 1e9f;
    ObjKind best_kind = ObjKind::NONE;
    void *best_ptr = NULL;
    auto consider = [&](ObjKind kind, void *ptr, const Rect &r)
    {
        // A bbox fully outside the window is never a target (stale objects
        // from another scene/level, spawn-holding zombies off the right edge).
        if (!on_screen(r.x, r.y, r.w, r.h))
            return;
        // A click inside the bbox always wins over a merely-near one.
        float dx = mx < r.x ? (float)(r.x - mx) : (mx > r.x + r.w ? (float)(mx - r.x - r.w) : 0.0f);
        float dy = my < r.y ? (float)(r.y - my) : (my > r.y + r.h ? (float)(my - r.y - r.h) : 0.0f);
        float d = sqrtf(dx * dx + dy * dy);
        if (d < best)
        {
            best = d;
            best_kind = kind;
            best_ptr = ptr;
        }
    };
    if (group_enumerated(ObjKind::ZOMBIE, scene))
        for (auto &zombie : game_characters.zombies)
            consider(ObjKind::ZOMBIE, zombie, zombie_rect(zombie));
    if (group_enumerated(ObjKind::PLANT, scene))
        for (auto &plant : game_characters.plants)
            consider(ObjKind::PLANT, plant, plant_rect(plant));
    if (group_enumerated(ObjKind::MOWER, scene))
        for (auto &mower : game_characters.mowers)
            if (mower != NULL)
                consider(ObjKind::MOWER, mower, mower_rect(mower));
    if (group_enumerated(ObjKind::SEED_PACKET, scene) && !belt.active)
        for (auto &packet : player.seed_packets)
            consider(ObjKind::SEED_PACKET, &packet, packet_rect(&packet));
    if (group_enumerated(ObjKind::SUN, scene))
        for (auto &sun : game_characters.suns)
            consider(ObjKind::SUN, &sun, sun_rect(&sun));
    if (group_enumerated(ObjKind::BULLET, scene))
        for (auto &bullet : game_characters.bullets)
            consider(ObjKind::BULLET, bullet, bullet_rect(bullet));
    if (best <= 120.0f)
    {
        sel.kind = best_kind;
        sel.ptr = best_ptr;
    }
    else
    {
        sel = Selection();
    }
}

bool selection_movable()
{
    // Bullets stay measure-only; everything else can be dragged/nudged
    // (through the transient layout offsets).
    return sel.kind == ObjKind::PLANT || sel.kind == ObjKind::ZOMBIE ||
           sel.kind == ObjKind::MOWER || sel.kind == ObjKind::SEED_PACKET ||
           sel.kind == ObjKind::SUN;
}

bool selection_scalable()
{
    // Bullets have no render-side scale hook: measure + dump only.
    return sel.kind != ObjKind::NONE && sel.kind != ObjKind::BULLET;
}

void move_selection(int dx, int dy)
{
    if (!valid_selection())
        return;
    switch (sel.kind)
    {
    case ObjKind::PLANT:
    case ObjKind::ZOMBIE:
    case ObjKind::MOWER:
    case ObjKind::SEED_PACKET:
    case ObjKind::SUN:
    {
        // Transient render offsets; every gameplay anchor (block, lane,
        // x_location, slot rect) stays put.
        int ox;
        int oy;
        get_layout_offset(sel.kind, sel.ptr, ox, oy);
        set_layout_offset(sel.kind, sel.ptr, ox + dx, oy + dy);
        break;
    }
    default:
        break;
    }
}

void reset_selection()
{
    if (!valid_selection())
        return;
    // Everything the edits touch is transient render state: restore the
    // spawn-default offsets (roof bake-aware) and drop the scale override.
    int dx, dy;
    default_layout_offset(sel.kind, sel.ptr, dx, dy);
    set_layout_offset(sel.kind, sel.ptr, dx, dy);
    scale_of(sel.kind, sel.ptr) = 1.0f;
}

void scale_selection(float delta)
{
    if (!valid_selection() || !selection_scalable())
        return;
    float &s = scale_of(sel.kind, sel.ptr);
    s += delta;
    if (s < 0.2f)
        s = 0.2f;
    if (s > 5.0f)
        s = 5.0f;
}

void ensure_userdata_dir()
{
#ifdef _WIN32
    _mkdir("userdata");
#else
    mkdir("userdata", 0755);
#endif
}

int dump_all(string &err)
{
    const SceneClass scene = scene_class();
    ensure_userdata_dir();
    FILE *f = fopen("userdata/layout_dump.txt", "w");
    if (f == NULL)
    {
        err = "cannot open userdata/layout_dump.txt";
        return -1;
    }
    fprintf(f, "# pvz layout editor dump (level %d, clk %d)\n", level.level_num, clk);
    fprintf(f, "# format: type=<kind> id=<specific> x=<anchor px> y=<anchor px> w=<bbox px> h=<bbox px> scale=<editor factor> [dx=<off> dy=<off>] [offscreen=1]\n");
    int n = 0, n_offscreen = 0;
    auto dump_one = [&](ObjKind kind, void *ptr, const char *id, const Rect &r)
    {
        float ax, ay;
        anchor_of(kind, ptr, r, ax, ay);
        float s = scale_of(kind, ptr);
        fprintf(f, "type=%s id=%s x=%.1f y=%.1f w=%d h=%d scale=%.3f",
                kind_name(kind), id, ax, ay, r.w, r.h, s);
        int odx, ody;
        get_layout_offset(kind, ptr, odx, ody);
        if (odx != 0 || ody != 0)
            fprintf(f, " dx=%d dy=%d", odx, ody);
        // Objects completely outside the window still dump, flagged (the
        // pick path refuses them, but the dump keeps the full census).
        if (!on_screen(r.x, r.y, r.w, r.h))
        {
            fprintf(f, " offscreen=1");
            n_offscreen++;
        }
        fputc('\n', f);
        n++;
    };
    if (group_enumerated(ObjKind::PLANT, scene))
        for (auto &plant : game_characters.plants)
        {
            const PlantDef *def = plant_def_by_type(plant->get_type());
            char idbuf[64];
            snprintf(idbuf, sizeof idbuf, "%s@r%dc%d",
                     def ? def->key : "unknown", plant->get_row(), plant->get_col());
            dump_one(ObjKind::PLANT, plant, idbuf, plant_rect(plant));
        }
    if (group_enumerated(ObjKind::ZOMBIE, scene))
        for (auto &zombie : game_characters.zombies)
        {
            char idbuf[64];
            snprintf(idbuf, sizeof idbuf, "%s@row%d",
                     zombie_type_name(zombie->type).c_str(), zombie->row);
            dump_one(ObjKind::ZOMBIE, zombie, idbuf, zombie_rect(zombie));
        }
    if (group_enumerated(ObjKind::MOWER, scene))
        for (auto &mower : game_characters.mowers)
            if (mower != NULL)
            {
                char idbuf[64];
                snprintf(idbuf, sizeof idbuf, "row_%d", mower->row);
                dump_one(ObjKind::MOWER, mower, idbuf, mower_rect(mower));
            }
    if (group_enumerated(ObjKind::SEED_PACKET, scene) && !belt.active)
        for (auto &packet : player.seed_packets)
        {
            const PlantDef *def = plant_def_by_type((int)packet.plant_name);
            char idbuf[64];
            snprintf(idbuf, sizeof idbuf, "%s@slot%d", def ? def->key : "unknown",
                     packet_index_of(&packet));
            dump_one(ObjKind::SEED_PACKET, &packet, idbuf, packet_rect(&packet));
        }
    if (group_enumerated(ObjKind::SUN, scene))
    {
        int idx = 0;
        for (auto &sun : game_characters.suns)
        {
            char idbuf[80];
            snprintf(idbuf, sizeof idbuf, "sun%s_%d@r%dc%d#%d",
                     sun.from_plant ? "_plant" : "_sky", sun.value,
                     sun.final_row, sun.final_col, idx++);
            dump_one(ObjKind::SUN, &sun, idbuf, sun_rect(&sun));
        }
    }
    if (group_enumerated(ObjKind::BULLET, scene))
    {
        int idx = 0;
        for (auto &bullet : game_characters.bullets)
        {
            char idbuf[64];
            snprintf(idbuf, sizeof idbuf, "%s@row%d#%d",
                     bullet_type_name(bullet->type), bullet->row, idx++);
            dump_one(ObjKind::BULLET, bullet, idbuf, bullet_rect(bullet));
        }
    }
    fprintf(f, "# %d objects dumped (%d fully offscreen); scene %d\n", n, n_offscreen, (int)scene);
    fclose(f);
    return n;
}

void draw_box(const Rect &r, const RGB &color)
{
    win.draw_rect(r.x, r.y, r.w, r.h, color);
    win.draw_rect(r.x - 1, r.y - 1, r.w + 2, r.h + 2, color);
}

void draw_overlay(const string &status_line)
{
    if (valid_selection())
    {
        Rect r = selection_rect();
        draw_box(r, RED);
        float s = scale_of(sel.kind, sel.ptr);
        char info[256];
        float ax, ay;
        anchor_of(sel.kind, sel.ptr, r, ax, ay);
        int odx, ody;
        get_layout_offset(sel.kind, sel.ptr, odx, ody);
        // Every selectable render path honors the scale now (bullets are the
        // read-only measurement exception); the box drawn above is the real,
        // scaled footprint.
        snprintf(info, sizeof info, "sel: type=%s id=%s x=%.1f y=%.1f w=%d h=%d scale=%.3f dx=%d dy=%d%s%s",
                 kind_name(sel.kind), selection_id().c_str(), ax, ay, r.w, r.h, s, odx, ody,
                 sel.kind == ObjKind::BULLET ? "  (read-only)" : "",
                 on_screen(r.x, r.y, r.w, r.h) ? "" : "  (offscreen)");
        win.show_text_shadowed(info, 15, 100, CYAN, BRIANNE_TTF, 18, 1);
    }
    else
    {
        const SceneClass sc = scene_class();
        win.show_text_shadowed(sc == SceneClass::GAMEPLAY
                                   ? "no selection - click near a plant/zombie/mower/seed packet/sun/bullet"
                                   : sc == SceneClass::SURVEY
                                         ? "no selection - mowers + seed packets selectable in this view"
                                         : sc == SceneClass::CHOOSER
                                               ? "no selection - seed packets selectable in the chooser"
                                               : "no selection - nothing selectable in menus (coords + F9 dump only)",
                               15, 100, CYAN, BRIANNE_TTF, 18, 1);
    }
    // Raw mouse coordinates, plus the tile under the cursor on board scenes:
    // the measurement readout for screens where nothing is selectable.
    int mx = 0, my = 0;
    SDL_GetMouseState(&mx, &my);
    char mpos[128];
    int col = cells.empty() ? -1 : get_block_col(mx);
    int row = cells.empty() ? -1 : get_block_row(mx, my);
    int n = snprintf(mpos, sizeof mpos, "mouse x=%d y=%d", mx, my);
    if (row >= 0 && col >= 0)
        snprintf(mpos + n, sizeof mpos - n, "  cell r%d c%d", row, col);
    win.show_text_shadowed(mpos, 15, 126, CYAN, BRIANNE_TTF, 18, 1);
    win.show_text_shadowed("F2/ESC exit  click select  drag+arrows move (Shift x10)  +/- wheel scale  R reset  G grid  F9 dump",
                           15, WINDOW_HEIGHT - 32, WHITE, BRIANNE_TTF, 16, 1);
    if (!status_line.empty())
        win.show_text_shadowed(status_line, 15, 152, GREEN, BRIANNE_TTF, 18, 1);
}
} // namespace

void install()
{
    SDL_SetEventFilter(f2_filter, NULL);
}

void set_scene_redraw(std::function<void()> redraw)
{
    g_redraw = redraw;
}

// Blocking nested scene, invoked by the F2 event filter from whatever scene
// pump picked the key up. Redraws the registered scene callback while
// gameplay is frozen (IS_PAUSED holds every renderer's frame/animation
// counters); with no callback it replays the frozen last frame instead.
void open_editor()
{
    const bool was_paused = check_status(game_state, IS_PAUSED);
    const int n_channels = Mix_AllocateChannels(-1);
    std::vector<bool> channel_paused((size_t)n_channels);
    for (int i = 0; i < n_channels; i++)
        channel_paused[i] = Mix_Paused(i) != 0;
    const bool music_paused = Mix_PausedMusic() != 0;
    Mix_Pause(-1);
    Mix_PauseMusic();
    set_status(game_state, IS_PAUSED, true);
    g_editor_active = true;
    // Keep the last presented frame intact for the frozen-scene replay path.
    win.sprite_batch().setLastFrameFrozen(true);

    bool leave = false;
    bool dragging = false;
    int grab_dx = 0, grab_dy = 0;
    string status_line;
    while (!quit && !leave)
    {
        if (!valid_selection())
            sel = Selection();
        prune_edit_state();
        apply_debug_scales();
        win.clear_renderer();
        if (g_redraw)
            g_redraw();
        else
            replay_last_frame();
        if (g_redraw && g_show_grid && check_status(game_state, IS_LEVEL_CHOSEN))
            draw_grid_overlay();
        draw_overlay(status_line);

        HANDLE_SDL_EVENT(
            QUIT();
            KEY_PRESS(F2, leave = true;);
            KEY_PRESS(ESCAPE, leave = true;);
            LCLICK({
                pick_nearest(mouse_x, mouse_y);
                dragging = false;
                if (valid_selection() && selection_movable())
                {
                    // Grab relative to the visual box origin so the sprite
                    // edge follows the cursor from where it was picked up.
                    Rect r = selection_rect();
                    grab_dx = mouse_x - r.x;
                    grab_dy = mouse_y - r.y;
                    dragging = true;
                }
            });
            LRELEASE(dragging = false; (void)mouse_x; (void)mouse_y;);
            if (e.type == SDL_MOUSEMOTION && dragging && valid_selection() &&
                selection_movable())
            {
                // Drag the transient render offset so the visual rect origin
                // tracks the cursor (gameplay anchors untouched).
                Rect r = selection_rect();
                int ox;
                int oy;
                get_layout_offset(sel.kind, sel.ptr, ox, oy);
                set_layout_offset(sel.kind, sel.ptr,
                                  ox + (e.motion.x - grab_dx) - r.x,
                                  oy + (e.motion.y - grab_dy) - r.y);
            };
            if (e.type == SDL_MOUSEWHEEL)
                scale_selection(e.wheel.y > 0 ? 0.1f : -0.1f);
            if (e.type == SDL_KEYDOWN)
            {
                const int step = (e.key.keysym.mod & KMOD_SHIFT) ? 10 : 1;
                switch (e.key.keysym.sym)
                {
                case SDLK_LEFT:
                    move_selection(-step, 0);
                    break;
                case SDLK_RIGHT:
                    move_selection(step, 0);
                    break;
                case SDLK_UP:
                    move_selection(0, -step);
                    break;
                case SDLK_DOWN:
                    move_selection(0, step);
                    break;
                case SDLK_EQUALS:
                case SDLK_KP_PLUS:
                    scale_selection(step > 1 ? 0.2f : 0.05f);
                    break;
                case SDLK_MINUS:
                case SDLK_KP_MINUS:
                    scale_selection(step > 1 ? -0.2f : -0.05f);
                    break;
                case SDLK_r:
                    reset_selection();
                    break;
                case SDLK_g:
                    g_show_grid = !g_show_grid;
                    break;
                case SDLK_F9:
                {
                    string err;
                    int n = dump_all(err);
                    status_line = err.empty()
                                      ? "dumped " + to_string(n) + " objects -> userdata/layout_dump.txt"
                                      : "dump failed: " + err;
                    break;
                }
                default:
                    break;
                }
            });
        win.update_screen();
    }

    restore_all_edits();
    rsdl_clear_debug_scales();
    win.sprite_batch().setLastFrameFrozen(false);
    set_status(game_state, IS_PAUSED, was_paused);
    for (int i = 0; i < n_channels; i++)
        if (!channel_paused[i])
            Mix_Resume(i);
    if (!music_paused)
        Mix_ResumeMusic();
    g_editor_active = false;
}
} // namespace layout_editor
