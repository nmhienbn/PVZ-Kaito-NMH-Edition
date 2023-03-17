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

window win(WINDOW_WIDTH, WINDOW_HEIGHT);
int main(int argv, char **args)
{
    srand(time(NULL));
    init_game(win, level, player, map);

    while (!quit)
    {
        Uint32 start_time = SDL_GetTicks();

        if (has_player_lost(game_characters))
            display_losing_message(win, game_characters, map);
        else if (has_player_won(level, game_characters))
            display_winning_message(win);
        else
        {
            display_game_layout(win, player, icons);
            display_game_elements(win, game_characters, map);
            handle_movements(game_characters, map, clk);
            handle_changes(icons, game_characters, map, level, clk);
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

        Uint32 now_time = SDL_GetTicks() - start_time;
        if (now_time < sec_per_frame)
        {
            DELAY(sec_per_frame - now_time);
        }
    }
    return 0;
}
