#include "unlock_plant.hpp"

extern int game_state;
extern bool quit;
extern Level level;
extern Elements game_characters;
extern Window win;

/*
If player unlock a new plant, display this
*/
void display_unlock_plant(const int &new_plant_dir)
{
    win.clear_renderer();
    win.draw_png_center(new_plant_dir);
    win.draw_png_height_scaled(CONTINUE_DIRECTORY, CONTINUE.x1, CONTINUE.y1, CONTINUE_WIDTH);
}

/*
Display and handle player click on unlocking plant screen.
*/
void unlock_plant(const int &_plant_type)
{
    display_unlock_plant(NEW_PEASHOOTER_DIRECTORY + _plant_type);
    play_music(NEW_PLANT_MUSIC_DIRECTORY, 0);
    CONTINUE.blink();
    win.update_screen();
    HANDLE_SDL_EVENT(
        QUIT();
        LCLICK({
            if (CONTINUE.is_mouse_in(mouse_x, mouse_y))
            {
                play_music(OPENING_MUSIC_DIRECTORY);
                win.fade_out();
                set_status(game_state, IS_UNLOCKING_PLANT, false);
            }
        });

    );
}