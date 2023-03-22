#include "level3.h"

void start_level_3(window &win, Player &player, Icons &icons, Map &cells,
                   Level &level, Elements &game_characters, int &clk, bool &quit,
                   bool &is_game_started, bool &is_level_chosen, bool &is_paused)
{
    int start_time = SDL_GetTicks();
    if (!is_game_started)
    {
        win.clear_renderer();
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
            display_losing_message(win, game_characters, cells, level);
            win.update_screen();
            SDL_Delay(3000);
            level.waves_finished = false;
            is_level_chosen = false;
            Mix_HaltMusic();
            reset_level(game_characters, cells);
            return;
        }
        if (has_player_won(level, game_characters))
        {
            display_winning_message(win, game_characters, cells, level);
            win.update_screen();
            SDL_Delay(3000);
            update_unlocked_level(player, level);
            level.waves_finished = false;
            is_level_chosen = false;
            Mix_HaltMusic();
            reset_level(game_characters, cells);
            return;
        }
        if (is_paused)
        {
            display_game_pause(win, game_characters, cells, level);
        }
        else
        {
            win.clear_renderer();
            display_game_layout(win, player, icons, level);
            display_game_elements(win, game_characters, cells);
            display_chosen_plant(win, player, icons);
            handle_movements(game_characters, cells, clk);
            handle_changes(icons, game_characters, cells, level, clk);
            play_in_game_music();
        }
    }
    HANDLE(
        QUIT(quit = true);
        KEY_PRESS(q, quit = true);
        KEY_TO_WIN(level.waves_finished = 1; game_characters.zombies.clear());
        LCLICK({
            if (is_paused)
            {
                handle_menu_click(player, game_characters, level, icons, cells,
                                  mouse_x, mouse_y, clk,
                                  is_paused, is_game_started, is_level_chosen);
            }
            else
            {
                handle_menu_icon_click(mouse_x, mouse_y, is_paused);
                handle_user_click(player, level, icons, game_characters, cells, mouse_x, mouse_y);
            }
        });

    );

    if (is_paused == false)
        clk++;
    win.update_screen();

    int now_time = SDL_GetTicks() - start_time;
    int delay_time = std::max(5, ticks_per_frame - now_time);
    // DELAY(delay_time);
}