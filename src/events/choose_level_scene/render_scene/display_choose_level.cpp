#include "display_choose_level.hpp"
#include "LevelSelector.hpp"
#include "elements/Player/Player.hpp"
#include "level/level_data.hpp"
#include <deque>

extern Window win;
extern deque<int> shown_level;
extern Player player;

void display_background();
void display_selectors(const bool &is_mouse_needed);
void display_change_page_buttons(const bool &is_mouse_needed);
void display_name_bar(const bool &is_mouse_needed);
void display_quit_button(const bool &is_mouse_needed);
void display_reset_level_button(const bool &is_mouse_needed);

void display_choose_level(const bool &is_mouse_needed)
{
    win.clear_renderer();
    display_background();
    display_change_page_buttons(is_mouse_needed);
    display_name_bar(is_mouse_needed);
    display_selectors(is_mouse_needed);
    display_quit_button(is_mouse_needed);
    display_reset_level_button(is_mouse_needed);
}

void display_background()
{
    win.draw_png_height_scaled(LevelSelector::BACKGROUND_CHOOSE_LEVEL, 0, 0, WINDOW_WIDTH);
    win.draw_png(DAY_DIRECTORY, 239, 435, 101, 76);
    win.draw_png(NIGHT_DIRECTORY, 345, 443, 124, 58);
    if (LevelSelector::TYPE_LEVEL == DAY_LIT_DIRECTORY)
        win.draw_png(DAY_LIT_DIRECTORY, 239, 435, 101, 76);
    else
        win.draw_png(NIGHT_LIT_DIRECTORY, 345, 443, 124, 58);
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
                    if (shown_level[i] >= 8)
                    {
                        LevelSelector::BACKGROUND_CHOOSE_LEVEL = CHOOSE_LEVELS_2_DIRECTORY;
                        LevelSelector::TYPE_LEVEL = NIGHT_LIT_DIRECTORY;
                    }
                    else
                    {
                        LevelSelector::BACKGROUND_CHOOSE_LEVEL = CHOOSE_LEVELS_1_DIRECTORY;
                        LevelSelector::TYPE_LEVEL = DAY_LIT_DIRECTORY;
                    }
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

void display_name_bar(const bool &is_mouse_needed)
{
    RENAME_BUTTON.display(NAME_DIRECTORY);
    win.show_text(player.name, 40, 350, WHITE, PVZUI_TTF, 30);
}

void display_quit_button(const bool &is_mouse_needed)
{
    QUIT_BUTTON.display(STONE_DIRECTORY, 358);
    QUIT_BUTTON.show_text("QUIT", 0, -8, 30, BLACK, RGB(162, 203, 134));
    if (is_mouse_needed)
    {
        int _x = 0, _y = 0;
        SDL_GetMouseState(&_x, &_y);
        if (QUIT_BUTTON.is_mouse_in(_x, _y))
            QUIT_BUTTON.show_text("QUIT", 0, -8, 30, RGB(255, 222, 247), RGB(244, 67, 191));
    }
}

void display_reset_level_button(const bool &is_mouse_needed)
{
    RESET_LEVEL_BUTTON.display(STONE_DIRECTORY, 358);
    RESET_LEVEL_BUTTON.show_text("reset level", 0, -8, 30, BLACK, RGB(162, 203, 134));
    if (is_mouse_needed)
    {
        int _x = 0, _y = 0;
        SDL_GetMouseState(&_x, &_y);
        if (RESET_LEVEL_BUTTON.is_mouse_in(_x, _y))
            RESET_LEVEL_BUTTON.show_text("reset level", 0, -8, 30, RGB(255, 222, 247), RGB(244, 67, 191));
    }
}