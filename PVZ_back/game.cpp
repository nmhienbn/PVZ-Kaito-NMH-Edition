#include "game.h"

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

bool level_chosen = false;
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
    init_music();

    while (!quit)
    {
        win.clear_renderer();
        if (!level_chosen)
        {
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
            start_level_3(win, player, icons, map, level, game_characters, clk, quit, is_game_started, level_chosen);
        }
    }
    close_music();

    return 0;
}
