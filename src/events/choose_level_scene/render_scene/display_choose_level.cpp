#include "display_choose_level.hpp"
#include "LevelSelector.hpp"
#include "area_tabs.hpp"
#include "elements/Player/Player.hpp"
#include "level/level_data.hpp"
#include <cmath>
#include <deque>

extern Window win;
extern deque<int> shown_level;
extern Player player;

void display_background();
void display_selectors(const bool &is_mouse_needed);
void display_change_page_buttons(const bool &is_mouse_needed);
void display_quit_button(const bool &is_mouse_needed);
void display_reset_level_button(const bool &is_mouse_needed);
void display_back_button(const bool &is_mouse_needed);

void display_choose_level(const bool &is_mouse_needed)
{
    win.clear_renderer();
    display_background();
    display_area_tabs(is_mouse_needed);
    display_change_page_buttons(is_mouse_needed);
    display_selectors(is_mouse_needed);
    display_quit_button(is_mouse_needed);
    display_reset_level_button(is_mouse_needed);
    display_back_button(is_mouse_needed);
}

/*PvZ Replanted world map at the retail panel's own scale. The frontend
CanvasScaler (Frontend.unity, the canvas hosting P_LevelSelect) is 4096x2160
with height match, so canvas->window is the uniform scale k = 600/2160 and
the window shows the central 3780x2160 of the canvas (30 canvas px cropped
per side, same as retail crops its 3840-wide canvas at 16:9). Under the map,
the scene's BG_Sky sprite (SPR_MainMenu_Sky, full-canvas stretch) supplies
the sky band above the map; the retail level select renders it through a
hue-shift material, so resources/images/replanted/sky.png is the original
asset graded OFFLINE to the level-select screenshot's dusk band. (sky_rip
is the RIP-mode variant, unwired.) The prefab's 5368x1640 map image (pivot
(0.5,0), anchored (-34,0) at the canvas bottom-center in
P_LevelSelect.prefab) lands at {-230, 144, 1491, 456}, bottom-anchored like
retail. Same scale and origin as the area signs (area_tabs.cpp), so every
sign sits on its map spot. One art for every world page -- the old
day/night CHOOSE_LEVELS flip is gone with the reskin.*/
void display_background()
{
    const double k = WINDOW_HEIGHT / 2160.0;
    const SDL_Point sky = LevelSelector::backdrop_art_size(REPLANTED_SKY_DIRECTORY);
    if (sky.x > 0 && sky.y > 0)
    {
        // Full-canvas image under everything: the window frames the central
        // 3780 of the 4096-wide canvas, so the src crop is centered the same.
        const int sw = (int)lround(sky.x * 3780.0 / 4096.0);
        win.draw_png(REPLANTED_SKY_DIRECTORY, (sky.x - sw) / 2, 0, sw, sky.y,
                     0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    }
    const SDL_Point art =
        LevelSelector::backdrop_art_size(REPLANTED_LEVELSELECT_BG_DIRECTORY);
    if (art.x > 0 && art.y > 0)
    {
        const int w = (int)lround(5368 * k);
        const int h = (int)lround(1640 * k);
        const int x = (int)lround((1920 - 34 - 30 - 5368 / 2.0) * k);
        const int y = (int)lround((2160 - 1640) * k);
        win.draw_png(REPLANTED_LEVELSELECT_BG_DIRECTORY, 0, 0, art.x, art.y,
                     x, y, w, h);
    }
}

void display_selectors(const bool &is_mouse_needed)
{
    for (int i = 0, _ = shown_level.size(); i < _; ++i)
    {
        auto level_now = LevelSelector(shown_level[i]);
        const int &x = LEVEL_BUTTON[i].x;
        const int &y = LEVEL_BUTTON[i].y;
        if (player.unlocked_level >= shown_level[i])
        {
            if (is_mouse_needed)
            {
                int _x = 0, _y = 0;
                SDL_GetMouseState(&_x, &_y);
                if (level_now.is_mouse_in(_x, _y, x, y))
                {
                    level_now.display(x, y, GREEN);
                    level_now.display_blink(x, y);
                }
                else
                    level_now.display(x, y, WHITE);
            }
            else
                level_now.display(x, y, WHITE);
            if (player.unlocked_level > shown_level[i])
                level_now.display_complete(x, y);
        }
        else
        {
            level_now.display(x, y, WHITE);
            level_now.display_locked(x, y);
        }
    }
}

void display_change_page_buttons(const bool &is_mouse_needed)
{
    if (shown_level[0] != 1)
        PREV_PAGE_BUTTON.display(BACK_BUTTON_DIRECTORY);
    if (shown_level.back() != LEVEL_COUNT)
        NEXT_PAGE_BUTTON.display(BACK_BUTTON_DIRECTORY, 0, SDL_FLIP_HORIZONTAL);
    if (is_mouse_needed)
    {
        int _x = 0, _y = 0;
        SDL_GetMouseState(&_x, &_y);
        if (PREV_PAGE_BUTTON.is_mouse_in(_x, _y) && shown_level[0] != 1)
            PREV_PAGE_BUTTON.display(BACK_PRESS_BUTTON_DIRECTORY);
        if (NEXT_PAGE_BUTTON.is_mouse_in(_x, _y) && shown_level.back() != LEVEL_COUNT)
            NEXT_PAGE_BUTTON.display(BACK_PRESS_BUTTON_DIRECTORY, 0, SDL_FLIP_HORIZONTAL);
    }
}

/*Chrome stone slab with a READABLE label (user request: buttons back to
English): the STONE art stays, the label is the pre-atlas Dwarvesc TTF with
the old palette -- black ink over a pale-green fringe, pink/magenta on hover
(the exact look before the dwarven atlas migration). scale rides the
slab's height (scale 1.4 = the old 30-pt QUIT/reset fill).*/
static void show_stone_label(const Button &btn, const string &text, bool hot, float scale)
{
    const RGB ink = hot ? RGB(255, 222, 247) : BLACK;
    const RGB edge = hot ? RGB(244, 67, 191) : RGB(162, 203, 134);
    const int size = (int)lroundf(21.4f * scale);
    int tw, th;
    TTF_SizeText(win.get_font(DWARVESC_TTF, size), text.c_str(), &tw, &th);
    win.show_text_shadowed(text, (btn.x1 + btn.x2 - tw) / 2,
                           (btn.y1 + btn.y2 - th) / 2 - (int)(2 * scale), ink,
                           DWARVESC_TTF, size, 0, 1, edge);
}

void display_quit_button(const bool &is_mouse_needed)
{
    QUIT_BUTTON.display(STONE_DIRECTORY, 358);
    show_stone_label(QUIT_BUTTON, "QUIT", false, 1.4f);
    if (is_mouse_needed)
    {
        int _x = 0, _y = 0;
        SDL_GetMouseState(&_x, &_y);
        if (QUIT_BUTTON.is_mouse_in(_x, _y))
            show_stone_label(QUIT_BUTTON, "QUIT", true, 1.4f);
    }
}

void display_back_button(const bool &is_mouse_needed)
{
    BACK_TO_SELECTOR_BUTTON.display(STONE_DIRECTORY, 358);
    show_stone_label(BACK_TO_SELECTOR_BUTTON, "BACK", false, 1.0f);
    if (is_mouse_needed)
    {
        int _x = 0, _y = 0;
        SDL_GetMouseState(&_x, &_y);
        if (BACK_TO_SELECTOR_BUTTON.is_mouse_in(_x, _y))
            show_stone_label(BACK_TO_SELECTOR_BUTTON, "BACK", true, 1.0f);
    }
}

void display_reset_level_button(const bool &is_mouse_needed)
{
    RESET_LEVEL_BUTTON.display(STONE_DIRECTORY, 358);
    show_stone_label(RESET_LEVEL_BUTTON, "reset level", false, 1.4f);
    if (is_mouse_needed)
    {
        int _x = 0, _y = 0;
        SDL_GetMouseState(&_x, &_y);
        if (RESET_LEVEL_BUTTON.is_mouse_in(_x, _y))
            show_stone_label(RESET_LEVEL_BUTTON, "reset level", true, 1.4f);
    }
}