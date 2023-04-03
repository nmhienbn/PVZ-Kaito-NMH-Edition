#include "render_message.h"

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
If player unlock a new plant, display this
*/
void display_unlock_plant(const int &new_plant_dir)
{
    win.clear_renderer();
    win.draw_png_center(new_plant_dir);
    win.draw_png_scale(CONTINUE_DIRECTORY, CONTINUE.x1, CONTINUE.y1, CONTINUE_WIDTH, CONTINUE_HEIGHT);
}