#include "click_handle/out_game/choose_level/choose_level.hpp"

extern bool level_chosen;
extern bool quit;
extern Level level;
extern Player player;
extern window win;
extern bool is_quit, is_reset;

/*New function: Display choosing level
Display choosing level screen.
*/
void display_choosing_level_screen()
{
    win.clear_renderer();
    win.draw_png_scale(CHOOSE_LEVELS_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    display_button(RENAME_BUTTON, RENAME_DIRECTORY);
    display_button(RESET_LEVEL_BUTTON, RESET_LEVEL_DIRECTORY);
    display_button(QUIT_BUTTON, QUIT_DIRECTORY);
    RENAME_BUTTON.blink();
    RESET_LEVEL_BUTTON.blink();
    QUIT_BUTTON.blink();

    int _x = 0, _y = 0;
    SDL_GetMouseState(&_x, &_y);
    for (int i = 1; i <= LEVEL_COUNT; i++)
    {
        if (player.unlocked_level >= i)
        {
            if (LEVEL_BUTTON[i].is_mouse_in(_x, _y))
            {
                win.show_text("Level " + to_string(i), LEVEL_BUTTON[i].x1 + 60, LEVEL_BUTTON[i].y1 + 130, GREEN);
                win.draw_png(WHITE_SCREEN_DIRECTORY, LEVEL_BUTTON[i].x1, LEVEL_BUTTON[i].y1, LEVEL_BUTTON[i].x2 - LEVEL_BUTTON[i].x1, LEVEL_BUTTON[i].y2 - LEVEL_BUTTON[i].y1);
            }
            else
                win.show_text("Level " + to_string(i), LEVEL_BUTTON[i].x1 + 60, LEVEL_BUTTON[i].y1 + 130, WHITE);
        }
        else
        {
            win.show_text("Level " + to_string(i), LEVEL_BUTTON[i].x1 + 60, LEVEL_BUTTON[i].y1 + 130, BLACK);

            display_level_is_locked(LEVEL_BUTTON[i]);
        }
    }
}

void handle_choosing_level_screen()
{
    if (is_quit)
    {
        display_game_quit();
    }
    else if (is_reset)
    {
        display_game_reset();
    }
    else
    {
        display_choosing_level_screen();
    }
    win.update_screen();
    HANDLE(
        QUIT(quit = true; exit(0););
        // KEY_PRESS(q, quit = true);
        LCLICK({
            if (is_quit)
            {
                handle_quit_menu_click(mouse_x, mouse_y);
                break;
            }
            if (is_reset)
            {
                handle_reset_menu_click(mouse_x, mouse_y);
                break;
            }
            if (RESET_LEVEL_BUTTON.is_mouse_in(mouse_x, mouse_y))
            {
                play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                is_reset = true;
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
                is_quit = true;
                break;
            }
            for (int i = 1; i <= LEVEL_COUNT; i++)
                if (player.unlocked_level >= i && LEVEL_BUTTON[i].is_mouse_in(mouse_x, mouse_y))
                {
                    level.level_num = i;
                    load_level();
                    level.background_directory = BACKGROUND_LV1_DIRECTORY + level.map_type;
                    level_chosen = true;
                    break;
                }
        });

    );
}