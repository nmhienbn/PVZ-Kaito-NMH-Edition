#include "level.hpp"
#include "draw/render_elements.hpp"
#include "draw/render_message.hpp"
#include "elements/elements_actions.hpp"
#include "player_click.hpp"

#include "../credit/credit.hpp"
#include "../lose/lose.hpp"
#include "../main_menu/leave_menu.hpp"
#include "../pause_menu/game_pause.hpp"
#include "../restart_menu/restart_menu.hpp"
#include "../turbo/turbo.hpp"
#include "../win/win.hpp"

#include "elements/Level/Level.hpp"
#include "elements/Player/Player.hpp"
#include "elements/elements.hpp"
#include "music/music.hpp"
#include "player/player_data.hpp"

extern int game_state;
extern bool quit;
extern int clk;
extern Level level;
extern Elements game_characters;
extern Player player;
extern Window win;

const int TIME_FOR_WIN = 180;
int delay_for_win = INF;

/*
If game has not started:
    display credit.
    0.5s: display "Ready - Set - Plant": 0.5 - 0.5 - 2.

Else: (game has started)
    If player lose: display lose message. Reset level.
    Else if player win:
        display win message.
        check if is a new plant is unlocked or not.
        reset level.
    Else if player click restart: handle restart.
    Else if player click leave: handle leave.
    Else if game is paused: handle pause.
    Else display gameplay.

Handle key: 1,2,3,4,5,6: plant; s: shovel
Handle window focus lose:
Handle user event:
    Quit event.
    Key to win. //For developer.
    Key to choose plant, shovel.
    Lose keyboard focus: pause game
    Player click: Player click on game.
    Mouse event: Volume change.

Move to next frame (clk++)
Display frame: no display even frame if IS_FAST
*/
void start_level()
{
    if (!check_status(game_state, IS_GAME_STARTED))
    {
        // display credit.
        // 0.5s: display "Ready - Set - Plant": 0.5 - 0.5 - 2.
        display_credit();
        clk = 1;
        return;
    }
    else
    {
        if (has_player_lost())
        {
            // display lose message. Reset level.
            display_lose();
            return;
        }
        if (has_player_won())
        {
            if (delay_for_win == INF)
            {
                delay_for_win = TIME_FOR_WIN;
            }
            if (--delay_for_win <= 0)
            {
                delay_for_win = INF;
                // check if is a new plant is unlocked or not.
                bool is_newest_level = (level.level_num == player.unlocked_level);
                // display win message.
                // reset level.
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
        }
        if (check_status(game_state, IS_RESTART) == true)
        {
            // handle restart.
            display_game_restart();
        }
        else if (check_status(game_state, IS_LEAVE) == true)
        {
            // handle leave.
            display_game_leave();
        }
        else if (check_status(game_state, IS_PAUSED) == true)
        {
            // handle pause
            display_game_pause();
        }
        else
        {
            // display gameplay
            display_all_in_game();
        }
        HANDLE_SDL_EVENT(
            QUIT();

            // Key to win. //For developer.
            KEY_TO_WIN({
                level.waves_finished = 1;
                game_characters.zombies.clear();
                game_characters.zombie_parts.clear();
            });

            // Key to choose plant, shovel.
            if (check_status(game_state, IS_PAUSED) == false) {
                if (e.type == SDL_KEYDOWN && is_in(SDLK_1, e.key.keysym.sym, SDLK_1 + PLANT_COUNT - 1))
                {
                    change_chosen_status(e.key.keysym.sym - SDLK_1);
                }
                KEY_PRESS(s, {
                    play_sound_effect(SHOVEL_MUSIC_DIRECTORY);
                    player.is_shoveling ^= 1;
                    remove_chosen_plant();
                });
            };

            // Lose keyboard focus: pause game
            LOST_FOCUS(
                set_status(game_state, IS_PAUSED, true);
                Mix_PauseMusic();
                Mix_Pause(-1););

            /* Player click:
                If player is_restart: handle_restart_menu_click
                Else player is_leave: handle_leave_menu_click
                Else player is_paused: handle_leave_menu
                Menu icon and menu click.
                Turbo icon to speed up the game.
                Player click on game.
                */
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
                    handle_pause_icon_click(mouse_x, mouse_y);
                    handle_turbo_icon_click(mouse_x, mouse_y);
                    handle_user_click(mouse_x, mouse_y);
                }
            });

        );
        // Mouse event: Volume change.
        if (check_status(game_state, IS_PAUSED) == true)
        {
            handle_volume_change();
        }
    }

    // Move to next frame
    if (check_status(game_state, IS_PAUSED) == false)
        clk++;

    // Display frame: no display even frame if IS_FAST
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
        // no display even frame if IS_FAST
        display_game_layout();
        display_game_announce();
        win.show_announcer_text();
        display_chosen_plant();
        display_turbo_icon();
    }
    else
    {
        // update all elements' state
        display_game_elements();
    }
    // Move:
    handle_movements();
    // Changes:
    handle_changes();
    // Auto disable faster
    auto_disable_turbo();
    // Music and sound effect:
    play_music(BACKGROUND_MUSIC_DIR[level.level_num]);
    if (game_characters.zombies.size() && !rand(0, 500))
    {
        play_sound_effect(GROAN_MUSIC_DIRECTORY);
    }
}