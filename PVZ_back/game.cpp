#include "game.h"

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

bool quit = false;
long int clk = 0;
Level level;
Elements game_characters;
Icons icons;
Player player;
Map map;

window win(WINDOW_WIDTH, WINDOW_HEIGHT);
int main(int argv, char **args)
{
    init_game(win, level, player, map);

    while (!quit)
    {

        if (has_player_lost(game_characters))
            display_losing_message(win);
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
        DELAY(CLK_LENGTH);
    }
    return 0;
}
