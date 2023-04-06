#include "draw/render_message.hpp"

extern Level level;
extern Player player;
extern window win;

/*
Display announcement: Ready - Set - Plant.
Player_name's trip to Plants vs. Zombies
*/
void display_ready_set_plant(const int &image_num)
{
    win.clear_renderer();
    win.draw_bg(level.background_directory);
    win.draw_png_center(image_num);
    win.show_announcer_text();
}
/*
Display announcement: huge wave
*/
void display_game_announce()
{
    win.draw_png_center(level.announce_directory);
}