#include "credit.hpp"
#include <algorithm>

extern int game_state;
extern bool quit;
extern Level level;
extern Player player;
extern window win;

void review_playground(vector<Zombie> &zombies, const int &i)
{
    win.clear_renderer();
    win.draw_bg(level.background_directory, i, 0); //, WINDOW_WIDTH, WINDOW_HEIGHT);
    for (auto &zombie : zombies)
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

/*
Reset speed to normal, and no paused.
Display welcome player
Display ready-set-plant
Now game is really start
 */
void display_credit()
{
    play_music(ChooseYourSeeds_DIRECTORY);
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
    sort(tmp.begin(), tmp.end(), [](const Zombie &x, const Zombie &y) -> bool
         { return x.y_location < y.y_location; });
    for (int i = 0; i <= 60; i++)
    {
        review_playground(tmp, 0);
    }
    for (int i = 0; i <= 1400 - WINDOW_WIDTH; i += 10)
    {
        review_playground(tmp, i);
    }
    for (int i = 0; i <= 180; i++)
    {
        review_playground(tmp, 1400 - WINDOW_WIDTH);
    }
    for (int i = 1400 - WINDOW_WIDTH; i >= 0; i -= 10)
    {
        review_playground(tmp, i);
    }
    play_sound_effect(R_S_P_MUSIC_DIRECTORY);
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