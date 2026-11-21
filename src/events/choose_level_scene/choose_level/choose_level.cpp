#include "choose_level.hpp"
#include "draw/render_elements.hpp"
#include "events/anim_viewer/anim_viewer.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Player/Player.hpp"
#include "events/choose_level_scene/quit_menu/quit_menu.hpp"
#include "events/choose_level_scene/render_scene/LevelSelector.hpp"
#include "events/choose_level_scene/render_scene/area_tabs.hpp"
#include "events/choose_level_scene/render_scene/display_choose_level.hpp"
#include "events/choose_level_scene/reset_level/reset_menu.hpp"
#include "level/level_data.hpp"
#include "player/player_data.hpp"
#include <deque>
using namespace std;

extern int game_state;
extern bool quit;
extern Level level;
extern Player player;
extern Window win;
extern deque<int> shown_level;

const int LEVELS_PER_PAGE = 5;

deque<int> get_page_starting_at(int first)
{
    deque<int> page;
    for (int lv = first; lv < first + LEVELS_PER_PAGE && lv <= LEVEL_COUNT; lv++)
        page.push_back(lv);
    return page;
}

deque<int> get_prev_page(deque<int> shown_level)
{
    int first = shown_level[0] - LEVELS_PER_PAGE;
    if (first < 1)
        first = 1;
    return get_page_starting_at(first);
}

deque<int> get_next_page(deque<int> shown_level)
{
    if (shown_level.back() >= LEVEL_COUNT)
        return shown_level;
    return get_page_starting_at(shown_level[0] + LEVELS_PER_PAGE);
}

/*
Handle choosing level screen:
    Display choosing level screen (3 type: normal, quit, reset level).
    Handle quit menu, reset menu, quit/reset level button, levels, respectively.
    (Rename lives on the selector/mode screen's wood sign, a la retail.)
*/
void handle_choosing_level_screen()
{
    // Quit
    if (check_status(game_state, IS_QUIT) == true)
    {
        display_game_quit();
    }
    // Reset levels
    else if (check_status(game_state, IS_RESET) == true)
    {
        display_game_reset();
    }
    // choose level
    else
    {
        display_choose_level(true);
    }
    win.update_screen();
    HANDLE_SDL_EVENT(
        QUIT();
        LCLICK({
            // Going to quit
            if (check_status(game_state, IS_QUIT) == true)
            {
                handle_quit_menu_click(mouse_x, mouse_y);
                break;
            }
            // Going to reset levels
            if (check_status(game_state, IS_RESET) == true)
            {
                handle_reset_menu_click(mouse_x, mouse_y);
                break;
            }
            // Back to the selector/mode screen (tombstone picker)
            if (BACK_TO_SELECTOR_BUTTON.is_mouse_in(mouse_x, mouse_y))
            {
                play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                set_status(game_state, IS_SELECTOR_SCREEN, true);
                break;
            }
            // Reset levels
            if (RESET_LEVEL_BUTTON.is_mouse_in(mouse_x, mouse_y))
            {
                play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                set_status(game_state, IS_RESET, true);
                break;
            }
            // Quit
            if (QUIT_BUTTON.is_mouse_in(mouse_x, mouse_y))
            {
                play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                set_status(game_state, IS_QUIT, true);
                break;
            }
            //
            if (PREV_PAGE_BUTTON.is_mouse_in(mouse_x, mouse_y))
            {
                play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                shown_level = get_prev_page(shown_level);
                break;
            }
            if (NEXT_PAGE_BUTTON.is_mouse_in(mouse_x, mouse_y))
            {
                play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                shown_level = get_next_page(shown_level);
                break;
            }
            // World buttons: jump the page to the world's first level. A
            // locked or decorative button swallows the click silently so
            // nothing behind it fires.
            AreaTabClick tab_click = handle_area_tab_click(mouse_x, mouse_y);
            if (tab_click != AREA_TAB_NONE)
            {
                if (tab_click == AREA_TAB_JUMPED)
                    play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                break;
            }
            // Level choosing
            for (int i = 0, _ = shown_level.size(); i < _; i++)
                if (player.unlocked_level >= shown_level[i] &&
                    LevelSelector(shown_level[i]).is_mouse_in(mouse_x, mouse_y, LEVEL_BUTTON[i].x, LEVEL_BUTTON[i].y))
                {
                    level.level_num = shown_level[i];
                    load_level();
                    level.background_directory = BACKGROUND_LV1_DIRECTORY + level.map_type;
                    set_status(game_state, IS_LEVEL_CHOSEN, true);
                    break;
                }
        });
        // Debug animation viewer (renders with the real game pipeline)
        KEY_PRESS(F1, display_anim_viewer(););

    );
}
