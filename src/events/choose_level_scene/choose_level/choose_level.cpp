#include "choose_level.hpp"
#include "draw/render_elements.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Player/Player.hpp"
#include "events/choose_level_scene/quit_menu/quit_menu.hpp"
#include "events/choose_level_scene/render_scene/LevelSelector.hpp"
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

deque<int> get_prev_page(deque<int> shown_level)
{
    for (int i = 1; i <= 6; i++)
    {
        if (shown_level[0] > 1)
            shown_level.push_front(shown_level[0] - 1);
        else
            break;
    }
    while (shown_level.size() > 6)
        shown_level.pop_back();
    return shown_level;
}

deque<int> get_next_page(deque<int> shown_level)
{
    for (int i = 1; i <= 6; i++)
    {
        if (shown_level.back() < LEVEL_COUNT)
            shown_level.push_back(shown_level.back() + 1);
        else
            break;
    }
    while (shown_level.size() > 6)
        shown_level.pop_front();
    return shown_level;
}

/*
Handle choosing level screen:
    Display choosing level screen (4 type: normal, quit, reset level; last type is rename in player_name.hpp)
    Handle quit menu, reset menu, quit/reset level/rename button, levels, respectively.
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
            // Reset levels
            if (RESET_LEVEL_BUTTON.is_mouse_in(mouse_x, mouse_y))
            {
                play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                set_status(game_state, IS_RESET, true);
                break;
            }
            // Rename
            if (RENAME_BUTTON.is_mouse_in(mouse_x, mouse_y))
            {
                play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                display_get_name_player(RENAME_MENU_DIRECTORY);
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

    );
}
