#include "game.hpp"

int game_state = 0;
bool quit = false;
int clk = 0;
Level level;
Elements game_characters;
Icons icons;
Player player;
Map cells;
Window win(WINDOW_WIDTH, WINDOW_HEIGHT);

/*
Init game & music. Set mouse cursor
If player is a new user: Get name and unlock Peashooter.
If a level is not chosen:
    Handle unlocking screen if necessary.
    Handle choosing level screen.
Else: start level.
*/
#include <iomanip>
int main(int argv, char **args)
{
    // Load all image to check image error
    // for (int i = 0; i < COUNT_USED_DIRECTORY; i++)
    // {
    //     win.load_texture(i);
    //     cout << setw(2) << all_img[i].n_sheet << ' ' << all_img[i].c_sheet << ' ';
    //     cout << all_img[i].img_dir << '\n';
    // }

    // Init game & music.
    init_music();
    Mouse Mouse_cursor;
    Mouse_cursor.set_cursor();
    init_game();

    // If player is a new user: Get name and unlock Peashooter.
    display_get_name_player(NEW_USER_DIRECTORY);
    if (player.unlocked_level == 0)
    {
        set_status(game_state, IS_UNLOCKING_PLANT, true);
        while (!quit && check_status(game_state, IS_UNLOCKING_PLANT) == true)
        {
            unlock_plant(PEASHOOTER_TYPE);
        }
        update_unlocked_level();
    }

    // Main loop
    while (!quit)
    {
        if (check_status(game_state, IS_LEVEL_CHOSEN) == false)
        {
            if (check_status(game_state, IS_UNLOCKING_PLANT) == true)
            {
                for (int i = 0; i < PLANT_COUNT; i++)
                {
                    if (level.level_num == level_unlock_new_plant[i] - 1)
                    {
                        unlock_plant(i);
                        break;
                    }
                    else if (level.level_num == 12)
                    {
                        unlock_plant(SUNFLOWER_TROPHY);
                        break;
                    }
                }
            }
            else
            {
                play_music(OPENING_MUSIC_DIRECTORY);
                // win.clear_renderer();
                handle_choosing_level_screen();
                if (check_status(game_state, IS_LEVEL_CHOSEN))
                {
                    Mix_HaltMusic();
                    play_sound_effect(EVIL_LAUGH_MUSIC_DIRECTORY);
                    clk = 0;
                    set_status(game_state, IS_GAME_STARTED, false);
                    win.fade_out();
                }
            }
        }
        else
        {
            win.clear_renderer();
            start_level();
        }
    }
    close_music();
    return 0;
}
