#include "choose_level.h"

extern bool level_chosen;
extern bool quit;
extern bool is_game_started;
extern bool is_paused;
extern bool is_unlocking_plant;
extern int clk;
extern Level level;
extern Elements game_characters;
extern Icons icons;
extern Player player;
extern Map cells;
extern window win;
extern bool is_quit, is_reset;

/*New function: Display choosing level
Display choosing level screen.
*/
void display_choosing_level_screen()
{
    win.clear_renderer();
    win.draw_png_scale(CHOOSE_LEVELS_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    display_button(win, RENAME_BUTTON, RENAME_DIRECTORY);
    display_button(win, RESET_LEVEL_BUTTON, RESET_LEVEL_DIRECTORY);
    display_button(win, QUIT_BUTTON, QUIT_DIRECTORY);
    RENAME_BUTTON.blink(win);
    RESET_LEVEL_BUTTON.blink(win);
    QUIT_BUTTON.blink(win);

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
                win.show_text("Level " + to_string(i), LEVEL_BUTTON[i].x1 + 60, LEVEL_BUTTON[i].y1 + 130);
        }
        else
        {
            win.show_text("Level " + to_string(i), LEVEL_BUTTON[i].x1 + 60, LEVEL_BUTTON[i].y1 + 130, BLACK);

            display_level_is_locked(win, LEVEL_BUTTON[i]);
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
                display_get_name_player(win, player, quit, RENAME_MENU_DIRECTORY);
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
                    load_level(player, level);
                    level.background_directory = BACKGROUND_LV1_DIRECTORY + level.map_type;
                    level_chosen = true;
                    break;
                }
        });

    );
}