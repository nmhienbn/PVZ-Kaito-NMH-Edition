#include "sun.hpp"
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/Player/Player.hpp"
#include "elements/zombies/zombie_struct.hpp" // CS_VEL
#include "game_param.hpp"                     // BOARD_OFFSET_X, CS_TICKS
#include "music/music.hpp"
#include "support.hpp"
#include <algorithm>
#include <cmath>

extern Map cells;
extern Window win;
extern Level level;
extern Player player;

/*
Random final row among the level's playable lanes and column (0-8) for sun from the sky
Then add it into vector<Sun>
*/
void gen_random_sun_from_sky(vector<Sun> &suns)
{
    Sun temp;
    // repvz Board::AddCoin(RandRangeInt(100,649), 60, COIN_MOTION_FROM_SKY):
    // the original x is board-relative, so shift it by BOARD_OFFSET_X. It
    // falls straight down at 0.67 px/cs and lands at y = 300 + Rand(250)
    // (Coin.cpp:315-318).
    int x = BOARD_OFFSET_X + rand(100, 649);
    temp.final_row = level.playable_rows[rand(0, (int)level.playable_rows.size() - 1)];
    temp.final_col = max(0, min(8, (x - X_UPPER_LEFT) / BLOCK_WIDTH));
    temp.fx = (float)x;
    temp.fy = 60.0f;
    temp.ground_y = 300 + rand(0, 250);
    temp.x_location = x;
    temp.y_location = 60;
    temp.wait_seconds = 0;
    // Tutorial 1-1: the first sky sun waits for the player's click forever.
    temp.persists = (level.level_num == 1 && level.sky_sun_fallen == 0);
    suns.push_back(temp);
}

/*
Natural on-screen size (px) of the icon a sun/money pickup starts from: full
sun 100, small sun 50 (retail GetSunScale), money pickups their art's own
size (no stretching to sun size).
*/
static int sun_art_size(const int &value, const int &money_kind)
{
    if (money_kind == MONEY_SILVER_COIN)
        return COIN_SILVER_SIZE;
    if (money_kind == MONEY_DIAMOND)
        return DIAMOND_DRAW_WIDTH;
    if (money_kind == MONEY_SEED_PACKET)
        return SEED_PACKET_PICKUP_SIZE;
    return value >= NORMAL_SUN_VALUE ? SUN_WIDTH : SUN_WIDTH / 2;
}

/*
Push one fresh pickup hopping out of a spot: shared COIN_MOTION_FROM_PLANT
ballistics for plant pops (spawn_sun_from_plant) and loose drops
(spawn_money_pickup).
*/
static void push_popped_sun(vector<Sun> &suns, const int &row, const int &col,
                            const float &fx, const float &fy,
                            const int &value, const int &money_kind)
{
    Sun temp;
    temp.final_row = row;
    temp.final_col = col;
    temp.value = value;
    temp.money_kind = money_kind;
    temp.from_plant = true;
    temp.fx = fx;
    temp.fy = fy;
    // repvz COIN_MOTION_FROM_PLANT: velY = -1.7 - RandRangeFloat(0,1.7),
    // velX = -0.4 + RandRangeFloat(0,0.8) px/cs (Coin.cpp:327-331).
    temp.vx = CS_VEL(-0.4f + rand(0, 80) / 100.0f);
    temp.vy = CS_VEL(-1.7f - rand(0, 170) / 100.0f);
    temp.ground_y = (int)temp.fy + 15 + rand(0, 19); // +15 + Rand(20)
    temp.x_location = (int)temp.fx;
    temp.y_location = (int)temp.fy;
    temp.wait_seconds = 0;
    suns.push_back(temp);
}

/*
A sun popped by a plant hops out of the plant body on a short arc and settles
beside it, at 40% scale growing to full size while it flies (repvz Coin.cpp
COIN_MOTION_FROM_PLANT: vy0 -1.7..-3.4, vx -0.4..0.4 px/cs, gravity 0.09
px/cs, ground at +15..+35 px; 60 tps conversions: velocities *5/3, accel
*(5/3)^2).
*/
void spawn_sun_from_plant(vector<Sun> &suns, const int &row, const int &col, const int &value,
                          const int &money_kind)
{
    int sun_w = sun_art_size(value, money_kind);
    float fx = (cells[row][col].x1 + cells[row][col].x2 - sun_w) / 2.0f + rand(-20, 20);
    float fy = (cells[row][col].y1 + roof_tile_dy(col)) + 10.0f;
    push_popped_sun(suns, row, col, fx, fy, value, money_kind);
}

void spawn_money_pickup(vector<Sun> &suns, const int &row, const int &x, const int &y,
                        const int &value, const int &money_kind)
{
    // The click hitbox looks cells[final_row][final_col] up: snap the drop's
    // column from x, clamped onto the grid (whack-a-zombie sun drop idiom).
    int col = get_block_col(x);
    if (col < 0)
        col = 0;
    if (col > 8)
        col = 8;
    push_popped_sun(suns, row, col, (float)x, (float)y, value, money_kind);
}

