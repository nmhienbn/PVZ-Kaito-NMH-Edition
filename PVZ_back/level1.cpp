#include "level1.h"

void start_level_1(window &win, Player &player, Icons &icons, Map &map, Level &level, Elements &game_characters, int &clk, bool &quit, bool &is_game_started, bool &is_level_chosen)
{
    win.clear_renderer();
    int start_time = SDL_GetTicks();
    if (!is_game_started)
    {
        if (clk < 30)
        {
            display_ready_set_plant(win, START_READY_DIRECTORY, level);
        }
        else if (clk < 60)
        {
            display_ready_set_plant(win, START_SET_DIRECTORY, level);
        }
        else if (clk < 90)
        {
            display_ready_set_plant(win, START_PLANT_DIRECTORY, level);
        }
        else
        {
            clk = 0;
            is_game_started = true;
        }
    }
    else
    {
        if (has_player_lost(game_characters))
        {
            display_losing_message(win, game_characters, map);
            Mix_HaltMusic();
        }
        else if (has_player_won(level, game_characters))
        {
            display_winning_message(win);
            update_unlocked_level(player, level);
            level.waves_finished = false;
            is_level_chosen = false;
            Mix_HaltMusic();
            reset_level(game_characters);
        }
        else
        {
            display_game_layout(win, player, icons, level);
            display_game_elements(win, game_characters, map);
            display_chosen_plant(win, player, icons);
            handle_movements(game_characters, map, clk);
            handle_changes(icons, game_characters, map, level, clk);
            play_in_game_music();
        }
    }

    HANDLE(
        QUIT(quit = true);
        KEY_PRESS(q, quit = true);
        KEY_TO_WIN(level.waves_finished = 1; game_characters.zombies.clear());
        LCLICK({
            handle_user_click(player, level, icons, game_characters, map, mouse_x, mouse_y);
        });

    );

    clk++;
    win.update_screen();

    int now_time = SDL_GetTicks() - start_time;
    int delay_time = std::max(5, ticks_per_frame - now_time);
    // DELAY(delay_time);
}