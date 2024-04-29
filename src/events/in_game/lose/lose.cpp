#include "lose.hpp"
#include "draw/render_elements.hpp"
#include "elements/Level/Level.hpp"
#include "elements/button/button.hpp"
#include "elements/elements.hpp"
#include "elements/mower/mower.hpp"
#include "music/music.hpp"
#include "player/player_data.hpp"

extern int game_state;
extern bool quit;
extern Level level;
extern Elements game_characters;
extern Window win;
static bool has_display_lost = false;

/*
If player lose, display this.
*/
void display_losing_message()
{
    set_status(game_state, IS_PAUSED, true);
    for (int clk = 1; clk <= 120; clk++)
    {
        win.draw_bg(level.background_directory);
        display_game_paused_elements();
        win.draw_png_center(LOSING_MESSAGE_DIRECTORY, double(clk) / 120);
        win.update_screen();
        HANDLE_SDL_EVENT(QUIT(););
    }
    win.draw_png_center(LOSING_MESSAGE_DIRECTORY);
    CONTINUE.display_with_text(BUTTON_DIRECTORY, "Continue", 20, RGB(0, 196, 0), RGB(43, 44, 58));
    has_display_lost = true;
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
    play_music(LOSE_MUSIC_DIRECTORY, 0);
    if (!has_display_lost)
        display_losing_message();
    win.update_screen();
    while (has_display_lost)
    {
        HANDLE_SDL_EVENT(
            QUIT();
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
                has_display_lost = false;
            });

        );
    }
}
