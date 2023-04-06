#include "lose.hpp"

extern bool level_chosen, quit;
extern Level level;
extern Elements game_characters;
extern window win;

/*
If player lose, display this.
*/
void display_losing_message()
{
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
    for (auto &zombie : game_characters.zombies)
    {
        if (zombie.x_location + 70 < X_UPPER_LEFT - 30)
            return true;
    }
    return false;
}

void display_lose()
{
    display_losing_message();
    play_music(LOSE_MUSIC_DIRECTORY, 0);
    CONTINUE.blink();
    win.update_screen();
    HANDLE(
        QUIT(quit = true);
        LCLICK({
            if (CONTINUE.is_mouse_in(mouse_x, mouse_y))
            {
                level.waves_finished = false;
                level_chosen = false;
                play_music(OPENING_MUSIC_DIRECTORY);
                win.fade_out();
            }
        });

    );
}
