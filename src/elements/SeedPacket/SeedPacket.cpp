#include "SeedPacket.hpp"
#include "draw/plant_icon_dir.hpp"
#include "draw/world_scale.hpp"
#include "elements/button/button.hpp"
#include "reanim/reanim_plant.hpp"

#include <SDL_image.h>
#include <algorithm>
#include <cmath>
#include <map>

extern Window win;
// Level num at which each plant seed becomes available (enum PlantType
// order): peashooter@1-1, sunflower@1-2, wallnut@1-4, snowpea@1-7,
// potatomine@1-6, cherrybomb@1-3, repeater@1-9, chomper@1-8, puffshroom@2-1,
// sunshroom@2-2, fumeshroom@2-3, gravebuster@2-4, hypnoshroom@2-6 (whack
// reward), scaredyshroom@2-7, iceshroom@2-8, doomshroom@2-9.
// (GOTY rewards: after 1-5 potato mine, 1-6 snow pea, 1-7 chomper,
// 1-8 repeater, 1-10 puff-shroom; 2-1 sun-shroom, 2-2 fume-shroom,
// 2-3 grave buster, 2-5 hypno-shroom, 2-6 scaredy-shroom, 2-7 ice-shroom,
// 2-8 doom-shroom.)
// Area 3-5 (GOTY rewards): lilypad@3-1, squash@3-2, threepeater@3-3,
// tanglekelp@3-4 (3-5 gives the car keys instead), jalapeno@3-6,
// spikeweed@3-7, torchwood@3-8, tallnut@3-9; seashroom@4-1, plantern@4-2,
// cactus@4-3, blover@4-4 (4-5 gives the taco), splitpea@4-6, starfruit@4-7,
// pumpkin@4-8, magnetshroom@4-9; cabbagepult@5-1, flowerpot@5-2,
// kernelpult@5-3, coffeebean@5-4 (5-5 gives the watering can), garlic@5-6,
// umbrellaleaf@5-7, marigold@5-8, melonpult@5-9.
const int level_unlock_new_plant[] = {1, 2, 4, 7, 6, 3, 9, 8, 11, 12, 13, 14, 16, 17, 18, 19,
                                      21, 22, 23, 24, 26, 27, 28, 29,
                                      31, 32, 33, 34, 36, 37, 38, 39,
                                      41, 42, 43, 44, 46, 47, 48, 49};

PlantType SeedPacket::chosen_plant = PLANT_COUNT;

namespace
{
// F2 layout-editor transient offsets, keyed by the live packet object (see
// the header). Empty registry in normal gameplay.
struct PacketLayout
{
    int dx = 0, dy = 0;
};
std::map<const SeedPacket *, PacketLayout> g_packet_layouts;
} // namespace

void seed_packet_layout_set(const SeedPacket *packet, int dx, int dy)
{
    if (packet == NULL)
        return;
    if (dx == 0 && dy == 0)
        g_packet_layouts.erase(packet);
    else
        g_packet_layouts[packet] = {dx, dy};
}

void seed_packet_layout_get(const SeedPacket *packet, int &dx, int &dy)
{
    auto it = g_packet_layouts.find(packet);
    if (it == g_packet_layouts.end())
    {
        dx = dy = 0;
        return;
    }
    dx = it->second.dx;
    dy = it->second.dy;
}

void seed_packet_layout_clear(const SeedPacket *packet)
{
    g_packet_layouts.erase(packet);
}

void seed_packet_layout_clear_all()
{
    g_packet_layouts.clear();
}

namespace
{
/*Opaque bbox (alpha > 0) of a portrait image, measured once from the PNG on
disk and cached per directory. (0,0,w,h) full-frame fallback on load/style
failure keeps callers on the old whole-image behavior.*/
SDL_Rect opaque_bbox(int file_num)
{
    static std::map<int, SDL_Rect> cache;
    auto it = cache.find(file_num);
    if (it != cache.end())
        return it->second;
    SDL_Rect box{0, 0, 0, 0};
    SDL_Surface *surf = IMG_Load(all_img[file_num].img_dir.c_str());
    if (surf)
    {
        SDL_Surface *rgba = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(surf);
        if (rgba)
        {
            int min_x = rgba->w, min_y = rgba->h, max_x = -1, max_y = -1;
            const Uint32 *pixels = (const Uint32 *)rgba->pixels;
            int stride = rgba->pitch / sizeof(Uint32);
            for (int y = 0; y < rgba->h; y++)
                for (int x = 0; x < rgba->w; x++)
                    if ((pixels[y * stride + x] >> 24) != 0) // RGBA32: alpha in top byte
                    {
                        min_x = std::min(min_x, x);
                        max_x = std::max(max_x, x);
                        min_y = std::min(min_y, y);
                        max_y = std::max(max_y, y);
                    }
            if (max_x >= min_x && max_y >= min_y)
                box = SDL_Rect{min_x, min_y, max_x - min_x + 1, max_y - min_y + 1};
            SDL_FreeSurface(rgba);
        }
    }
    return cache[file_num] = box;
}
} // namespace

