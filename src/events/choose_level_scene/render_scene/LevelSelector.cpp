#include "LevelSelector.hpp"
#include "draw/plant_icon_dir.hpp"
#include "level/debut_data.hpp"
#include "level/level_data.hpp"
#include "level/plants_registry.hpp"
#include <SDL_image.h>
#include <algorithm>
#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
using namespace nlohmann;

extern Window win;

static const int selector_width = 139;

/*Selector thumbnails by ZombieType. Only a few zombie thumbnail arts ship
with the pack: STUB-AREA-AGENT zombies reuse the closest existing thumbnail
until the area agents ship real icons (water variants -> normal, athletes
and riders -> football, oddball poses -> dancer), and any type missing here
falls back to the plain zombie icon.*/
static const map<int, int> zombie_selector_icons = {
    {NORMAL_TYPE, NORMAL_DIRECTORY},
    {FLAG_TYPE, FLAG_DIRECTORY},
    {CONE_TYPE, CONE_DIRECTORY},
    {BUCKET_TYPE, BUCKET_DIRECTORY},
    {DOOR_TYPE, DOOR_DIRECTORY},
    {NEWSPAPER_TYPE, NEWSPAPER_SELECTOR_DIRECTORY},
    {FOOTBALL_TYPE, FOOTBALL_SELECTOR_DIRECTORY},
    {DANCER_TYPE, DANCER_SELECTOR_DIRECTORY},
    {POLEVAULT_TYPE, FOOTBALL_SELECTOR_DIRECTORY},   // athlete
    {DUCKYTUBE_TYPE, NORMAL_DIRECTORY},              // basic + pool ring
    {SNORKEL_TYPE, NORMAL_DIRECTORY},                // basic in swim gear
    {ZOMBONI_TYPE, FOOTBALL_SELECTOR_DIRECTORY},
    {BOBSLED_TYPE, FOOTBALL_SELECTOR_DIRECTORY},
    {DOLPHINRIDER_TYPE, FOOTBALL_SELECTOR_DIRECTORY}, // rider
    {JACKBOX_TYPE, NEWSPAPER_SELECTOR_DIRECTORY},
    {BALLOON_TYPE, DANCER_SELECTOR_DIRECTORY},       // airborne oddball
    {DIGGER_TYPE, DOOR_DIRECTORY},
    {POGO_TYPE, FOOTBALL_SELECTOR_DIRECTORY},        // hunched bouncer
    {BUNGEE_TYPE, DANCER_SELECTOR_DIRECTORY},        // posed oddball
    {LADDER_TYPE, DOOR_DIRECTORY},
    {CATAPULT_TYPE, FOOTBALL_SELECTOR_DIRECTORY},
    {GARGANTUAR_TYPE, FOOTBALL_SELECTOR_DIRECTORY},
    {IMP_TYPE, NORMAL_DIRECTORY},                    // small basic
    {ZOMBOSS_TYPE, FOOTBALL_SELECTOR_DIRECTORY}};

// Plant reward icons come from the single icon registry
// (draw/plant_icon_dir.hpp) instead of computing PEASHOOTER_DIRECTORY + type.
// The assert pins the legacy contiguous layout as a regression tripwire only.
static_assert(MELONPULT_DIRECTORY - PEASHOOTER_DIRECTORY == PLANT_COUNT - 1,
              "plant icon dirs must stay contiguous in PlantType order");

// --- Preview fitting (shared by every selector preview) ---------------------
// Card-local cells: the level preview (backdrop crop + debut zombies) is
// drawn at (5, 10), 128x96 (4:3 -- backdrop_src_rect crops a 4:3 box so it
// lands undistorted), the reward-plant box at (45, 110), 50x50
// (container.png art is square).
const int THUMB_CELL_X = 5, THUMB_CELL_Y = 10, THUMB_CELL_W = 128, THUMB_CELL_H = 96;
const int PLANT_CELL_X = 45, PLANT_CELL_Y = 110, PLANT_CELL_W = 50, PLANT_CELL_H = 50;
const int PREVIEW_SIDE_INSET = 2;  // icon keeps 2 px off the cell side edges
const int PREVIEW_BOTTOM_LIFT = 6; // icon sits this far above the cell bottom
const float ZOMBIE_PREVIEW_H = 70; // legacy tuned full-frame heights; fitting
const float PLANT_PREVIEW_H = 50;  // may only shrink below these, never grow

