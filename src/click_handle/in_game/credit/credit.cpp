#include "credit.hpp"
#include <algorithm>

extern int game_state;
extern bool quit;
extern Level level;
extern Player player;
extern window win;

bool cmp(const Zombie &x, const Zombie &y)
{
    return x.y_location < y.y_location;
}

/*
Reset speed to normal, and no paused.
Display welcome player
Display ready-set-plant
Now game is really start
 */
void display_credit()
{
    set_status(game_state, IS_FAST, false);
    set_status(game_state, IS_PAUSED, false);
    win.show_announcer_text(player.name + "\'S TRIP TO PLANTS VS. ZOMBIES", 300);
    vector<Zombie> tmp;
    for (int i = NORMAL_TYPE; i < COUNT_ZOMBIE_TYPE; i++)
    {
        for (int j = 0, _ = (int)(level.wave_zombie_count[i].back() + 1) / 2; j < _; j++)
        {
            Zombie t = Zombie(i, level.level_num);
            t.make_credit();
            tmp.push_back(t);
        }
    }
    sort(tmp.begin(), tmp.end(), cmp);
    for (int i = 0; i <= 1400 - WINDOW_WIDTH; i += 7)
    {
        win.clear_renderer();
        win.draw_bg(level.background_directory, i, 0); //, WINDOW_WIDTH, WINDOW_HEIGHT);
        for (auto &zombie : tmp)
        {
            zombie.display2(i);
        }
        win.show_announcer_text();
        win.update_screen();
        HANDLE(
            QUIT(quit = true; return;);
            LOST_FOCUS(
                set_status(game_state, IS_PAUSED, true);
                Mix_PauseMusic();
                Mix_Pause(-1););)
    }
    for (int i = 0; i <= 60; i++)
    {
        win.clear_renderer();
        win.draw_bg(level.background_directory, 1400 - WINDOW_WIDTH, 0); //, WINDOW_WIDTH, WINDOW_HEIGHT);
        for (auto &zombie : tmp)
        {
            zombie.display2(1400 - WINDOW_WIDTH);
        }
        win.show_announcer_text();
        win.update_screen();
        HANDLE(
            QUIT(quit = true; return;);
            LOST_FOCUS(
                set_status(game_state, IS_PAUSED, true);
                Mix_PauseMusic();
                Mix_Pause(-1););)
    }
    for (int i = 1400 - WINDOW_WIDTH; i >= 0; i -= 7)
    {
        win.clear_renderer();
        win.draw_bg(level.background_directory, i, 0); //, WINDOW_WIDTH, WINDOW_HEIGHT);
        for (auto &zombie : tmp)
        {
            zombie.display2(i);
        }
        win.show_announcer_text();
        win.update_screen();
        HANDLE(
            QUIT(quit = true; return;);
            LOST_FOCUS(
                set_status(game_state, IS_PAUSED, true);
                Mix_PauseMusic();
                Mix_Pause(-1););)
    }
    play_music(R_S_P_MUSIC_DIRECTORY);
    for (int clk = 0; clk <= 180; clk++)
    {
        win.clear_renderer();
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
            set_status(game_state, IS_GAME_STARTED, true);
        }
        win.update_screen();
        HANDLE(
            QUIT(quit = true; return;);
            LOST_FOCUS(
                set_status(game_state, IS_PAUSED, true);
                Mix_PauseMusic();
                Mix_Pause(-1););)
    }
}