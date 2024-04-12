#include "draw/render_message.hpp"

extern Level level;
extern Player player;
extern Window win;

/*
Display announcement: Ready - Set - Plant.
Player_name's trip to Plants vs. Zombies
*/
void display_ready_set_plant(const int &image_num)
{
    win.clear_renderer();
    display_game_layout();
    win.draw_png_center(image_num);
    win.show_announcer_text();
}

/*Display announcement: huge wave, final wave*/
void display_game_announce()
{
    level.announcer.display();
}