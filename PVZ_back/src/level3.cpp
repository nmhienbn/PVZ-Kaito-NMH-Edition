#include "level3.h"

/*
If game has not started:
    0.5s: display ready.
    0.5s: display set.
    2s: display plant.
Else: (game has started)
    Check if player lose/win: display lose/win message. Reset level.
    Else check if game is paused or not.
    Else display gameplay.

Handle user click:
    Quit event.
    Key to win. //For developer.
    Menu icon and menu click.
    Player click on game.
*/
void start_level_3(window &win, Player &player, Icons &icons, Map &cells,
                   Level &level, Elements &game_characters, int &clk, bool &quit,
                   bool &is_game_started, bool &is_level_chosen, bool &is_paused)
{
    if (!is_game_started)
    {
        play_music(R_S_P_MUSIC_DIRECTORY);
        if (clk < 30)
        {
            display_ready_set_plant(win, player, START_READY_DIRECTORY, level);
        }
        else if (clk < 60)
        {
            display_ready_set_plant(win, player, START_SET_DIRECTORY, level);
        }
        else if (clk < 180)
        {
            display_ready_set_plant(win, player, START_PLANT_DIRECTORY, level);
        }
        else
        {
            display_ready_set_plant(win, player, START_PLANT_DIRECTORY, level);
            clk = 0;
            is_game_started = true;
        }
    }
    else
    {
        if (has_player_lost(game_characters))
        {
            display_losing_message(win, game_characters, cells, level);
            play_music(LOSE_MUSIC_DIRECTORY, 0);
            win.update_screen();
            HANDLE(
                QUIT(quit = true);
                KEY_PRESS(q, quit = true);
                LCLICK({
                    if (CONTINUE.is_mouse_in(mouse_x, mouse_y))
                    {
                        level.waves_finished = false;
                        is_level_chosen = false;
                        play_music(OPENING_MUSIC_DIRECTORY);
                        reset_level(game_characters, cells, icons);
                    }
                });

            );
            win.fade_out();
            return;
        }
        if (has_player_won(level, game_characters))
        {
            display_winning_message(win, game_characters, cells, level);
            play_music(WIN_MUSIC_DIRECTORY, 0);
            win.update_screen();
            HANDLE(
                QUIT(quit = true);
                KEY_PRESS(q, quit = true);
                LCLICK({
                    if (CONTINUE.is_mouse_in(mouse_x, mouse_y))
                    {
                        update_unlocked_level(player, level);
                        level.waves_finished = false;
                        is_level_chosen = false;
                        reset_level(game_characters, cells, icons);
                        play_music(OPENING_MUSIC_DIRECTORY);
                    }
                });

            );
            win.fade_out();
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
            display_game_announce(win, level);
            display_chosen_plant(win, player, icons);
            handle_movements(game_characters, cells, clk);
            handle_changes(icons, game_characters, cells, level, clk);
            play_music(BACKGROUND_MUSIC_DIRECTORY);
            if (game_characters.zombies.size() && !rand(0, 500))
            {
                play_sound_effect(GROAN_MUSIC_DIRECTORY);
            }
        }
    }
    HANDLE(
        QUIT(quit = true);
        KEY_PRESS(q, quit = true);
        KEY_TO_WIN(level.waves_finished = 1; game_characters.zombies.clear(); game_characters.dead_zombies.clear(););
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
}