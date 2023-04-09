#include "level.hpp"

bool is_fast = false;

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
    0.5s: display "Ready"
    0.5s: display "Set"
    2s: display "Plant!"
    Note that any mouse click has no effect.

Else: (game has started)
    If player lose: display lose message. Reset level.
    Else if player win:
        display win message.
        check if is a new plant is unlocked or not.
        reset level.
    Else if player click restart: handle restart.
    Else if player click main menu: handle main menu.
    Else if game is paused: handle pause.
    Else display gameplay.

Handle key: 1,2,3,4,5: plant; s: shovel
Handle user click:
    Quit event.
    Key to win. //For developer.
    If player is_restart: handle_restart_menu_click
    Else player is_leave: handle_leave_menu_click
    Else player is_paused: handle_leave_menu
    Menu icon and menu click.
    Turbo icon to speed up the game.
    Player click on game.

Move to next frame (clk++)
*/
void start_level()
{
    if (is_game_started == false)
    {
        display_R_S_P();
        HANDLE(QUIT(quit = true; return;))
        clk++;
        win.update_screen();
        return;
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
                    if (level.level_num == level_unlock_new_plant[i] - 1 || level.level_num == 12)
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
        HANDLE(
            QUIT(quit = true; return;);
            KEY_TO_WIN({
                level.waves_finished = 1;
                game_characters.zombies.clear();
                game_characters.dead_zombies.clear();
            });
            KEY_PRESS(1, {
                change_chosen_status(0, player.is_choosing_a_plant);
            });
            KEY_PRESS(2, {
                change_chosen_status(1, player.is_choosing_a_plant);
            });
            KEY_PRESS(3, {
                change_chosen_status(2, player.is_choosing_a_plant);
            });
            KEY_PRESS(4, {
                change_chosen_status(3, player.is_choosing_a_plant);
            });
            KEY_PRESS(5, {
                change_chosen_status(4, player.is_choosing_a_plant);
            });
            KEY_PRESS(s, {
                play_sound_effect(SHOVEL_MUSIC_DIRECTORY);
                player.is_shoveling = true;
                remove_chosen_plant();
            });

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
                    handle_turbo_icon_click(mouse_x, mouse_y);
                    handle_user_click(mouse_x, mouse_y);
                }
            });

        );
    }

    if (is_paused == false)
        clk++;
    if (is_paused == true || is_fast == false || (clk & 1))
        win.update_screen();
}

/*
Reset speed to normal, and no paused.
Display welcome player
Display ready-set-plant
Now game is really start
 */
void display_R_S_P()
{
    is_fast = false;
    is_paused = false;
    play_music(R_S_P_MUSIC_DIRECTORY);
    win.show_announcer_text(player.name + "\'S TRIP TO PLANTS VS. ZOMBIES", 10);
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

/*
Handle all game: graphic, move, value changes
*/
void display_all_in_game()
{
    // Graphics:
    win.clear_renderer();
    display_game_layout();
    display_game_announce();
    win.show_announcer_text();
    display_chosen_plant();
    display_turbo_icon();
    // Move:
    handle_movements();
    // Changes:
    handle_changes();
    // Music and sound effect:
    play_music(BACKGROUND_MUSIC_DIRECTORY);
    if (game_characters.zombies.size() && !rand(0, 500))
    {
        play_sound_effect(GROAN_MUSIC_DIRECTORY);
    }
}