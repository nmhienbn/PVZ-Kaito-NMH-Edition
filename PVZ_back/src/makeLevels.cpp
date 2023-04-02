#include "makeLevels.h"

extern bool quit;
extern Level level;
extern Elements game_characters;
extern Player player;
extern Map cells;
extern window win;

/*
Initialize game:
    Display loading and starting game.
    Read player data
    Read level data
    Random zombie appear time
    Create block on frontyard (playground)
    Init player's sun
*/
void init_game()
{
    display_starting_screen();
    read_savedata();
    cells = create_a_collection_of_blocks();
}

/*
Check if player has lost: Any zombie go to the house
*/
bool has_player_lost()
{
    for (auto &zombie : game_characters.zombies)
    {
        if (zombie.x_location + 70 < X_UPPER_LEFT - 30)
            return true;
    }
    return false;
}

/*
Check if player won: no wave or zombie left.
*/
bool has_player_won()
{
    if (level.waves_finished && game_characters.zombies.empty() && game_characters.dead_zombies.empty())
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
void display_starting_screen()
{
    bool game_started = false;
    bool quit = false;
    play_music(OPENING_MUSIC_DIRECTORY);
    while (!quit && !game_started)
    {
        win.draw_png_scale(STARTING_SCREEN_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        TAP_TO_START.blink();
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
