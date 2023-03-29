#include "render_message.h"

/*
Display announcement: Ready - Set - Plant.
Player_name's trip to Plants vs. Zombies
*/
void display_ready_set_plant(window &win, Player &player, int image_num, Level &level)
{
    win.clear_renderer();
    win.draw_bg(level.background_directory);
    win.draw_png(image_num, (WINDOW_WIDTH - READY_WIDTH) >> 1, (WINDOW_HEIGHT - READY_HEIGHT) >> 1, READY_WIDTH, READY_HEIGHT);

    win.draw_png(BLACK_SCREEN_DIRECTORY, 0, 500, WINDOW_WIDTH, 100);
    int w = 0, h = 0;
    string announcement = player.name + "\'s trip to Plants vs. Zombies";
    TTF_SizeText(win.get_font("FreeSans.ttf", WHITE, 24), announcement.c_str(), &w, &h);
    win.show_text(announcement, (WINDOW_WIDTH - w) / 2, 535);
}
/*
Display announcement: huge wave
*/
void display_game_announce(window &win, Level &level)
{
    if (level.announce_directory != NULL_DIRECTORY)
    {
        win.draw_png(level.announce_directory, (WINDOW_WIDTH - HUGE_WAVE_WIDTH) >> 1, (WINDOW_HEIGHT - HUGE_WAVE_HEIGHT) >> 1, HUGE_WAVE_WIDTH, HUGE_WAVE_HEIGHT);
    }
}

/*
If player win, display this
*/
void display_winning_message(window &win, Elements &elements, Map &cells, Level &level)
{
    win.draw_bg(level.background_directory);
    display_game_paused_elements(win, elements, cells, level);
    win.draw_png_scale(WINNING_MESSAGE_DIRECTORY,
                       (WINDOW_WIDTH - WINNING_MESSAGE_WIDTH) / 2, (WINDOW_HEIGHT - WINNING_MESSAGE_HEIGHT) / 2,
                       WINNING_MESSAGE_WIDTH, WINNING_MESSAGE_HEIGHT);
    win.draw_png_scale(CONTINUE_DIRECTORY, CONTINUE.x1, CONTINUE.y1, CONTINUE_WIDTH, CONTINUE_HEIGHT);
}

/*
If player lose, display this.
*/
void display_losing_message(window &win, Elements &elements, Map &cells, Level &level)
{
    win.draw_bg(level.background_directory);
    display_game_paused_elements(win, elements, cells, level);
    win.draw_png_scale(LOSING_MESSAGE_DIRECTORY,
                       (WINDOW_WIDTH - LOSING_MESSAGE_WIDTH) / 2, (WINDOW_HEIGHT - LOSING_MESSAGE_HEIGHT) / 2,
                       LOSING_MESSAGE_WIDTH, LOSING_MESSAGE_HEIGHT);
    win.draw_png_scale(CONTINUE_DIRECTORY, CONTINUE.x1, CONTINUE.y1, CONTINUE_WIDTH, CONTINUE_HEIGHT);
}