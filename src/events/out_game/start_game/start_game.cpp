#include "start_game.hpp"

extern bool quit;
extern Level level;
extern Elements game_characters;
extern Player player;
extern Map cells;
extern window win;

/*Initialize game:
    Display loading and starting game.
    Read player data
    Create block on frontyard (playground)
*/
void init_game()
{
    play_music(OPENING_MUSIC_DIRECTORY);
    display_starting_screen();
    read_savedata();
    cells = create_a_collection_of_blocks();
}

/*
Display starting screen.
Updated: Now player can exit game from here.
Handle player click on TAP_TO_START
*/
void display_starting_screen()
{
    play_music(OPENING_MUSIC_DIRECTORY);
    bool game_started = false;
    bool quit = false;
    while (!quit && !game_started)
    {
        // Change color when mouse over TAP_TO_START
        int _x = 0, _y = 0;
        SDL_GetMouseState(&_x, &_y);
        if (TAP_TO_START.is_mouse_in(_x, _y))
        {
            win.draw_png_scale(STARTING_SCREEN_CLICKED_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        }
        else
        {
            win.draw_png_scale(STARTING_SCREEN_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        }
        HANDLE_SDL_EVENT(
            QUIT();
            // Handle player click on TAP_TO_START
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
