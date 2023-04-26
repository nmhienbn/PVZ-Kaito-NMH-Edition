#include "win.hpp"

extern int game_state;
extern bool quit;
extern Level level;
extern Elements game_characters;
extern window win;

/*
If player win, display this
*/
void display_winning_message()
{
}

/*
Check if player won: no wave or zombie left.
*/
bool has_player_won()
{
    if (level.waves_finished && game_characters.zombies.empty() && game_characters.zombie_parts.empty())
        return true;
    return false;
}

/*
Handle when player win the game:
    Display winning message.
    Check if player click on 'continue'.
    Check if player unlocked a new level.
*/
bool display_win()
{
    win.delete_all_texture();
    play_music(WIN_MUSIC_DIRECTORY, 0);
    set_status(game_state, IS_PAUSED, true);
    for (int i = 0;; i++)
    {
        if (i >= 74)
            i = 24;
        for (int j = 1; j <= 5; j++)
        {
            win.draw_png(win_dir + i, 0, 0, 1050, 591);
            if (i >= 23)
            {
                win.draw_png_scale(CONTINUE_DIRECTORY, CONTINUE.x1, CONTINUE.y1, CONTINUE_WIDTH, CONTINUE_HEIGHT);
                CONTINUE.blink();
            }
            win.update_screen();
            HANDLE(
                QUIT(quit = true; return true);
                LCLICK({
                    if (i >= 23 && CONTINUE.is_mouse_in(mouse_x, mouse_y))
                    {
                        update_unlocked_level();
                        level.waves_finished = false;
                        set_status(game_state, IS_LEVEL_CHOSEN, false);
                        set_status(game_state, IS_PAUSED, false);
                        if (check_status(game_state, IS_UNLOCKING_PLANT))
                            play_music(URF_DIRECTORY);
                        win.fade_out();
                        return true;
                    }
                });

            );
        }
    }
    return false;
}