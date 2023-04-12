#include "choose_level.hpp"

extern int game_state;
extern bool quit;
extern Level level;
extern Player player;
extern window win;

/*
Handle choosing level screen:
    Display choosing level screen (3 type: normal, quit, reset level; 1 more type is rename in player_name.hpp)
    Handle quit menu, reset menu, quit/reset level/rename button, levels respectively.
*/
void handle_choosing_level_screen()
{
    if (check_status(game_state, IS_QUIT) == true)
    {
        display_game_quit();
    }
    else if (check_status(game_state, IS_RESET) == true)
    {
        display_game_reset();
    }
    else
    {
        display_choose_level(true);
    }
    win.update_screen();
    HANDLE(
        QUIT(quit = true; exit(0););
        LCLICK({
            if (check_status(game_state, IS_QUIT) == true)
            {
                handle_quit_menu_click(mouse_x, mouse_y);
                break;
            }
            if (check_status(game_state, IS_RESET) == true)
            {
                handle_reset_menu_click(mouse_x, mouse_y);
                break;
            }
            if (RESET_LEVEL_BUTTON.is_mouse_in(mouse_x, mouse_y))
            {
                play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                set_status(game_state, IS_RESET, true);
                break;
            }
            if (RENAME_BUTTON.is_mouse_in(mouse_x, mouse_y))
            {
                play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                display_get_name_player(RENAME_MENU_DIRECTORY);
                break;
            }
            if (QUIT_BUTTON.is_mouse_in(mouse_x, mouse_y))
            {
                play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                set_status(game_state, IS_QUIT, true);
                break;
            }
            for (int i = 1; i <= LEVEL_COUNT; i++)
                if (player.unlocked_level >= i && LEVEL_BUTTON[i].is_mouse_in(mouse_x, mouse_y))
                {
                    level.level_num = i;
                    load_level();
                    level.background_directory = BACKGROUND_LV1_DIRECTORY + level.map_type;
                    set_status(game_state, IS_LEVEL_CHOSEN, true);
                    break;
                }
        });

    );
}