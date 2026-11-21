#include "area_tabs.hpp"
#include "elements/Player/Player.hpp"
#include "events/choose_level_scene/choose_level/choose_level.hpp"
#include "level/level_data.hpp"
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <deque>
#include <map>
using namespace std;

extern Window win;
extern Player player;
extern deque<int> shown_level;

/*Retail layout (no single row) from the PvZ Replanted Unity project dump
(assets_src/replanted_dump, reference only): the Frontend CanvasScaler is
4096x2160 with height match (m_MatchWidthOrHeight: 1), so canvas->window is
one uniform scale k = 600/2160 and this 1050x600 window shows the central
3780x2160 of the canvas (30 canvas px cropped per side, like retail crops
its 3840-wide canvas at 16:9). ModeButtons in UI/Prefabs/P_LevelSelect.prefab
is anchored to the canvas bottom-center (apos 63,183, pivot (0.5,0),
2300x1050) and the sign centers below are its children (all pivot (0.5,0.5),
scale 1), canvas-Y-up from its bottom:
   day    (-899,   623)    594x657    night (-483.5, 768)   545x481
   pool   (17,     850.4)  529x488    fog   (490,    885.9) 482x653
   roof   (844,    666)    516x599    cloudy(1459,   415)  544x688
   bonus  (1307,   824)    418x620
Boxes = those rects x k, shrunk to each sprite's opaque-pixel content (the
content is what gets drawn). The map image in display_background is drawn
at the same k anchored to the same canvas origin, so every sign lands on
its map spot exactly like retail, keeping the diagonal march (day low,
rising to fog, roof dropping again, bonus/cloudy far right).
All boxes stay inside the 1050x600 window and below the level cards (bottom
240) and page arrows (bottom 175). Two soft contacts with the clone's own
chrome (absent in retail) are accepted and resolved by z-order -- the chrome
draws after the signs and its clicks are handled first (choose_level.cpp):
day's left edge slips under the name plaque, cloudy day's bottom-right post
over the quit stone. Signs are drawn back-to-front in AREA_TAB_DRAW_ORDER
(the retail prefab stacking); the hit-test scans the order backwards so the
visually front sign wins the few px where the transparent corners
interlock.*/

static Button area_tab_hit(const SDL_Rect &box)
{
    const int ix = box.w * AREA_TAB_HIT_INSET / 100;
    const int iy = box.h * AREA_TAB_HIT_INSET / 100;
    return Button(box.x + ix, box.x + box.w - ix, box.y + iy, box.y + box.h - iy);
}

const AreaTab AREA_TABS[AREA_TAB_COUNT] = {
    {REPLANTED_BTN_DAY_DIRECTORY, REPLANTED_BTN_DAY_SELECTED_DIRECTORY,
     REPLANTED_BTN_DAY_TOGGLED_DIRECTORY, REPLANTED_BTN_DAY_DISABLED_DIRECTORY,
     {205, 336, 138, 174}, area_tab_hit({205, 336, 138, 174})},
    {REPLANTED_BTN_NIGHT_DIRECTORY, REPLANTED_BTN_NIGHT_SELECTED_DIRECTORY,
     REPLANTED_BTN_NIGHT_TOGGLED_DIRECTORY, REPLANTED_BTN_NIGHT_DISABLED_DIRECTORY,
     {315, 320, 151, 132}, area_tab_hit({315, 320, 151, 132})},
    {REPLANTED_BTN_POOL_DIRECTORY, REPLANTED_BTN_POOL_SELECTED_DIRECTORY,
     REPLANTED_BTN_POOL_TOGGLED_DIRECTORY, REPLANTED_BTN_POOL_DISABLED_DIRECTORY,
     {456, 296, 147, 133}, area_tab_hit({456, 296, 147, 133})},
    {REPLANTED_BTN_FOG_DIRECTORY, REPLANTED_BTN_FOG_SELECTED_DIRECTORY,
     REPLANTED_BTN_FOG_TOGGLED_DIRECTORY, REPLANTED_BTN_FOG_DISABLED_DIRECTORY,
     {598, 263, 130, 181}, area_tab_hit({598, 263, 130, 181})},
    {REPLANTED_BTN_ROOF_DIRECTORY, REPLANTED_BTN_ROOF_SELECTED_DIRECTORY,
     REPLANTED_BTN_ROOF_TOGGLED_DIRECTORY, REPLANTED_BTN_ROOF_DISABLED_DIRECTORY,
     {688, 332, 143, 166}, area_tab_hit({688, 332, 143, 166})},
    {REPLANTED_BTN_CLOUDY_DIRECTORY, REPLANTED_BTN_CLOUDY_SELECTED_DIRECTORY,
     REPLANTED_BTN_CLOUDY_TOGGLED_DIRECTORY, REPLANTED_BTN_CLOUDY_DISABLED_DIRECTORY,
     {865, 389, 139, 183}, area_tab_hit({865, 389, 139, 183})},
    {REPLANTED_BTN_BONUS_DIRECTORY, -1 /* Bonus Levels ships no Selected art */,
     REPLANTED_BTN_BONUS_TOGGLED_DIRECTORY, REPLANTED_BTN_BONUS_DISABLED_DIRECTORY,
     {830, 287, 116, 170}, area_tab_hit({830, 287, 116, 170})},
};

