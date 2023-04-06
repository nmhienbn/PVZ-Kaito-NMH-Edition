#include "unlock_plant.hpp"

extern bool quit, is_unlocking_plant;
extern Level level;
extern Elements game_characters;
extern window win;

/*
If player unlock a new plant, display this
*/
void display_unlock_plant(const int &new_plant_dir)
{
    win.clear_renderer();
    win.draw_png_center(new_plant_dir);
    win.draw_png_scale(CONTINUE_DIRECTORY, CONTINUE.x1, CONTINUE.y1, CONTINUE_WIDTH, CONTINUE_HEIGHT);
}

void unlock_plant(const int &_plant_type)
{
    display_unlock_plant(NEW_PEASHOOTER_DIRECTORY + _plant_type);
    play_music(NEW_PLANT_MUSIC_DIRECTORY, 0);
    CONTINUE.blink();
    win.update_screen();
    HANDLE(
        QUIT(quit = true);
        LCLICK({
            if (CONTINUE.is_mouse_in(mouse_x, mouse_y))
            {
                win.fade_out();
                is_unlocking_plant = false;
                play_music(OPENING_MUSIC_DIRECTORY);
            }
        });

    );
}