namespace
{
struct PreviewContent
{
    SDL_Rect content{0, 0, 0, 0}; // opaque-pixel bbox inside the art
    int w = 0, h = 0;             // full art size
};

/*Opaque-pixel bbox + full size of an icon art, cached. Icons carry uneven
transparent padding, so fitting against the visible pixels keeps every
preview centered and at the same visual weight (seed packets/almanac do the
same). Missing art yields a zero content -> the icon is skipped.*/
const PreviewContent &preview_content(int dir)
{
    static map<int, PreviewContent> cache;
    auto it = cache.find(dir);
    if (it != cache.end())
        return it->second;
    PreviewContent pc;
    if (SDL_Surface *surf = IMG_Load(all_img[dir].img_dir.c_str()))
    {
        pc.w = surf->w;
        pc.h = surf->h;
        if (SDL_Surface *rgba = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0))
        {
            int min_x = rgba->w, min_y = rgba->h, max_x = -1, max_y = -1;
            const Uint32 *pixels = (const Uint32 *)rgba->pixels;
            const int stride = rgba->pitch / sizeof(Uint32);
            for (int y = 0; y < rgba->h; y++)
                for (int x = 0; x < rgba->w; x++)
                    if ((pixels[y * stride + x] >> 24) != 0) // RGBA32: alpha in top byte
                    {
                        min_x = min(min_x, x);
                        min_y = min(min_y, y);
                        max_x = max(max_x, x);
                        max_y = max(max_y, y);
                    }
            if (max_x >= min_x && max_y >= min_y)
                pc.content = SDL_Rect{min_x, min_y, max_x - min_x + 1, max_y - min_y + 1};
            SDL_FreeSurface(rgba);
        }
        SDL_FreeSurface(surf);
    }
    if (pc.content.w <= 0 || pc.content.h <= 0)
        pc.content = SDL_Rect{0, 0, pc.w, pc.h};
    return cache.emplace(dir, pc).first->second;
}

/*The one fit rule every selector preview obeys: content scaled to the cell
interior -- side inset on the left/right, bottom_lift off the bottom --
aspect kept, capped at the legacy tuned height (target_h on the full frame)
and at the art's natural size (never upscaled). The min() clamps on dst w/h
keep the returned rect inside the cell even under rounding, so containment is
guaranteed by construction, not by clipping.*/
SDL_Rect fit_preview_in_cell(int dir, const SDL_Rect &cell, float target_h, int bottom_lift)
{
    SDL_Rect dst{cell.x, cell.y + cell.h - bottom_lift, 0, 0};
    const PreviewContent &pc = preview_content(dir);
    if (pc.content.w <= 0 || pc.content.h <= 0 || pc.h <= 0)
        return dst;
    float scale = (cell.w - 2 * PREVIEW_SIDE_INSET) / (float)pc.content.w;
    scale = min(scale, (cell.h - bottom_lift) / (float)pc.content.h);
    scale = min(scale, target_h / pc.h);
    scale = min(scale, 1.0f);
    dst.w = max(1, min(cell.w - 2 * PREVIEW_SIDE_INSET, (int)(pc.content.w * scale + 0.5f)));
    dst.h = max(1, min(cell.h - bottom_lift, (int)(pc.content.h * scale + 0.5f)));
    dst.x = cell.x + (cell.w - dst.w) / 2;
    dst.y = cell.y + cell.h - bottom_lift - dst.h;
    return dst;
}

// Draw only the opaque content at the fitted rect (the transparent padding
// never participates in placement).
void draw_preview_icon(int dir, const SDL_Rect &dst)
{
    if (dst.w <= 0 || dst.h <= 0)
        return;
    const PreviewContent &pc = preview_content(dir);
    win.draw_png(dir, pc.content.x, pc.content.y, pc.content.w, pc.content.h,
                 dst.x, dst.y, dst.w, dst.h);
}
} // namespace

SDL_Rect LevelSelector::zombie_cell(int x, int y)
{
    return SDL_Rect{x + THUMB_CELL_X, y + THUMB_CELL_Y, THUMB_CELL_W, THUMB_CELL_H};
}

SDL_Rect LevelSelector::plant_cell(int x, int y)
{
    return SDL_Rect{x + PLANT_CELL_X, y + PLANT_CELL_Y, PLANT_CELL_W, PLANT_CELL_H};
}

/*Backdrop art size, cached (the arts are the plain Background .png boards,
all opaque). Missing art yields {0, 0} and the crop is skipped.*/
SDL_Point LevelSelector::backdrop_art_size(int dir)
{
    static map<int, SDL_Point> cache;
    auto it = cache.find(dir);
    if (it != cache.end())
        return it->second;
    SDL_Point size{0, 0};
    if (SDL_Surface *surf = IMG_Load(all_img[dir].img_dir.c_str()))
    {
        size.x = surf->w;
        size.y = surf->h;
        SDL_FreeSurface(surf);
    }
    return cache.emplace(dir, size).first->second;
}

