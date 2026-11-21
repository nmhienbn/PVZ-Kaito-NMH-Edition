#pragma once
#include <vector>
using namespace std;

// On-screen px width of a full (25-value) sun icon: the retail Sun.reanim
// median is ~100x99 at Coin.cpp's REANIM_SUN x1.0 (scale audit S8.4), and
// small (15-value) suns render at SUN_WIDTH / 2 (retail GetSunScale 0.5,
// Coin.cpp:1297). Shared by sun.cpp (spawn position, hitbox, wall bounce,
// draw) and the F2 layout editor's preview rect; ELEMENT_WIDTH stays for its
// other (~30 legacy sheet) callers.
const int SUN_WIDTH = 100;

/*Natural on-screen sizes (px) of the money-pickup art: the silver coin sheet
is 45x45, the coin glow sheet 99x99, and the 68x71 diamond art draws ~40x42
(retail Coin.cpp picks coin/glow images at the coin's own scale).*/
const int COIN_SILVER_SIZE = 45;
const int COIN_GLOW_SIZE = 99;
const int DIAMOND_DRAW_WIDTH = 40;
/*Seed-packet pickup (vasebreaker): the 277x174 packet art drawn 44 px tall
(~70 wide), about a hand's size on the lawn next to the sun icon's 100.*/
const int SEED_PACKET_PICKUP_SIZE = 44;

/*Money pickups (marigold coin, yeti diamond) ride the sun motion/collect
pipeline: same COIN_MOTION_FROM_PLANT arc, same click hitbox, and they still
add their `value` as sun -- this clone has no money HUD (retail CoinType is
COIN_TYPE_* in Coin.cpp). Only the artwork and the pickup chime differ.
MONEY_SEED_PACKET is the vasebreaker plant vase (retail COIN_USABLE_SEED_
PACKET, Challenge.cpp:3880): it collects onto the cursor instead of the sun
count -- SeedPacket::chosen_plant takes its plant, click-to-plant from
there. Hover-collect skips it (a click, like the original).*/
enum SunMoneyKind
{
    MONEY_NONE = 0,
    MONEY_SILVER_COIN,
    MONEY_DIAMOND,
    MONEY_SEED_PACKET
};

struct Sun
{
    int final_col, final_row;
    int x_location, y_location;
    int wait_seconds;
    int value = 25; // NORMAL_SUN_VALUE; small suns (Sun-shroom) carry 15
    // MONEY_* kind: a money pickup, not a plain sun (art + chime swap).
    int money_kind = MONEY_NONE;
    // MONEY_SEED_PACKET: the plant the packet grants when collected.
    int seed_plant = -1;
    bool is_clicked = false;
    // Tutorial 1-1: the first sun of the level never expires until collected
    // (repvz Board.cpp:2056-2060 shows [ADVICE_CLICK_ON_SUN] on the first
    // coin of first-time level 1 and the script waits for the click).
    bool persists = false;
    /*Plant-popped suns (sunflower / sun-shroom / marigold): a short ballistic
    hop out of the plant body instead of the sky sun's straight fall (repvz
    Coin.cpp COIN_MOTION_FROM_PLANT, Coin.cpp:304-310 + UpdateFall 0x430AC0).
    fx/fy keep the float position; x_location/y_location mirror it rounded.*/
    bool from_plant = false;
    float fx = 0, fy = 0;
    float vx = 0, vy = 0;
    int ground_y = 0; // landing line of the hop (plants bump free suns down)
    int pop_age = 0;  // ticks since the pop; grows the icon 40% -> 100%
    /*F2 layout-editor transient offsets (px): display_suns shifts the icon
    (scale rides the rsdl debug registry). Render-only, default 0 -> gameplay
    untouched; the editor zeroes them on close.*/
    int layout_dx = 0, layout_dy = 0;
};

void gen_random_sun_from_sky(vector<Sun> &suns);
/*The sun a sun producer pops out of its body: sunflower/sun-shroom (and the
grown/small value difference) all use COIN_MOTION_FROM_PLANT (repvz
Plant.cpp:1030-1047). money_kind != MONEY_NONE swaps it to money-pickup
art/chime (marigold's silver coin).*/
void spawn_sun_from_plant(vector<Sun> &suns, const int &row, const int &col, const int &value,
                          const int &money_kind = MONEY_NONE);
/*A coin/diamond pickup popped loose at a pixel spot (yeti loot): the same
COIN_MOTION_FROM_PLANT hop and collect pipeline, from explicit px coords.
row is the lane the click hitbox reports the drop in (col snapped from x).*/
void spawn_money_pickup(vector<Sun> &suns, const int &row, const int &x, const int &y,
                        const int &value, const int &money_kind);
/*A seed-packet pickup (vasebreaker plant vase): the retail
COIN_USABLE_SEED_PACKET coin hopping out of the broken pot
(Challenge.cpp:3880), persisting on the lawn until its click puts the
packet onto the cursor (SeedPacket::chosen_plant).*/
void spawn_seed_packet_pickup(vector<Sun> &suns, const int &row, const int &col, const int &plant_type);
void remove_suns(vector<Sun> &suns);
void move_suns(vector<Sun> &suns);
void display_suns(vector<Sun> &suns);

// Reflourished-style: sun fades out in steps as its remaining lifetime runs
// down (255 / 170 / 85). wait_seconds tracks ticks waited since landing.
int sun_alpha_for(const Sun &sun);

// Reflourished-style: hovering the cursor over a sun collects it like a
// click (starts the fly-to-sunbar motion, adds value, plays the sound).
// Uses the cursor from SDL_GetMouseState; @out collected_count, if non-null,
// receives how many suns this pass collected. try_collect_sun is the
// coordinate-explicit core (shared hitbox with pick_sun_if_clicked_on).
bool try_collect_sun(Sun &sun, const int &mx, const int &my);
void hover_collect_suns(vector<Sun> &suns, int *collected_count = nullptr);