const int AREA_TAB_DRAW_ORDER[AREA_TAB_COUNT] = {
    6, 2, 1, 3, 0, 4, 5, // bottom -> top: BONUS POOL NIGHT FOG DAY ROOF CLOUDY
};

/*World-name labels of the signs: offline-baked PNGs (see make_area_labels.py;
bake sizes: day 96x40, night 107x34, pool 90x28, fog 108x47, roof 128x43,
bonus 81x43 — regenerate the table + these PNGs together), centered at the
retail Label_* anchors. Trường cuối = độ nghiêng chữ (slant, px shear ngang
trên chiều cao nhãn; >0 nghiêng phải, <0 nghiêng trái — đổi tại đây, không
cần bake lại). Cloudy Day has no label.*/
const AreaLabel AREA_LABELS[AREA_TAB_COUNT] = {
    {288, 392, AREA_LABEL_DAY_DIRECTORY, AREA_LABEL_DAY_HOT_DIRECTORY, 0.0f},
    {403, 404, AREA_LABEL_NIGHT_DIRECTORY, AREA_LABEL_NIGHT_HOT_DIRECTORY, 0.0f},
    {521, 357, AREA_LABEL_POOL_DIRECTORY, AREA_LABEL_POOL_HOT_DIRECTORY, 0.0f},
    {652, 396, AREA_LABEL_FOG_DIRECTORY, AREA_LABEL_FOG_HOT_DIRECTORY, 0.0f},
    {751, 407, AREA_LABEL_ROOF_DIRECTORY, AREA_LABEL_ROOF_HOT_DIRECTORY, 0.0f},
    {0, 0, NULL_DIRECTORY, NULL_DIRECTORY, 0.0f},
    {882, 330, AREA_LABEL_BONUS_DIRECTORY, AREA_LABEL_BONUS_HOT_DIRECTORY, 0.0f},
};

int area_of_level(int level_num)
{
    int area = (level_num - 1) / 10;
    if (area < 0)
        area = 0;
    if (area >= AREA_TAB_AREAS)
        area = AREA_TAB_AREAS - 1;
    return area;
}

int area_first_level(int area)
{
    return area * 10 + 1;
}

bool area_unlocked(int area, int unlocked_level)
{
    return unlocked_level >= area_first_level(area);
}

int area_tab_at(int mouse_x, int mouse_y)
{
    // Front-most first: AREA_TAB_DRAW_ORDER runs backwards.
    for (int i = AREA_TAB_COUNT - 1; i >= 0; i--)
    {
        const int t = AREA_TAB_DRAW_ORDER[i];
        if (AREA_TABS[t].hit.is_mouse_in(mouse_x, mouse_y))
            return t;
    }
    return -1;
}

int area_tab_art_dir(int tab, bool hovered)
{
    const AreaTab &tab_ = AREA_TABS[tab];
    // Decorative stubs (Cloudy Day / Bonus) and still-locked worlds.
    if (tab >= AREA_TAB_AREAS || !area_unlocked(tab, player.unlocked_level))
        return tab_.disabled_dir;
    const int active = shown_level.empty() ? 0 : area_of_level(shown_level[0]);
    if (tab == active && tab_.selected_dir != -1)
        return tab_.selected_dir;
    // Hover sheen: the retail Selected art (a white gloss over the normal
    // sign). The pink Toggled art is the pressed state, kept in the set but
    // never shown -- clicks here resolve instantly. Bonus ships no Selected
    // art (in the dump either), so its hover falls back to the normal art.
    if (hovered)
        return tab_.selected_dir != -1 ? tab_.selected_dir : tab_.normal_dir;
    return tab_.normal_dir;
}

AreaTabClick handle_area_tab_click(int mouse_x, int mouse_y)
{
    const int t = area_tab_at(mouse_x, mouse_y);
    if (t == -1)
        return AREA_TAB_NONE;
    // Locked worlds and the decorative stubs swallow the click silently so
    // nothing behind the sign fires.
    if (t >= AREA_TAB_AREAS || !area_unlocked(t, player.unlocked_level))
        return AREA_TAB_DISABLED;
    shown_level = get_page_starting_at(area_first_level(t));
    return AREA_TAB_JUMPED;
}

