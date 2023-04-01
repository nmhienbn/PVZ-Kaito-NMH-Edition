#include "makeLevels.h"

/*
Create 5 x 9 tiles
*/
Map create_a_collection_of_blocks()
{
    Map result;
    vector<Block> temps;
    Block temp;
    for (int y = 0; y < VERT_BLOCK_COUNT; y++)
    {
        temps.clear();
        for (int x = 0; x < HORIZ_BLOCK_COUNT; x++)
        {
            temp.x1 = X_UPPER_LEFT + (x * BLOCK_WIDTH);
            temp.x2 = temp.x1 + BLOCK_WIDTH;
            temp.y1 = Y_UPPER_LEFT + (y * BLOCK_HEIGHT);
            temp.y2 = temp.y1 + BLOCK_HEIGHT;

            temps.push_back(temp);
        }
        result.push_back(temps);
    }
    return result;
}

/*
Initialize game:
    Display loading and starting game.
    Read player data
    Read level data
    Random zombie appear time
    Create block on frontyard (playground)
    Init player's sun
*/
void init_game(window &win, Level &level, Player &player, Map &cells)
{
    display_starting_screen(win);
    read_savedata(player);
    cells = create_a_collection_of_blocks();
}

/*
Check if player has lost: Any zombie go to the house
*/
bool has_player_lost(Elements &elements)
{
    for (auto &zombie : elements.zombies)
    {
        if (zombie.x_location + 70 < X_UPPER_LEFT - 30)
            return true;
    }
    return false;
}

/*
Check if player won: no wave or zombie left.
*/
bool has_player_won(Level &level, Elements &elements)
{
    if (level.waves_finished && elements.zombies.empty() && elements.dead_zombies.empty())
        return true;
    return false;
}

/*
DISPLAY STARTING SCREEN AND CHOOSING LEVEL SCREEN
*/

/*Need update: Fake loading screen
Display starting screen
Updated: Now player can exit game from here.
*/
void display_starting_screen(window &win)
{
    bool game_started = false;
    bool quit = false;
    play_music(OPENING_MUSIC_DIRECTORY);
    while (!quit && !game_started)
    {
        win.draw_png_scale(STARTING_SCREEN_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        TAP_TO_START.blink(win);
        HANDLE(
            QUIT(quit = true; exit(0););
            // KEY_PRESS(q, quit = true);
            LCLICK({
                if (TAP_TO_START.is_mouse_in(mouse_x, mouse_y))
                    game_started = true;
            });

        );

        win.update_screen();
    }
    win.fade_out();
    win.clear_renderer();
}

/*New function (Need update):
Reset all things after a level finished.
*/
void reset_level(Elements &elements, Map &cells, Icons &icons)
{

    elements.peashooters.clear();
    elements.sunflowers.clear();
    elements.walnuts.clear();
    elements.snowpeas.clear();
    elements.cherrybombs.clear();

    elements.suns.clear();
    elements.peas.clear();

    elements.zombies.clear();
    elements.dead_zombies.clear();

    for (int y = 0; y < VERT_BLOCK_COUNT; y++)
    {
        for (int x = 0; x < HORIZ_BLOCK_COUNT; x++)
        {
            cells[y][x].is_planted = false;
        }
    }
    icons = Icons();
}