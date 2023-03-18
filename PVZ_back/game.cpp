#include "game.h"

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

bool quit = false;
int clk = 0;
Level level;
Elements game_characters;
Icons icons;
Player player;
Map map;
bool is_game_started = false;
window win(WINDOW_WIDTH, WINDOW_HEIGHT);

int main(int argv, char **args)
{
    srand(time(NULL));
    init_game(win, level, player, map);

    while (!quit)
    {
        int start_time = SDL_GetTicks();
        if (!is_game_started)
        {
            if (clk < 30)
            {
                display_ready_set_plant(win, START_READY_DIRECTORY);
            }
            else if (clk < 60)
            {
                display_ready_set_plant(win, START_SET_DIRECTORY);
            }
            else if (clk < 90)
            {
                display_ready_set_plant(win, START_PLANT_DIRECTORY);
            }
            else
            {
                clk = 0;
                is_game_started = true;
            }
        }
        if (is_game_started)
        {
            if (has_player_lost(game_characters))
                display_losing_message(win, game_characters, map);
            else if (has_player_won(level, game_characters))
                display_winning_message(win);
            else
            {
                display_game_layout(win, player, icons);
                display_game_elements(win, game_characters, map);
                display_chosen_plant(win, player, icons);
                handle_movements(game_characters, map, clk);
                handle_changes(icons, game_characters, map, level, clk);
            }
        }

        HANDLE(
            QUIT(quit = true);
            KEY_PRESS(q, quit = true);
            LCLICK({
                handle_user_click(player, icons, game_characters, map, mouse_x, mouse_y);
            });

        );

        clk++;
        win.update_screen();

        int now_time = SDL_GetTicks() - start_time;
        int delay_time = max(5, ticks_per_frame - now_time);
        DELAY(delay_time);
    }
    return 0;
}