namespace
{
struct SpriteContent
{
    SDL_Rect content{0, 0, 0, 0}; // opaque-pixel bbox inside the sprite
    int w = 0, h = 0;             // full sprite size
};

/*Opaque-pixel bbox + full size of a button sprite, cached (IMG_Load once,
the same idiom as the selector cards' preview_content). Signs carry uneven
transparent padding, so the measured retail boxes pin the CONTENT, not the
file edges. Missing art yields a zero content -> the box itself is drawn.*/
const SpriteContent &sprite_content(int dir)
{
    static map<int, SpriteContent> cache;
    auto it = cache.find(dir);
    if (it != cache.end())
        return it->second;
    SpriteContent sc;
    if (SDL_Surface *surf = IMG_Load(all_img[dir].img_dir.c_str()))
    {
        sc.w = surf->w;
        sc.h = surf->h;
        if (SDL_Surface *rgba = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0))
        {
            int min_x = rgba->w, min_y = rgba->h, max_x = -1, max_y = -1;
            const Uint32 *pixels = (const Uint32 *)rgba->pixels;
            const int stride = rgba->pitch / sizeof(Uint32);
            for (int y = 0; y < rgba->h; y++)
                for (int x = 0; x < rgba->w; x++)
                    if ((pixels[y * stride + x] >> 24) != 0)
                    {
                        min_x = min(min_x, x);
                        min_y = min(min_y, y);
                        max_x = max(max_x, x);
                        max_y = max(max_y, y);
                    }
            if (max_x >= min_x && max_y >= min_y)
                sc.content = SDL_Rect{min_x, min_y, max_x - min_x + 1, max_y - min_y + 1};
            SDL_FreeSurface(rgba);
        }
        SDL_FreeSurface(surf);
    }
    return cache.emplace(dir, sc).first->second;
}
} // namespace

/*Draw rect of a sign sprite: uniformly scaled so its opaque content lands on
the measured retail box (the tighter axis scale wins, so neither axis
overshoots; any slack centers). State variants are stretched into this same
rect, so switching state never jolts the layout.*/
static SDL_Rect area_tab_dest(const AreaTab &tab)
{
    const SpriteContent &sc = sprite_content(tab.normal_dir);
    if (sc.content.w <= 0 || sc.content.h <= 0 || sc.w <= 0 || sc.h <= 0)
        return tab.box;
    const float sw = tab.box.w / (float)sc.content.w;
    const float sh = tab.box.h / (float)sc.content.h;
    const float s = sw < sh ? sw : sh;
    SDL_Rect d{0, 0, (int)(sc.w * s), (int)(sc.h * s)};
    d.x = tab.box.x + (int)((tab.box.w - sc.content.w * s) / 2 - sc.content.x * s);
    d.y = tab.box.y + (int)((tab.box.h - sc.content.h * s) / 2 - sc.content.y * s);
    return d;
}

/*World-name text of one sign: the offline-baked label image (approved
recipe) drawn centered at the prefab label anchor, warped by the label's
`slant` (horizontal shear qua quad 4 đỉnh của sprite batch — chỉnh độ nghiêng
ở AREA_LABELS).*/
void show_area_label(const AreaLabel &label, const bool hovered)
{
    const int dir = hovered ? label.label_hot_dir : label.label_dir;
    if (dir == NULL_DIRECTORY)
        return;
    GlTexture *tex = win.load_texture(dir);
    if (!tex || tex->id == 0 || tex->w <= 0 || tex->h <= 0)
        return;
    const float x0 = label.cx - tex->w / 2.0f;
    const float y0 = label.cy - tex->h / 2.0f;
    // Shear ngang: mép trên dịch -slant/2, mép dưới +slant/2 (px).
    const float st = -label.slant / 2.0f, sb = label.slant / 2.0f;
    const float x1 = x0 + tex->w, y1 = y0 + tex->h;
    const GlSpriteBatch::Vert verts[4] = {
        {x0 + st, y0, 0, 0, 255, 255, 255, 255},      // TL
        {x1 + st, y0, 1, 0, 255, 255, 255, 255},      // TR
        {x1 + sb, y1, 1, 1, 255, 255, 255, 255},      // BR
        {x0 + sb, y1, 0, 1, 255, 255, 255, 255},      // BL
    };
    win.sprite_batch().drawGeometry(tex->id, verts);
}

void display_area_tabs(const bool &is_mouse_needed)
{
    int _x = 0, _y = 0;
    if (is_mouse_needed)
        SDL_GetMouseState(&_x, &_y);
    // Back-to-front in the retail stacking order; each label goes right
    // after its own sign, so like the prefab a front sign can overlap a
    // lower sign's label.
    for (int i = 0; i < AREA_TAB_COUNT; i++)
    {
        const int t = AREA_TAB_DRAW_ORDER[i];
        const AreaTab &tab = AREA_TABS[t];
        const bool enabled = t < AREA_TAB_AREAS && area_unlocked(t, player.unlocked_level);
        const bool hovered = enabled && is_mouse_needed && tab.hit.is_mouse_in(_x, _y);
        const int dir = area_tab_art_dir(t, hovered);
        const SDL_Rect d = area_tab_dest(tab);
        win.draw_png(dir, d.x, d.y, d.w, d.h);
        const AreaLabel &label = AREA_LABELS[t];
        if (label.cx > 0)
            show_area_label(label, hovered);
    }
}