/*Cell convention: a 4:3 crop of the level's backdrop (exactly the cell's
aspect, undistorted), positioned per backdrop class (bg index = dir -
BACKGROUND_LV1_DIRECTORY, level_data.cpp background_map), windows verified
against the shipped 1400x600 arts and pinned by selector_preview_check:

  yards (0-3): zoomed-out 30% window flush with the art's right edge and
      top: left ~20% slice of lawn (x ~880-985; dirt lane on idx 0), then
      the sidewalk, and the street still dominating the right half. The
      window shows the leftmost 1050 px of the art, so the art's right edge
      is exactly the offscreen approach zombies walk in from.
  pool (4): half-height window centered on the water rectangle measured on
      the art (x 280..990, y 255..445 -> center 635,350): lawn strip above,
      the water band, lawn strip below.
  fog (5): same size, dead-centered on the art -- the water band still
      crosses its middle.
  roof (6-7): zoomed-out 30% window flush with the art's BOTTOM-right
      corner. The roof has no street up there; the bottom-right is the deck
      edge the zombies arrive over (roof cards: bungee, ladder, catapult,
      gargantuar, zomboss) -- do not move that corner (all tiles above the
      gutter, no sky). */
SDL_Rect LevelSelector::backdrop_src_rect(int dir)
{
    const SDL_Point art = backdrop_art_size(dir);
    if (art.x <= 0 || art.y <= 0)
        return SDL_Rect{0, 0, 0, 0};
    const int idx = dir - BACKGROUND_LV1_DIRECTORY;
    int sh = art.y / 2;      // 300 of 600: legacy size, kept for pool/fog
    if (idx <= 3 || idx >= 6)
        sh = art.y * 13 / 20; // 390 of 600: yards/roofs zoomed out ~30%
    int sw = sh * THUMB_CELL_W / THUMB_CELL_H;
    if (sw > art.x) // narrower-than-4:3 art: pin the crop to the art instead
    {
        sw = art.x;
        sh = sw * THUMB_CELL_H / THUMB_CELL_W;
    }
    if (idx == 4) // pool: center the window on the water rect's center 635,350
        return SDL_Rect{635 - sw / 2, 350 - sh / 2, sw, sh};
    if (idx == 5) // fog: dead-centered on the art
        return SDL_Rect{(art.x - sw) / 2, (art.y - sh) / 2, sw, sh};
    if (idx >= 6) // roof: flush bottom-right deck edge
        return SDL_Rect{art.x - sw, art.y - sh, sw, sh};
    // yards: flush right edge and top, the zoomed approach strip
    return SDL_Rect{art.x - sw, 0, sw, sh};
}

/*Fitted rect of the index-th debut icon of count, inside the card at (x, y):
the cell width is dealt into equal side-by-side slots (the group then spans
the cell symmetrically), each icon fitted inside its own slot, so N debuts
still cannot overflow the cell.*/
SDL_Rect LevelSelector::debut_preview_rect(int dir, int index, int count, int x, int y)
{
    const SDL_Rect cell = zombie_cell(x, y);
    if (index < 0 || index >= count) // no slot: empty fit
        return SDL_Rect{cell.x, cell.y, 0, 0};
    const int slot_w = cell.w / count;
    const SDL_Rect slot{cell.x + index * slot_w, cell.y,
                        index == count - 1 ? cell.w - index * slot_w : slot_w,
                        cell.h};
    return fit_preview_in_cell(dir, slot, ZOMBIE_PREVIEW_H, PREVIEW_BOTTOM_LIFT);
}

int LevelSelector::zombie_icon_dir(int type)
{
    auto it = zombie_selector_icons.find(type);
    return it != zombie_selector_icons.end() ? it->second : NORMAL_DIRECTORY;
}

SDL_Rect LevelSelector::plant_preview_rect(int dir, int x, int y)
{
    // Lily pad stays bottom-flush in its box (it already reads right that
    // way); every other plant is lifted off the box floor like the zombie.
    const int lift = dir == (int)PLANT_ICON_DIR[LILYPAD_TYPE] ? 0 : PREVIEW_BOTTOM_LIFT;
    return fit_preview_in_cell(dir, plant_cell(x, y), PLANT_PREVIEW_H, lift);
}

int LevelSelector::BACKGROUND_CHOOSE_LEVEL = CHOOSE_LEVELS_1_DIRECTORY;
int LevelSelector::TYPE_LEVEL = DAY_LIT_DIRECTORY;