/*
A seed-packet pickup (vasebreaker plant vase) hopping out of the broken pot:
the retail COIN_USABLE_SEED_PACKET coin (Challenge.cpp:3879-3881 drops it at
the pot with COIN_MOTION_FROM_PLANT), here on the shared sun hop arc. It
never expires -- the packet waits for its click, which puts it onto the
cursor (pick_sun_if_clicked_on -> SeedPacket::chosen_plant).
*/
void spawn_seed_packet_pickup(vector<Sun> &suns, const int &row, const int &col, const int &plant_type)
{
    Sun temp;
    temp.final_row = row;
    temp.final_col = col;
    temp.value = 0;
    temp.money_kind = MONEY_SEED_PACKET;
    temp.seed_plant = plant_type;
    temp.persists = true; // no expiry: waits for the click like a retail coin
    temp.from_plant = true;
    temp.fx = cells[row][col].x1 + 20.0f;
    temp.fy = cells[row][col].y1 + 10.0f + roof_tile_dy(col);
    temp.vx = CS_VEL(-0.4f + rand(0, 80) / 100.0f);
    temp.vy = CS_VEL(-1.7f - rand(0, 170) / 100.0f);
    temp.ground_y = (int)temp.fy + 15 + rand(0, 19);
    temp.x_location = (int)temp.fx;
    temp.y_location = (int)temp.fy;
    temp.wait_seconds = 0;
    suns.push_back(temp);
}

/*
Remove the suns that are exist a long time. Tutorial-persistent suns (1-1's
first sun) stay until collected.
*/
void remove_suns(vector<Sun> &suns)
{
    for (int i = 0; i < (int)suns.size(); i++)
        if (!suns[i].persists && suns[i].wait_seconds >= SUN_APPEAR_LIMIT)
        {
            suns.erase(suns.begin() + i);
            i--;
        }
}

/*
Hitbox test shared by click (pick_sun_if_clicked_on) and hover collection.
*/
static bool cursor_on_sun(const Sun &sun, const int &mx, const int &my)
{
    // Sky suns spawn at an arbitrary board x and plant-popped suns drift off
    // their tile on the hop: both hitboxes ride the real x_location.
    int x1 = sun.x_location;
    int x2 = sun.x_location + SUN_WIDTH;
    return (mx > x1 && mx < x2 &&
            my > sun.y_location && my < sun.y_location + ELEMENT_HEIGHT);
}

/*
Collect a sun under (mx, my) exactly like pick_sun_if_clicked_on does for a
click: flag it, snap x to the cell border, add its value, play the sound.
@return 'true' when the sun was collected by this call.
*/
bool try_collect_sun(Sun &sun, const int &mx, const int &my)
{
    // Seed packets are click-only (a coin in the original; the clone's hover
    // nicety must not put packets onto an unattended cursor).
    if (sun.money_kind == MONEY_SEED_PACKET)
        return false;
    if (sun.is_clicked || !cursor_on_sun(sun, mx, my))
        return false;
    sun.is_clicked = true;
    sun.fx = (float)sun.x_location;
    sun.fy = (float)sun.y_location;
    player.sun_count += sun.value;
    // Money pickups chime with the coin sound; a plain sun keeps its own.
    play_sound_effect(sun.money_kind == MONEY_NONE ? SUN_CLICK_MUSIC_DIRECTORY : COIN_MUSIC_DIRECTORY);
    return true;
}

/*
Reflourished-style: the cursor sliding over a not-yet-collected sun collects
it exactly like a click -- is_clicked + fly to the sunbar, add value, and
play the sound.
*/
void hover_collect_suns(vector<Sun> &suns, int *collected_count)
{
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    if (collected_count)
        *collected_count = 0;
    for (auto &sun : suns)
        if (try_collect_sun(sun, mx, my) && collected_count)
            (*collected_count)++;
}

