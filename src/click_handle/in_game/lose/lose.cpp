#include "lose.hpp"

extern int game_state;
extern bool quit;
extern Level level;
extern Elements game_characters;
extern window win;

/*
If player lose, display this.
*/
void display_losing_message()
{
    set_status(game_state, IS_PAUSED, true);
    win.draw_bg(level.background_directory);
    display_game_paused_elements();
    win.draw_png_center(LOSING_MESSAGE_DIRECTORY);
    win.draw_png_scale(CONTINUE_DIRECTORY, CONTINUE.x1, CONTINUE.y1, CONTINUE_WIDTH, CONTINUE_HEIGHT);
}

/*
Check if player has lost: Any zombie go to the house
*/
bool has_player_lost()
{
    for (const auto &zombie : game_characters.zombies)
    {
        if (zombie->x_location + ZOMBIE_EXACT_LOCATION < X_UPPER_LEFT - BLOCK_WIDTH)
            return true;
    }
    return false;
}

/*
Handle when player lose:
    Display losing message.
    Check if player click 'continue'.
    Update some variables.
*/
void display_lose()
{
    // Display losing message.
    display_losing_message();
    play_music(LOSE_MUSIC_DIRECTORY, 0);
    CONTINUE.blink();
    win.update_screen();
    HANDLE_SDL_EVENT(
        QUIT(quit = true);
        // Check if player click 'continue'.
        LCLICK({
            if (CONTINUE.is_mouse_in(mouse_x, mouse_y))
            {
                // Update some variables.
                level.waves_finished = false;
                set_status(game_state, IS_LEVEL_CHOSEN, false);
                set_status(game_state, IS_PAUSED, false);
                play_music(OPENING_MUSIC_DIRECTORY);
                win.fade_out();
            }
        });

    );
}