void draw_plant_icon_scaled(int plant_type, int cx, int bottom, int avail_w,
                            int avail_h, float k)
{
    if (plant_type < 0 || plant_type >= PLANT_COUNT)
        return;
    int plant_dir = PLANT_ICON_DIR[plant_type];
    GlTexture *art = win.load_texture(plant_dir);
    if (!art || art->id == 0 || art->w <= 0 || art->h <= 0)
        return;
    SDL_Rect src = opaque_bbox(plant_dir);
    if (src.w <= 0 || src.h <= 0)
        src = SDL_Rect{0, 0, art->w, art->h};
    float fit = k;
    float max_fit = std::min(avail_w / (float)src.w, avail_h / (float)src.h);
    if (fit > max_fit)
        fit = max_fit;
    int dw = std::max(1, (int)(src.w * fit + 0.5f));
    int dh = std::max(1, (int)(src.h * fit + 0.5f));
    int old_alpha = all_img[plant_dir].alpha;
    win.set_texture_alpha(plant_dir, 255);
    // bottom edge pinned at `bottom`: grounds sit level across packets
    win.draw_png(plant_dir, src.x, src.y, src.w, src.h,
                 cx - dw / 2, bottom - dh + 1, dw, dh);
    win.set_texture_alpha(plant_dir, old_alpha);
}

void draw_plant_icon_world(int plant_type, int cx, int bottom, int avail_w,
                           int avail_h, float k)
{
    if (plant_type < 0 || plant_type >= PLANT_COUNT)
        return;
    int plant_dir = PLANT_ICON_DIR[plant_type];
    GlTexture *art = win.load_texture(plant_dir);
    if (!art || art->id == 0 || art->w <= 0 || art->h <= 0)
        return;
    SDL_Rect src = opaque_bbox(plant_dir);
    if (src.w <= 0 || src.h <= 0)
        src = SDL_Rect{0, 0, art->w, art->h};
    // World-height target, clamped into the box exactly like the old fit.
    float target_h = reanimplant::plant_idle_height_px(plant_type) *
                     WORLD_SCALE * SEED_ICON_SCALE * k;
    float fit = target_h / src.h;
    float max_fit = std::min(avail_w / (float)src.w, avail_h / (float)src.h);
    if (fit > max_fit)
        fit = max_fit;
    int dw = std::max(1, (int)(src.w * fit + 0.5f));
    int dh = std::max(1, (int)(src.h * fit + 0.5f));
    int old_alpha = all_img[plant_dir].alpha;
    win.set_texture_alpha(plant_dir, 255);
    // bottom edge pinned at `bottom`: grounds sit level across packets
    win.draw_png(plant_dir, src.x, src.y, src.w, src.h,
                 cx - dw / 2, bottom - dh + 1, dw, dh);
    win.set_texture_alpha(plant_dir, old_alpha);
}

SeedPacket::SeedPacket(int plant_name)
{
    try
    {
        this->plant_name = static_cast<PlantType>(plant_name);
    }
    catch (const std::exception &e)
    {
        throw invalid_argument("Invalid plant name in function SeedPacket::SeedPacket(int plant_name)");
    }
    this->loading_time = PLANT_LOADING_TIME[plant_name];
    this->remaining_time = 0;
    sun_cost = PLANT_SUN_COST[plant_name];
}

void SeedPacket::reset_remaining_time()
{
    remaining_time = loading_time;
}

