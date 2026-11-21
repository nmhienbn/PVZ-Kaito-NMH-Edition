#include "magnetshroom.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/elements.hpp"
#include "elements/zombies/zombie_struct.hpp"
#include "elements/zombies/pogo/pogo_zombie.hpp"
#include "elements/zombies/jackbox/jackbox_zombie.hpp"
#include "elements/zombies/digger/digger_zombie.hpp"
#include "game_param.hpp"
#include <cstdlib>

// Magnetshroom.reanim (fps 12 == 5 ticks/frame): idle 0..44, suck 45..58,
// charging 59..84, sleep 85..109.
#define MAGNET_ASSET_FRAMES 5
// Suck flourish: one pass of anim_shooting (14 frames x 5 ticks) + change.
#define MAGNET_SUCK_TICKS (14 * MAGNET_ASSET_FRAMES)
// Recharge after a successful suck: ~15 s (original: 1500 cs).
#define MAGNET_RECHARGE_TICKS (15 * 60)
// Pull radius (original: 270 px, 320 px while the zombie is eating).
#define MAGNET_RADIUS 270
#define MAGNET_RADIUS_EATING 320

static PlantAssets MAGNET_ASSET(MAGNET_ASSET_FRAMES, 60, 60, 60, 60, 12, 18);

extern int game_state;
extern Map cells;
extern Window win;
extern Level level;
extern Elements game_characters;

MagnetShroom::MagnetShroom(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = MAGNETSHROOM_TYPE;
    health = PLANT_HEALTH_LIMIT[MAGNETSHROOM_TYPE];
    sec_for_prepare = 1;
    directory_num = is_night() ? MAGNETSHROOM_SHEET_DIRECTORY : MAGNETSHROOM_SLEEP_DIRECTORY;
    attack_range = NO_ATTACK;
}

MagnetShroom::~MagnetShroom()
{
}

bool MagnetShroom::is_night() const
{
    return level.is_night() || coffee_awake; // a coffee bean wakes it by day
}

namespace
{
/*Body-only health cap: the second health step of the type (the tier where
its metal gear starts). Anything above it is metal armor that can fly off.
Only used for the armor types; tools (pogo stick, jack-in-the-box, digger
pickaxe) are damage-free steals in the original (repvz Plant.cpp
MagnetShroomAttactItem) and get their own hooks instead.*/
float body_health_cap(int zombie_type)
{
    const set<float> &dims = ZOMBIE_HEALTH_LIMIT[zombie_type];
    if (dims.size() < 2)
        return *dims.begin();
    return *next(dims.begin());
}

/*Zombie gear the magnet can pull right now (repvz PlantMagnetIsTargets):
metal helmets/shields with armor points left, or a tool still in hand.*/
bool magnet_targets(const Zombie *z)
{
    switch (z->type)
    {
    case BUCKET_TYPE:
    case DOOR_TYPE:
    case FOOTBALL_TYPE:
    case LADDER_TYPE:
        return const_cast<Zombie *>(z)->get_health() > body_health_cap(z->type) * z->hp_scale(); // armor tier left
    case POGO_TYPE:
        return static_cast<const PogoZombie *>(z)->has_stick();
    case JACKBOX_TYPE:
        return static_cast<const JackboxZombie *>(z)->has_box();
    case DIGGER_TYPE:
        return static_cast<const DiggerZombie *>(z)->can_lose_pickaxe();
    default:
        return false;
    }
}

/*Strip the gear: armor types pay the armor tier off their pooled health
(the armor_drop/degrade visuals fire there); tools come off damage-free
via their subclass hooks.*/
void pull_gear(Zombie *z, vector<ZombiePart> &zombie_parts)
{
    switch (z->type)
    {
    case POGO_TYPE:
        static_cast<PogoZombie *>(z)->drop_pogo();
        return;
    case JACKBOX_TYPE:
        static_cast<JackboxZombie *>(z)->steal_jackbox();
        return;
    case DIGGER_TYPE:
        static_cast<DiggerZombie *>(z)->lose_pickaxe();
        return;
    default:
        z->take_hit(z->get_health() - body_health_cap(z->type) * z->hp_scale(), zombie_parts);
        return;
    }
}
} // namespace

/*
Find the closest metal-carrying zombie in range and strip its gear: the
degrade/turn-to-basic visuals come for free from the zombie's armor_drop at
the body tier (bucket/door/football conversions already drop the part).
@return true when an item was pulled (starts the recharge).
*/
bool MagnetShroom::try_suck(vector<Zombie *> &zombies, vector<ZombiePart> &zombie_parts)
{
    int cx = (cells[row][col].x1 + cells[row][col].x2) / 2;
    int cy = ((cells[row][col].y1 + roof_tile_dy(col)) + (cells[row][col].y2 + roof_tile_dy(col))) / 2;
    Zombie *closest = nullptr;
    int closest_d2 = 0x7fffffff;
    for (auto *z : zombies)
    {
        if (z->is_charmed || !magnet_targets(z))
            continue;
        if (abs(z->row - row) > 2)
            continue;
        int zx = z->x_location + ZOMBIE_EXACT_LOCATION / 2;
        int zy = cells[z->row][0].y1 + 40;
        int dx = zx - cx, dy = zy - cy;
        int radius = z->is_moving ? MAGNET_RADIUS : MAGNET_RADIUS_EATING;
        int d2 = dx * dx + dy * dy;
        if (d2 > radius * radius)
            continue;
        if (d2 < closest_d2)
        {
            closest_d2 = d2;
            closest = z;
        }
    }
    if (!closest)
        return false;
    play_sound_effect(MAGNETSHROOM_MUSIC_DIRECTORY);
    pull_gear(closest, zombie_parts);
    return true;
}

void MagnetShroom::action()
{
    if (!is_night())
        return;
    if (directory_num == MAGNETSHROOM_SLEEP_DIRECTORY)
    {
        // coffee bean wake-up: rise from the sleep loop to the idle reel
        directory_num = MAGNETSHROOM_SHEET_DIRECTORY;
        frame = 0;
    }
    if (suck_time > 0)
    {
        if (--suck_time == 0)
            directory_num = MAGNETSHROOM_CHARGE_DIRECTORY;
        return;
    }
    if (charge > 0)
    {
        if (--charge == 0)
            directory_num = MAGNETSHROOM_SHEET_DIRECTORY;
        return;
    }
    if (try_suck(game_characters.zombies, game_characters.zombie_parts))
    {
        directory_num = MAGNETSHROOM_SUCK_DIRECTORY;
        frame = 0;
        suck_time = MAGNET_SUCK_TICKS;
        charge = MAGNET_RECHARGE_TICKS;
    }
}

void MagnetShroom::display(const int &_row)
{
    if (row != _row)
        return;
    if (display_reanim(MAGNET_ASSET))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        next_frame(MAGNET_ASSET);
        return;
    }
    win.draw_png_width_scaled(MAGNETSHROOM_DIRECTORY, cells[row][col].x1 + 10,
                              (cells[row][col].y1 + roof_tile_dy(col)) + 25, world_px(ELEMENT_WIDTH - 30));
    next_frame(MAGNET_ASSET);
}
