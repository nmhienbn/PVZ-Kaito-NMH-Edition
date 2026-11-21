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
#include "elements/conveyor/conveyor.hpp"
#include "elements/elements.hpp"
#include "events/anim_viewer/anim_viewer.hpp"
#include "events/dave_scene/dave_scene.hpp"
#include "game_param.hpp"
#include "music/music.hpp"
#include "player/player_data.hpp"
#include <cstdlib>

extern int game_state;
extern bool quit;
extern int clk;
extern Level level;
extern Elements game_characters;
extern Player player;
extern Window win;

const int TIME_FOR_WIN = 180;
int delay_for_win = INF;

int last_time_delete = 600;

// Auto-FPS gate for display-driven animations (declared in game_param.hpp):
// number of fixed-rate logic ticks covered by the current rendered frame.
int logic_ticked = 0;

// Ambient groan cadence (see update_ambient_groans below): base interval is
// 3.0-6.0s at 60 tps before the density divisor; the 1.0s floor keeps plays
// from overlapping on top of each other at high zombie counts.
const int GROAN_BASE_MIN_TICKS = 180;
const int GROAN_BASE_MAX_TICKS = 360;
const int GROAN_MIN_GAP_TICKS = 60;
int groan_timer = GROAN_MIN_GAP_TICKS;
// Debug groan logs: only with PVZ_GROAN_DEBUG=1 (same style as
// PVZ_LOADING_DEBUG in start_game/LoadZomHead.cpp).
static bool groan_debug()
{
    static int enabled = -1;
    if (enabled < 0)
    {
        const char *v = getenv("PVZ_GROAN_DEBUG");
        enabled = (v != NULL && v[0] == '1') ? 1 : 0;
    }
    return enabled == 1;
}
/*
If game has not started:
    display credit.
    0.5s: display "Ready - Set - Plant": 0.5 - 0.5 - 2.

Else: (game has started)
    If player lose: display lose message. Reset level.
    Else if player win:
        on-lawn win effect (display_win), reset level.
        check if is a new plant is unlocked or not.
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

Auto-FPS model: the gameplay branch runs fixed-rate logic ticks on a time
accumulator -- 60 tps normally, 120 in turbo (IS_FAST) -- and draws one full
render per call, presented uncapped (vsync/GPU speed decides the fps). clk
advances once per logic tick, not once per call. The blocking-scene paths
(pause/restart/leave/win/lose/credit) keep their internal 60 fps present
loops (set_frame_cap(FPS) at the top of this function).
*/

// Fixed-timestep accumulator (auto-FPS): leftover time in perf-counter
// units, the timestamp of the previous gameplay frame, and whether the
// previous start_level call reached the gameplay branch (false after a
// blocking scene -- pause/restart/leave/win/lose/credit -- so the gap is
// treated as paused time with a fresh baseline, no catch-up burst).
static long long tick_last_now = -1;
static double tick_acc = 0.0;
static bool prev_call_gameplay = false;

/*One fixed-rate logic tick (was the tail of display_all_in_game): movement,
value changes, turbo safety, music, ambient groans. The gameplay frame driver
in start_level runs this at 60 tps (120 in turbo) on an accumulator, decoupled
from the render rate (auto-FPS).*/
static void gameplay_tick()
{
    // Move:
    handle_movements();
    // Changes:
    handle_changes();
    // Auto disable faster
    auto_disable_turbo();
    // Music and sound effect:
    play_music(BACKGROUND_MUSIC_DIR[level.level_num]);
    // Ambient groans while zombies roam.
    update_ambient_groans();
    // Post-win delay (start_level's has_player_won block): drain at the
    // logic rate, not the render rate (auto-FPS; INF = not running).
    if (delay_for_win != INF)
        delay_for_win--;
}

/*Draw one gameplay frame (was the head of display_all_in_game): the full
scene every frame. The old (clk & 1) turbo draw skip is gone -- turbo now
raises the logic tick rate to 120 tps instead of halving the draws, and the
render rate is uncapped (set_frame_cap(0)).*/
static void render_all_in_game()
{
    win.clear_renderer();
    display_game_layout();
    display_game_announce();
    win.show_announcer_text();
    display_chosen_plant();
    display_turbo_icon();
}

