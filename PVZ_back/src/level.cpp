#include "level.h"

extern bool is_unlocking_plant;
extern bool is_leave, is_quit, is_reset, is_restart;
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
    if (is_game_started == false)
    {
        display_R_S_P(win, player, level, clk, is_game_started);
    }
    else
    {
        if (has_player_lost(game_characters))
        {
            display_lose(win, player, icons, cells, level, game_characters, quit, is_level_chosen);
            return;
        }
        if (has_player_won(level, game_characters))
        {
            bool is_newest_level = (level.level_num == player.unlocked_level);
            if (display_win(win, player, icons, cells, level, game_characters, quit, is_level_chosen) && is_newest_level)
            {
                for (int i = 0; i < PLANT_COUNT; i++)
                {
                    if (level.level_num == level_unlock_new_plant[i] - 1)
                    {
                        is_unlocking_plant = true;
                    }
                }
            }
            return;
        }
        if (is_restart)
        {
            display_game_restart(win, game_characters, cells, level);
        }
        else if (is_leave)
        {
            display_game_leave(win, game_characters, cells, level);
        }
        else if (is_paused)
        {
            display_game_pause(win, game_characters, cells, level);
            BACK_TO_GAME.blink(win);
            MAIN_MENU.blink(win);
            RESTART.blink(win);
        }
        else
        {
            display_all_in_game(win, player, icons, cells, level, game_characters, clk);
            MENU_ICON.blink(win);
        }
    }
    HANDLE(
        QUIT(quit = true);
        KEY_PRESS(q, quit = true);
        KEY_TO_WIN(level.waves_finished = 1; game_characters.zombies.clear(); game_characters.dead_zombies.clear(););
        LCLICK({
            if (is_restart)
            {
                handle_restart_menu_click(player, game_characters, level, icons, cells,
                                          mouse_x, mouse_y, clk,
                                          is_paused, is_game_started, is_level_chosen);
            }
            else if (is_leave)
            {
                handle_leave_menu_click(player, game_characters, level, icons, cells,
                                        mouse_x, mouse_y, clk,
                                        is_paused, is_game_started, is_level_chosen);
            }
            else if (is_paused)
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

bool display_win(window &win, Player &player, Icons &icons, Map &cells,
                 Level &level, Elements &game_characters, bool &quit, bool &is_level_chosen)
{
    display_winning_message(win, game_characters, cells, level);
    play_music(WIN_MUSIC_DIRECTORY, 0);
    CONTINUE.blink(win);
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
                win.fade_out();
                return true;
            }
        });

    );
    return false;
}

void display_lose(window &win, Player &player, Icons &icons, Map &cells,
                  Level &level, Elements &game_characters, bool &quit, bool &is_level_chosen)
{
    display_losing_message(win, game_characters, cells, level);
    play_music(LOSE_MUSIC_DIRECTORY, 0);
    CONTINUE.blink(win);
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
                win.fade_out();
            }
        });

    );
}

void display_R_S_P(window &win, Player &player, Level &level,
                   int &clk, bool &is_game_started)
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

void display_all_in_game(window &win, Player &player, Icons &icons, Map &cells,
                         Level &level, Elements &game_characters, int &clk)
{
    win.clear_renderer();
    display_game_layout(win, player, icons, level);
    if (player.is_choosing_a_plant || player.is_shoveling)
    {
        blink_row_and_col(win, cells, level);
    }
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

void unlock_plant(window &win, bool &quit, const int &_plant_type)
{
    display_unlock_plant(win, NEW_PEASHOOTER_DIRECTORY + _plant_type);
    play_music(NEW_PLANT_MUSIC_DIRECTORY, 0);
    CONTINUE.blink(win);
    win.update_screen();
    HANDLE(
        QUIT(quit = true);
        KEY_PRESS(q, quit = true);
        LCLICK({
            if (CONTINUE.is_mouse_in(mouse_x, mouse_y))
            {
                win.fade_out();
                is_unlocking_plant = false;
                play_music(OPENING_MUSIC_DIRECTORY);
            }
        });

    );
}