#include "game.h"

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

bool level_chosen = false;
bool quit = false;
bool is_game_started = false;
bool is_paused = false;
int clk = 0;
Level level;
Elements game_characters;
Icons icons;
Player player;
Map cells;
window win(WINDOW_WIDTH, WINDOW_HEIGHT);

int main(int argv, char **args)
{
    init_game(win, level, player, cells);
    init_music();

    while (!quit)
    {
        if (!level_chosen)
        {
            win.clear_renderer();
            display_choosing_level_screen(win, level, player.unlocked_level, level_chosen, quit);
            if (level_chosen)
            {
                load_level(player, level);
                clk = 0;
                is_game_started = false;
            }
        }
        else
        {
            start_level_3(win, player, icons, cells, level, game_characters, clk, quit, is_game_started, level_chosen, is_paused);
        }
    }
    close_music();

    return 0;
}
