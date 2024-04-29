#include "win.hpp"
#include "elements/Level/Level.hpp"
#include "elements/button/button.hpp"
#include "elements/elements.hpp"

extern int game_state;
extern bool quit;
extern Level level;
extern Elements game_characters;
extern Window win;

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
    Display winning message. (Loop infinite 24-74)
    Check if player click on 'continue'. (After 23th winning frame)
    Check if player unlocked a new level.
*/
bool display_win()
{
    win.delete_all_texture();
    play_music(WIN_MUSIC_DIRECTORY, 0);
    set_status(game_state, IS_PAUSED, true);
    for (int i = 0;; i++)
    {
        // back to first frame (infinite loop)
        if (i >= 74)
            i = 24;
        for (int j = 1; j <= 5; j++)
        {
            win.draw_png(win_dir + i, 0, 0, 1050, 591);
            // Display continue After 23th winning frame
            if (i >= 23)
            {
                CONTINUE.display_with_text(BUTTON_DIRECTORY, "Continue", 20, RGB(0, 196, 0), RGB(43, 44, 58));
            }
            win.update_screen();
            HANDLE_SDL_EVENT(
                QUIT();
                LCLICK({
                    // Handle continue click After 23th winning frame
                    if (i >= 23 && CONTINUE.is_mouse_in(mouse_x, mouse_y))
                    {
                        update_unlocked_level();
                        level.waves_finished = false;
                        set_status(game_state, IS_LEVEL_CHOSEN, false);
                        set_status(game_state, IS_PAUSED, false);
                        if (check_status(game_state, IS_UNLOCKING_PLANT) == false)
                            play_music(OPENING_MUSIC_DIRECTORY);
                        win.fade_out();
                        return true;
                    }
                });

            );
        }
    }
    return false;
}