void start_level()
{
    // Blocking scenes pace their internal present loops at 60; the gameplay
    // branch flips to uncapped at its present.
    win.set_frame_cap(FPS);
    const bool prev_was_gameplay = prev_call_gameplay;
    prev_call_gameplay = false;
    bool gameplay_frame = false;

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
            // gameplay_tick() drains the delay at the logic rate (auto-FPS:
            // a per-frame decrement here would tie the 3 s win delay to the
            // uncapped render rate -- 1.25 s at 144 fps).
            if (delay_for_win <= 0)
            {
                delay_for_win = INF;
                // check if is a new plant is unlocked or not.
                bool is_newest_level = (level.level_num == player.unlocked_level);
                // display win message.
                // reset level.
                if (display_win() && is_newest_level)
                {
                    // Code-keyed non-plant awards (1-4 shovel, 1-9 zombie note)
                    // ride on the same pending-award status as plant unlocks.
                    if (dave::award_for_level(level.level_num) != dave::DAVE_AWARD_NONE ||
                        level.level_num == 12)
                        set_status(game_state, IS_UNLOCKING_PLANT, true);
                    for (int i = 0; i < PLANT_COUNT; i++)
                    {
                        if (level.level_num == level_unlock_new_plant[i] - 1)
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
            // Display gameplay -- auto-FPS: logic ticks at a fixed rate (60 tps,
            // 120 in turbo) on an accumulator; the render below presents uncapped
            // (win.set_frame_cap(0)) and vsync/GPU speed decides the fps.
            gameplay_frame = true;
            prev_call_gameplay = true;
            const long long freq = SDL_GetPerformanceFrequency();
            const long long now = SDL_GetPerformanceCounter();
            if (!prev_was_gameplay)
            {
                // Resuming after a blocking scene / pause / first call: treat the
                // gap as paused time -- fresh baseline, no catch-up burst.
                tick_last_now = now;
                tick_acc = 0.0;
            }
            long long elapsed = now - tick_last_now;
            tick_last_now = now;
            if (elapsed > freq / 10) // stall guard (window drag, breakpoint): no backlog
                elapsed = freq / 10;
            const int tps = check_status(game_state, IS_FAST) ? 120 : 60; // turbo = 2x logic
            const double tick_len = (double)freq / tps;
            tick_acc += (double)elapsed;
            logic_ticked = 0;
            int ticks_run = 0;
            while (tick_acc >= tick_len && ticks_run < 5) // catch-up cap: drop backlog beyond 5 ticks
            {
                tick_acc -= tick_len;
                ticks_run++;
                // One logic tick (old order kept: tick first, then clk++, like the
                // old display_all_in_game -> clk++ -> present sequence).
                gameplay_tick();
                // Dave's 1-5 dig phase: gates wave 1 + the bowling belt until the
                // pre-placed peashooters are shoveled; no-op on other levels.
                dave::update_in_game();
                clk++;
                // Cache cleanup rides the tick rate so its cadence stays ~60/s
                // regardless of the (uncapped) render rate.
                win.delete_all_texture_no_use();
                if (--last_time_delete <= 0)
                {
                    last_time_delete = 600;
                    delete_all_sound_effect();
                }
            }
            // Anim gates advance once per logic tick (see game_param.hpp):
            // turbo's 120 tps sets 2 here, keeping anims foot-synced with
            // the doubled movement rate.
            logic_ticked = ticks_run;
            // Draw the frame.
            render_all_in_game();
            // Present uncapped (update_screen at the function tail paces nothing).
            win.set_frame_cap(0);
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
                if (belt.active)
                {
                    // Belt levels: 1-N keys hit the belt slots (pick, and
                    // press again on the dark slot to put it back).
                    if (e.type == SDL_KEYDOWN &&
                        is_in(SDLK_1, e.key.keysym.sym, SDLK_1 + belt.packet_count() - 1))
                        belt.toggle_slot(e.key.keysym.sym - SDLK_1);
                }
                else if (e.type == SDL_KEYDOWN &&
                    is_in(SDLK_1, e.key.keysym.sym, SDLK_1 + (int)player.seed_packets.size() - 1))
                {
                    // The 1-9 keys hit seed *slots*; change_chosen_status
                    // expects the plant *type* behind that slot.
                    int slot = e.key.keysym.sym - SDLK_1;
                    change_chosen_status(player.seed_packets[slot].plant_name);
                }
                KEY_PRESS(s, {
                    if (level.has_shovel)
                    {
                        play_sound_effect(SHOVEL_MUSIC_DIRECTORY);
                        player.is_shoveling ^= 1;
                        remove_chosen_plant();
                    }
                });
            };

            // Space toggles pause (Reflourished-style). Outside the "not
            // paused" guard above so it also unpauses; reuses the same pause
            // machinery as the pause icon click / BACK_TO_GAME button.
            KEY_PRESS(SPACE, {
                if (check_status(game_state, IS_RESTART) == false && check_status(game_state, IS_LEAVE) == false)
                {
                    if (check_status(game_state, IS_PAUSED))
                    {
                        unpause_game();
                    }
                    else
                    {
                        set_status(game_state, IS_PAUSED, true);
                        Mix_PauseMusic();
                        Mix_Pause(-1);
                        play_sound_effect(PAUSE_MUSIC_DIRECTORY);
                    }
                }
            });

            // Debug animation viewer (F1, designer tooling): blocking scene,
            // so the game is frozen exactly where it is (no clk advance, no
            // render) while it is open; audio is muted for the duration and
            // every channel/music state is restored when it closes.
            KEY_PRESS(F1, {
                if (check_status(game_state, IS_RESTART) == false && check_status(game_state, IS_LEAVE) == false)
                {
                    const int nch = Mix_AllocateChannels(-1);
                    std::vector<bool> channel_paused((size_t)nch);
                    for (int ci = 0; ci < nch; ci++)
                        channel_paused[ci] = Mix_Paused(ci) != 0;
                    const bool music_paused = Mix_PausedMusic() != 0;
                    Mix_Pause(-1);
                    Mix_PauseMusic();
                    display_anim_viewer();
                    for (int ci = 0; ci < nch; ci++)
                        if (!channel_paused[ci])
                            Mix_Resume(ci);
                    if (!music_paused)
                        Mix_ResumeMusic();
                }
            });

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

    // Move to the next frame. Gameplay advanced clk inside its tick loop
    // (once per logic tick); the blocking-scene paths keep the old once-per-
    // call bump, and never while paused.
    if (gameplay_frame == false && check_status(game_state, IS_PAUSED) == false)
        clk++;

    // Present. The blocking scenes already presented through their internal
    // loops at the 60 fps cap (restored at the top of this function); the
    // gameplay branch set the cap to 0 (uncapped) before reaching here.
    win.update_screen();
}

/*Ambient zombie groans (repvz Zombie::UpdatePlaying, Zombie.cpp:4577-4609):
each zombie rolls a "brains"-style groan on a re-arming timer whose pass
chance scales with 1/zombieCount. Board-level equivalent here: one timer for
the whole lawn; when it fires the odds were already folded into the cadence,
so the groan always plays.
Cadence (60 tps): base rand(180, 360) ticks (3.0-6.0s, the original's
500-1500cs re-arm scaled down for its 1/N pass gate) divided by
1 + (count-1)*0.15 -- more zombies on the lawn, tighter spacing. A 60-tick
(1s) floor caps overlap once the divisor shrinks intervals below the length
of the groan samples themselves.
Exclusions mirror the original: boss fight (ZOMBIE_BOSS guard) and the
level-award/win window (mBoard->HasLevelAwardDropped() -> waves_finished).
No per-level exceptions for whack/bowling/pool: the original groans wherever
zombies walk, so do we.
Debug log (gated on PVZ_GROAN_DEBUG=1, same style as PVZ_LOADING_DEBUG):
INFO lines are the harness hook for the density-spacing assertions.*/

// Number of zombies deciding the cadence: live zombies on the board.
static int groan_zombie_count()
{
    return (int)game_characters.zombies.size();
}

// Next groan delay in ticks for the current board population.
static int groan_interval_ticks(const int &zombie_count)
{
    float density_scale = 1.0f + (zombie_count - 1) * 0.15f;
    int interval = (int)(rand(GROAN_BASE_MIN_TICKS, GROAN_BASE_MAX_TICKS) / density_scale);
    return interval < GROAN_MIN_GAP_TICKS ? GROAN_MIN_GAP_TICKS : interval;
}

void update_ambient_groans()
{
    // Boss fight (original: ZOMBIE_BOSS never groans) and the win window
    // (original: groans stop once the level award drops) stay silent; on
    // other levels the groans keep rolling while zombies walk (whack-a-
    // zombie included -- the original makes no exception there).
    if (level.is_special(LevelSpecial::Boss) || level.waves_finished ||
        game_characters.zombies.empty())
    {
        groan_timer = GROAN_MIN_GAP_TICKS; // re-arm: first groan 1s into the next wave
        return;
    }
    if (--groan_timer > 0)
        return;
    const int zombie_count = groan_zombie_count();
    play_sound_effect(GROAN_MUSIC_DIRECTORY);
    groan_timer = groan_interval_ticks(zombie_count);
    if (groan_debug())
        printf("INFO: groan clk=%d zombies=%d next_in=%d ticks\n", clk,
               zombie_count, groan_timer);
}