void SeedPacket::display(int x, int y, int player_sun_count)
{
    // F2 layout-editor transient placement: offsets come from the packet
    // registry, the scale from the rsdl debug registry (about the slot
    // center). Identity in normal gameplay -> the original coordinates.
    int layout_dx, layout_dy;
    seed_packet_layout_get(this, layout_dx, layout_dy);
    const float s = rsdl_debug_scale(this);
    // X/Y map slot-local coords through scale-about-center + offset; W/H size.
    const float cx = x + ICON_WIDTH / 2.0f + layout_dx;
    const float cy = y + ICON_HEIGHT / 2.0f + layout_dy;
    const auto X = [&](int v) { return (int)lroundf(cx + (v - ICON_WIDTH / 2.0f) * s); };
    const auto Y = [&](int v) { return (int)lroundf(cy + (v - ICON_HEIGHT / 2.0f) * s); };
    const auto W = [&](int v) { return (int)lroundf(v * s); };
    // Seed packet
    win.draw_png_height_scaled(SEED_PACKET_DIRECTORY, X(0), Y(0), W(ICON_WIDTH));

    // Plant on seed packet: world-height sizing (draw_plant_icon_world) into
    // the common art box -- a tall-nut towers, a puff-shroom stays small,
    // oversize arts clamp into the box as the old box-fit did.
    draw_plant_icon_world(plant_name,
                          X(PACKET_ICON_BOX_X) + W(PACKET_ICON_BOX_W) / 2,
                          Y(PACKET_ICON_BOX_Y) + W(PACKET_ICON_BOX_H),
                          W(PACKET_ICON_BOX_W), W(PACKET_ICON_BOX_H), s);

    // Sun tag
    win.draw_png_height_scaled(SUN_TAG_DIRECTORY, X(58), Y(25), W(41));
    win.draw_png_height_scaled(SEED_PACKET_BOT_BORDER_DIRECTORY, X(0), Y(60), W(ICON_WIDTH));

    // Not enough sun or is chosen
    if (player_sun_count < sun_cost || SeedPacket::chosen_plant == plant_name || remaining_time)
    {
        win.draw_png(BLACK_SCREEN_DIRECTORY, X(0), Y(0), W(ICON_WIDTH), W(ICON_HEIGHT));
    }

    if (remaining_time == 0)
    {
        if (plant_name == chosen_plant)
            win.draw_png_height_scaled(SEED_CHOSEN_DIRECTORY, X(0), Y(0), W(ICON_WIDTH));
    }
    else
    {
        // Remaining time
        win.draw_png(BLACK_SCREEN_DIRECTORY, X(0), Y(0), W(ICON_WIDTH),
                     W(remaining_time * ICON_HEIGHT / loading_time));
    }

    int w, h;
    int fsize = std::max(8, (int)lroundf(30 * s));
    TTF_SizeText(win.get_font(PVZUI_TTF, fsize), to_string(sun_cost).c_str(), &w, &h);
    w = X(ICON_WIDTH) - W(4) - w;
    h = Y(ICON_HEIGHT) - h;
    win.show_text(to_string(sun_cost),
                  w + W(2), h + W(3), BLACK, PVZUI_TTF, fsize);
    win.show_text_outlined(to_string(sun_cost),
                           w, h, WHITE, PVZUI_TTF, fsize);
}

void SeedPacket::display_empty(int x, int y)
{
    // Same footprint as display(): the packet art is drawn ICON_WIDTH wide,
    // height-scaled to ICON_HEIGHT (277x174 -> 105x65); the silhouette fills
    // the slot edge to edge so empty slots read as wide as seeded ones.
    win.draw_png(SEED_PACKET_SILHOUETTE_DIRECTORY, x, y, ICON_WIDTH, ICON_HEIGHT);
}

void SeedPacket::blink(int x, int y) const
{
    // Hover test + flash on the packet's actual drawn rect (F2 layout-editor
    // offsets / debug scale included, see display()).
    int layout_dx, layout_dy;
    seed_packet_layout_get(this, layout_dx, layout_dy);
    const float s = rsdl_debug_scale(this);
    const int w = (int)lroundf(ICON_WIDTH * s);
    const int h = (int)lroundf(ICON_HEIGHT * s);
    const int rx = x + layout_dx + (ICON_WIDTH - w) / 2;
    const int ry = y + layout_dy + (ICON_HEIGHT - h) / 2;
    int _x, _y;
    SDL_GetMouseState(&_x, &_y);
    if (is_in(rx, _x, rx + w) && is_in(ry, _y, ry + h))
    {
        win.draw_png(WHITE_SCREEN_DIRECTORY, rx, ry, w, h);
    }
}