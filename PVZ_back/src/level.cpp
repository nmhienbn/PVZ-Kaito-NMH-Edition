#include "level.h"

extern bool level_chosen,
    quit,
    is_game_started,
    is_paused,
    is_unlocking_plant;
extern int clk;
extern Level level;
extern Elements game_characters;
extern Player player;
extern window win;
extern bool is_leave,
    is_restart;

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
void start_level()
{
    if (is_game_started == false)
    {
        display_R_S_P();
    }
    else
    {
        if (has_player_lost())
        {
            display_lose();
            return;
        }
        if (has_player_won())
        {
            bool is_newest_level = (level.level_num == player.unlocked_level);
            if (display_win() && is_newest_level)
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
            display_game_restart();
        }
        else if (is_leave)
        {
            display_game_leave();
        }
        else if (is_paused)
        {
            display_game_pause();
        }
        else
        {
            display_all_in_game();
            MENU_ICON.blink();
        }
    }
    HANDLE(
        QUIT(quit = true);
        KEY_PRESS(q, quit = true);
        KEY_TO_WIN(
            level.waves_finished = 1;
            game_characters.zombies.clear();
            game_characters.dead_zombies.clear();

        );
        LCLICK({
            if (is_restart)
            {
                handle_restart_menu_click(mouse_x, mouse_y);
            }
            else if (is_leave)
            {
                handle_leave_menu_click(mouse_x, mouse_y);
            }
            else if (is_paused)
            {
                handle_menu_click(mouse_x, mouse_y);
            }
            else
            {
                handle_menu_icon_click(mouse_x, mouse_y);
                handle_user_click(mouse_x, mouse_y);
            }
        });

    );

    if (is_paused == false)
        clk++;
    win.update_screen();
}

bool display_win()
{
    display_winning_message();
    play_music(WIN_MUSIC_DIRECTORY, 0);
    CONTINUE.blink();
    win.update_screen();
    HANDLE(
        QUIT(quit = true);
        KEY_PRESS(q, quit = true);
        LCLICK({
            if (CONTINUE.is_mouse_in(mouse_x, mouse_y))
            {
                update_unlocked_level();
                level.waves_finished = false;
                level_chosen = false;
                play_music(OPENING_MUSIC_DIRECTORY);
                win.fade_out();
                return true;
            }
        });

    );
    return false;
}

void display_lose()
{
    display_losing_message();
    play_music(LOSE_MUSIC_DIRECTORY, 0);
    CONTINUE.blink();
    win.update_screen();
    HANDLE(
        QUIT(quit = true);
        KEY_PRESS(q, quit = true);
        LCLICK({
            if (CONTINUE.is_mouse_in(mouse_x, mouse_y))
            {
                level.waves_finished = false;
                level_chosen = false;
                play_music(OPENING_MUSIC_DIRECTORY);
                win.fade_out();
            }
        });

    );
}

void display_R_S_P()
{
    play_music(R_S_P_MUSIC_DIRECTORY);
    if (clk < 30)
    {
        display_ready_set_plant(START_READY_DIRECTORY);
    }
    else if (clk < 60)
    {
        display_ready_set_plant(START_SET_DIRECTORY);
    }
    else if (clk < 180)
    {
        display_ready_set_plant(START_PLANT_DIRECTORY);
    }
    else
    {
        display_ready_set_plant(START_PLANT_DIRECTORY);
        clk = 0;
        is_game_started = true;
    }
}

void display_all_in_game()
{
    win.clear_renderer();
    display_game_layout();
    if (player.is_choosing_a_plant || player.is_shoveling)
    {
        blink_row_and_col();
    }
    display_game_elements();
    display_game_announce();
    display_chosen_plant();
    handle_movements();
    handle_changes();
    play_music(BACKGROUND_MUSIC_DIRECTORY);
    if (game_characters.zombies.size() && !rand(0, 500))
    {
        play_sound_effect(GROAN_MUSIC_DIRECTORY);
    }
}

void unlock_plant(const int &_plant_type)
{
    display_unlock_plant(NEW_PEASHOOTER_DIRECTORY + _plant_type);
    play_music(NEW_PLANT_MUSIC_DIRECTORY, 0);
    CONTINUE.blink();
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