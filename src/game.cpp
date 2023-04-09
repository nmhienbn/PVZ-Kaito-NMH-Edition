#include "game.hpp"

bool level_chosen = false;
bool quit = false;
bool is_game_started = false;
bool is_paused = false;
bool is_unlocking_plant = false;
int clk = 0;
Level level;
Elements game_characters;
Icons icons;
Player player;
Map cells;
window win(WINDOW_WIDTH, WINDOW_HEIGHT);

bool is_leave, is_quit, is_reset, is_restart;

/*
Init game & music.
If player is a new user: Get name and unlock Peashooter.
If a level is not chosen:
    Handle unlocking screen if necessary.
    Handle choosing level screen.
Else: start level.

*/
int main(int argv, char **args)
{
    // Init game & music.
    init_music();
    init_game();

    // If player is a new user: Get name and unlock Peashooter.
    display_get_name_player(NEW_USER_DIRECTORY);
    if (player.unlocked_level == 0)
    {
        is_unlocking_plant = true;
        while (!quit && is_unlocking_plant)
        {
            unlock_plant(PEASHOOTER);
        }
        update_unlocked_level();
    }

    // Main loop
    while (!quit)
    {
        if (level_chosen == false)
        {
            if (is_unlocking_plant)
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
                // win.clear_renderer();
                handle_choosing_level_screen();
                if (level_chosen)
                {
                    Mix_HaltMusic();
                    play_sound_effect(EVIL_LAUGH_MUSIC_DIRECTORY);
                    clk = 0;
                    is_game_started = false;
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
