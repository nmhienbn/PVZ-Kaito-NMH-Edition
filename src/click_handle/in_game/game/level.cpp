#include "level.hpp"

extern int game_state;
extern bool quit;
extern int clk;
extern Level level;
extern Elements game_characters;
extern Player player;
extern window win;

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
Handle window focus lose:
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
    if (check_status(game_state, IS_GAME_STARTED) == false)
    {
        display_credit();
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
                        set_status(game_state, IS_UNLOCKING_PLANT, true);
                    }
                }
            }
            return;
        }
        if (check_status(game_state, IS_RESTART) == true)
        {
            display_game_restart();
        }
        else if (check_status(game_state, IS_LEAVE) == true)
        {
            display_game_leave();
        }
        else if (check_status(game_state, IS_PAUSED) == true)
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
            if (check_status(game_state, IS_PAUSED) == false) {
                if (e.type == SDL_KEYDOWN && is_in(SDLK_1, e.key.keysym.sym, SDLK_5))
                {
                    change_chosen_status(e.key.keysym.sym - SDLK_1, player.is_choosing_a_plant);
                }
                KEY_PRESS(s, {
                    play_sound_effect(SHOVEL_MUSIC_DIRECTORY);
                    player.is_shoveling ^= 1;
                    remove_chosen_plant();
                });
            };

            LOST_FOCUS(
                set_status(game_state, IS_PAUSED, true);
                Mix_PauseMusic();
                Mix_Pause(-1););

            LCLICK({
                if (check_status(game_state, IS_RESTART) == true)
                {
                    handle_restart_menu_click(mouse_x, mouse_y);
                }
                else if (check_status(game_state, IS_LEAVE) == true)
                {
                    handle_leave_menu_click(mouse_x, mouse_y);
                }
                else if (check_status(game_state, IS_PAUSED) == true)
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

    if (check_status(game_state, IS_PAUSED) == false)
        clk++;
    if (check_status(game_state, IS_PAUSED) == true || check_status(game_state, IS_FAST) == false || (clk & 1))
    {
        win.update_screen();
    }
}

/*
Handle all game: graphic, move, value changes
*/
void display_all_in_game()
{
    // Graphics:
    win.clear_renderer();
    if (check_status(game_state, IS_FAST) == false || (!(clk & 1)))
    {
        display_game_layout();
        display_game_announce();
        win.show_announcer_text();
        display_chosen_plant();
        display_turbo_icon();
    }
    else
    {
        display_game_elements();
    }
    // Move:
    handle_movements();
    // Changes:
    handle_changes();
    // Auto disable faster
    auto_disable_turbo();
    // Music and sound effect:
    play_music(BACKGROUND_MUSIC_DIRECTORY);
    if (game_characters.zombies.size() && !rand(0, 500))
    {
        play_sound_effect(GROAN_MUSIC_DIRECTORY);
    }
}