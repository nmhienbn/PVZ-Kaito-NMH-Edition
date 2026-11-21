#pragma once
#include "elements/plants/plants.hpp"

extern const int level_unlock_new_plant[];

class SeedPacket
{

private:
    int x, y;
    int sun_cost;
    int loading_time;

public:
    PlantType plant_name;
    int remaining_time;

    static PlantType chosen_plant;

    SeedPacket(int plant_name);
    void display(int x, int y, int player_sun_count);
    /*Empty bank slot (the SEED_PACKET_SILHOUETTE art) at the exact footprint
    of a packet slot, ICON_WIDTH x ICON_HEIGHT at (x,y): in the original the
    silhouette image is 50x70 == SEED_PACKET_WIDTH x SEED_PACKET_HEIGHT
    (repvz GameConstants.h:25-26), so an empty slot always spans the same rect
    as a seeded one. Used for the unchosen slots of the seed-chooser bank.*/
    static void display_empty(int x, int y);
    void reset_remaining_time();
    void blink(int x, int y) const;
};

/*Art box of the plant portrait inside a bank packet (relative to the
packet's top-left): clear of the sun tag whose drawn rect starts at x+58
(Sun_Tag.png's leftmost ~3 drawn px, src cols 0-7, hold <=8/97 opaque rows --
measured from resources/images/Plant_seed/Sun_Tag.png -- so the 3 px of
x58..61 never clash) and of the price text, right-aligned ending at
x + ICON_WIDTH - 4 (SeedPacket::display).*/
inline constexpr int PACKET_ICON_BOX_X = 6;
inline constexpr int PACKET_ICON_BOX_Y = 4;
inline constexpr int PACKET_ICON_BOX_W = 55;
inline constexpr int PACKET_ICON_BOX_H = 54;

/*F2 layout-editor transient packet placement, keyed by the packet object.
With no entry the const plant_seed slot rect applies verbatim (normal
gameplay); the editor writes entries while open and calls
seed_packet_layout_clear_all() on close. Scale does not live here: the draw
reads the rsdl debug-scale registry keyed by the same packet pointer.
Render-only: the bank's click/plant logic keeps using the base slot rects,
and nothing is persisted (the F9 dump is the hand-off).*/
void seed_packet_layout_set(const SeedPacket *packet, int dx, int dy);
void seed_packet_layout_get(const SeedPacket *packet, int &dx, int &dy);
void seed_packet_layout_clear(const SeedPacket *packet);
void seed_packet_layout_clear_all();

/*
Draw a plant portrait in a packet's art box, sized by the PLANT's world
height rather than fitted per icon: target height = retail idle px
(reanimplant::plant_idle_height_px, the gSkins artH column) x WORLD_SCALE x
SEED_ICON_SCALE x `k` (this packet's local scale: the bank's rsdl debug
scale, the chooser's cell scale, BELT_PACKET_W/ICON_WIDTH on the conveyor),
clamped to the avail_w x avail_h box (aspect kept), centered on cx, bottom
edge pinned at `bottom`. Small plants (puff-shroom, spikeweed) read small
next to tall ones like in the yard; oversize tall arts keep clamping into
the box as before. Temporarily lifts the portrait's alpha to full, like
SeedPacket::display always did.
*/
void draw_plant_icon_world(int plant_type, int cx, int bottom, int avail_w,
                           int avail_h, float k);

/*
Draw a plant portrait at ONE explicit scale for every plant (no world-height
sizing): the icon art's opaque bbox x `k`, clamped to the avail_w x avail_h
box (aspect kept, only triggers when the caller under-budgets k), centered on
cx, bottom edge pinned at `bottom`. Used by the almanac grid, which reads a
uniform zoom across all packets (ALMANAC_SEED_SCALE) instead of each plant's
world height.
*/
void draw_plant_icon_scaled(int plant_type, int cx, int bottom, int avail_w,
                            int avail_h, float k);