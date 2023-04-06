#include "win.hpp"

extern bool level_chosen, quit;
extern Level level;
extern Elements game_characters;
extern window win;

/*
If player win, display this
*/
void display_winning_message()
{
    win.draw_bg(level.background_directory);
    display_game_paused_elements();
    win.draw_png_center(WINNING_MESSAGE_DIRECTORY);
    win.draw_png_scale(CONTINUE_DIRECTORY, CONTINUE.x1, CONTINUE.y1, CONTINUE_WIDTH, CONTINUE_HEIGHT);
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

bool display_win()
{
    display_winning_message();
    play_music(WIN_MUSIC_DIRECTORY, 0);
    CONTINUE.blink();
    win.update_screen();
    HANDLE(
        QUIT(quit = true);
        LCLICK({
            if (CONTINUE.is_mouse_in(mouse_x, mouse_y))
            {
                update_unlocked_level();
                level.waves_finished = false;
                level_chosen = false;
                play_music(OPENING_MUSIC_DIRECTORY);
                win.fade_out();
                return true;
            }
        });

    );
    return false;
}