/*
For all the sun:
    Hover over the sun collects it (Reflourished-style).
    If sun is clicked: move it to top left corner. Then remove it.
    Else
        If the sun has not reached the ground: move it.
        Else increase sun-wait time.
*/
void move_suns(vector<Sun> &suns)
{
    hover_collect_suns(suns);
    for (int i = 0; i < (int)suns.size(); i++)
    {
        if (suns[i].is_clicked)
        {
            // repvz Coin::UpdateCollected (Coin.cpp:667-684): every original
            // centisecond tick closes 1/21 of the remaining distance toward the
            // sun bank (15, 0). At 60 tps the per-tick fraction is
            // 1 - (1 - 1/21)^(100/60), so the wall-clock flight is unchanged.
            const float k = 1.0f - powf(1.0f - 1.0f / 21.0f, 100.0f / 60.0f);
            suns[i].fx += (15.0f - suns[i].fx) * k;
            suns[i].fy += (0.0f - suns[i].fy) * k;
            suns[i].x_location = (int)lroundf(suns[i].fx);
            suns[i].y_location = (int)lroundf(suns[i].fy);
            // Retail scores/removes the coin within ~8 px of the bank.
            if (hypotf(suns[i].fx - 15.0f, suns[i].fy) < 8.0f)
            {
                suns.erase(suns.begin() + i);
                i--;
            }
            continue;
        }
        // If the sun has not reached the ground: move it.
        // Else increase sun-wait time.
        if (suns[i].from_plant)
        {
            // COIN_MOTION_FROM_PLANT arc (repvz Coin::UpdateFall): gravity
            // 0.09 px/cs^2 -> 0.25 px/tick^2; land on the hop's ground line.
            Sun &s = suns[i];
            s.pop_age++;
            if (s.fy + s.vy < s.ground_y)
            {
                s.fy += s.vy;
                s.vy += 0.09f * (100.0f / 60.0f) * (100.0f / 60.0f);
                s.fx += s.vx;
                if (s.fx < 0)
                {
                    s.fx = 0;
                    s.vx = -s.vx;
                }
                if (s.fx > WINDOW_WIDTH - SUN_WIDTH)
                {
                    s.fx = WINDOW_WIDTH - SUN_WIDTH;
                    s.vx = -s.vx;
                }
                s.x_location = (int)s.fx;
                s.y_location = (int)s.fy;
            }
            else
            {
                s.y_location = s.ground_y;
                s.wait_seconds++;
            }
            continue;
        }
        // repvz COIN_MOTION_FROM_SKY (Coin.cpp:315-318): constant 0.67 px/cs
        // fall (not gravity), landing on the board-fixed ground line
        // 300 + Rand(250) set at spawn.
        Sun &s = suns[i];
        if (s.fy + CS_VEL(0.67f) < (float)s.ground_y)
        {
            s.fy += CS_VEL(0.67f);
            s.y_location = (int)lroundf(s.fy);
        }
        else
        {
            s.y_location = s.ground_y;
            s.wait_seconds++;
        }
    }
}

/*
Reflourished-style stepped fade with age: full opacity while more than 2/3
of the lifetime remains, ~2/3 alpha in the middle third, ~1/3 in the last.
(The original PvZ1 sun keeps alpha 255 and simply dies when its disappear
time is up; this is a requested departure from the original.)
*/
int sun_alpha_for(const Sun &sun)
{
    if (sun.persists)
        return 255; // tutorial sun: no fade while it waits for its click
    int remaining = SUN_APPEAR_LIMIT - sun.wait_seconds;
    if (remaining * 3 > 2 * SUN_APPEAR_LIMIT)
        return 255;
    if (remaining * 3 > SUN_APPEAR_LIMIT)
        return 170;
    return 85;
}

/*
Set suns' transparency by their age (stepped fade), then display them.
*/
void display_suns(vector<Sun> &suns)
{
    for (auto &sun : suns)
    {
        // Money pickups draw their retail art instead of the sun sprite.
        int dir = SUN_DIRECTORY;
        if (sun.money_kind == MONEY_SILVER_COIN)
            dir = COIN_SILVER_DOLLAR_DIRECTORY;
        else if (sun.money_kind == MONEY_DIAMOND)
            dir = DIAMOND_DIRECTORY;
        else if (sun.money_kind == MONEY_SEED_PACKET)
            dir = SEED_PACKET_DIRECTORY;
        if (!sun.is_clicked)
        {
            // x_location rides the sun's own motion (sky fall / plant hop).
            win.set_texture_alpha(dir, sun_alpha_for(sun));
        }
        else
        {
            win.set_texture_alpha(dir, 255);
        }
        // Small suns (Sun-shroom in its grown-up drop stage) render smaller
        // (retail GetSunScale 0.5: 100 -> 50 px); money pickups keep their
        // art's natural size (silver coin 45, diamond 40x42).
        int sun_h = sun_art_size(sun.value, sun.money_kind);
        // Pop grow-in: plant suns pop out at 40% and reach full size over the
        // hop (repvz Coin.cpp:588-597: mScale 0.4 +0.02/cs -> 1.0 in ~30 cs).
        if (sun.from_plant && sun.pop_age < 18)
            sun_h = sun_h * (40 + (60 * sun.pop_age) / 18) / 100;
        // F2 layout-editor adjustments (transient): offsets shift the icon;
        // the debug scale (1.0 in gameplay) grows it about its center.
        const float dbg = rsdl_debug_scale(&sun);
        const int sw = (int)lroundf(sun_h * dbg * WORLD_SCALE);
        const int dx = sun.x_location + sun.layout_dx + (sun_h - sw) / 2;
        const int dy = sun.y_location + sun.layout_dy + (sun_h - sw) / 2;
        // Soft glow behind an uncollected coin (retail CoinGlow, static is
        // fine; the original pulses it).
        if (sun.money_kind == MONEY_SILVER_COIN && !sun.is_clicked)
        {
            const int glow_h = sw * COIN_GLOW_SIZE / COIN_SILVER_SIZE;
            win.set_texture_alpha(COIN_GLOW_DIRECTORY, sun_alpha_for(sun));
            win.draw_png_height_scaled(COIN_GLOW_DIRECTORY,
                                       dx + (sw - glow_h) / 2, dy + (sw - glow_h) / 2, glow_h);
        }
        win.draw_png_height_scaled(dir, dx, dy, sw);
    }
}