LevelSelector::LevelSelector(int level_id)
{
    auto j = get_lv_json(level_id);
    // The cell backdrop is the level's own board art, cropped per backdrop
    // class (backdrop_src_rect).
    this->bg_dir = level_background_dir(level_id);
    if (this->bg_dir < 0)
        this->bg_dir = BACKGROUND_DIRECTORY;
    // Zombies debuting at this level (shared debut tables). Flags and ducky
    // tubes never sit in the JSON rosters (debut_data pins their debuts as
    // overrides), so pull them in via zombie_debut_level.
    this->debut = level_debut_zombies(level_id);
    for (int t : {FLAG_TYPE, DUCKYTUBE_TYPE})
        if (zombie_debut_level(t) == level_id &&
            find(debut.begin(), debut.end(), t) == debut.end())
            debut.push_back(t);
    // Levels where nothing debuts preview their roster's STRONGEST zombie
    // instead (level_roster_strongest_zombie: highest declared "cost"), so
    // no card is a bare backdrop. One icon, through the same slot pipeline;
    // roster-less specials keep the backdrop alone.
    if (this->debut.empty())
    {
        const int strongest = level_roster_strongest_zombie(level_id);
        if (strongest != -1)
            this->debut.push_back(strongest);
    }
    // Reward plant via the registry (reward can also be null/absent); a name
    // the registry misses falls back to no icon instead of throwing.
    this->display_plant = -1;
    if (j.count("reward") && !j["reward"].is_null() && j["reward"].count("plant"))
    {
        string dp = j["reward"]["plant"];
        if (const PlantDef *pdef = plant_def_by_key(dp))
            this->display_plant = PLANT_ICON_DIR[pdef->type];
    }

    this->level_name = j["#comment"];
}

void LevelSelector::display(int x, int y, RGB color)
{
    const SDL_Rect cell = zombie_cell(x, y);
    const SDL_Rect src = backdrop_src_rect(bg_dir);
    if (src.w > 0 && src.h > 0)
        win.draw_png(bg_dir, src.x, src.y, src.w, src.h, cell.x, cell.y, cell.w, cell.h);
    const int n = (int)debut.size();
    for (int i = 0; i < n; i++)
    {
        const int dir = zombie_icon_dir(debut[i]);
        draw_preview_icon(dir, debut_preview_rect(dir, i, n, x, y));
    }
    win.draw_png_height_scaled(SELECTOR_DIRECTORY, x, y, selector_width);
    if (display_plant != -1)
    {
        win.draw_png_width_scaled(CONTAINER_DIRECTORY, x + PLANT_CELL_X, y + PLANT_CELL_Y, PLANT_CELL_W);
        win.set_texture_alpha(display_plant, 255);
        draw_preview_icon(display_plant, plant_preview_rect(display_plant, x, y));
        win.set_texture_alpha(display_plant, 120);
        int w, h;
        TTF_SizeText(win.get_font(PVZUI_TTF, 30), level_name.c_str(), &w, &h);
        int kc = (selector_width - w) / 2;
        win.show_text_shadowed(level_name, x + kc, y + 155, color, PVZUI_TTF, 30);
    }
    else
    {
        int w, h;
        TTF_SizeText(win.get_font(PVZUI_TTF, 40), level_name.c_str(), &w, &h);
        int kc = (selector_width - w) / 2;
        win.show_text_shadowed(level_name, x + kc, y + 130, color, PVZUI_TTF, 40);
    }
}

void LevelSelector::display_locked(int x, int y)
{
    win.set_texture_color(SELECTOR_BLINK_DIRECTORY, 99, 99, 132);
    win.set_texture_alpha(SELECTOR_BLINK_DIRECTORY, 180);
    win.set_texture_color(WHITE_SCREEN_DIRECTORY, 99, 99, 132);
    win.set_texture_alpha(WHITE_SCREEN_DIRECTORY, 180);

    win.draw_png_height_scaled(WHITE_SCREEN_DIRECTORY, x + 5, y + 10, 128);
    win.draw_png_height_scaled(SELECTOR_BLINK_DIRECTORY, x, y, selector_width);
    win.draw_png(LOCK_DIRECTORY, x + selector_width - 25, y - 10, 35, 44);
    win.set_texture_alpha(WHITE_SCREEN_DIRECTORY, 70);
}

void LevelSelector::display_blink(int x, int y)
{
    win.set_texture_color(SELECTOR_BLINK_DIRECTORY, 255, 255, 255);
    win.set_texture_alpha(SELECTOR_BLINK_DIRECTORY, 70);
    win.draw_png_height_scaled(SELECTOR_BLINK_DIRECTORY, x, y, selector_width);
}

void LevelSelector::display_complete(int x, int y)
{
    win.draw_png(COMPLETED_DIRECTORY, x + selector_width - 20, y - 10, 35, 28);
}

bool LevelSelector::is_mouse_in(int mouse_x, int mouse_y, int x, int y)
{
    return (mouse_x >= x && mouse_x <= x + selector_width && mouse_y >= y && mouse_y <= y + 200);
}
