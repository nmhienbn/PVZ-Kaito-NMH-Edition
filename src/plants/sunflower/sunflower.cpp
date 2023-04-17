#include "sunflower.hpp"
#define SUNFLOWER_WIDTH 174
#define SUNFLOWER_HEIGHT 172
#define SUNFLOWER_FRAME 2

#define SUNFLOWER_G_WIDTH 120
#define SUNFLOWER_G_HEIGHT 120

extern int game_state;
extern Map cells;
extern window win;

/**
Generate suns from sunflowers not at the same time.
**/
void gen_sun_from_all_sunflowers(vector<Sunflower> &sunflowers, vector<Sun> &suns)
{
    for (auto &sunflower : sunflowers)
        if (--sunflower.sec_for_another_sun <= 0)
        {
            if (sunflower.directory_num == SUNFLOWER_SHEET_DIRECTORY)
            {
                if (sunflower.frame == 0)
                {
                    sunflower.directory_num = SUNFLOWER_HAPPY_DIRECTORY;
                }
            }
            else if (sunflower.frame == 30 * SUNFLOWER_FRAME)
            {
                gen_sun_from_a_sunflower(sunflower, suns);
            }
        }
        else
        {
            if (sunflower.directory_num == SUNFLOWER_HAPPY_DIRECTORY)
            {
                if (sunflower.frame == 0)
                {
                    sunflower.directory_num = SUNFLOWER_SHEET_DIRECTORY;
                }
            }
        }
}

/*
Generate sun from the sunflowers.
Init waiting time of sun.
Then add it into vector<Sun>.
Reset time to have next sun.
*/
void gen_sun_from_a_sunflower(Sunflower &sunflower, vector<Sun> &suns)
{
    sunflower.sec_for_another_sun = SUN_GEN_SUNFLOWER_CLK_COUNT;
    Sun temp;
    temp.final_col = sunflower.col;
    temp.final_row = sunflower.row;
    temp.y_location = cells[sunflower.row][sunflower.col].y1 - 20;
    temp.wait_seconds = 0;
    suns.push_back(temp);
}

/*
Display sunflowers
*/
void display_sunflowers(vector<Sunflower> &sunflowers, const int &_row)
{
    for (auto &sunflower : sunflowers)
        if (sunflower.row == _row)
        {
            if (sunflower.frame >= SUNFLOWER_FRAME * all_img[SUNFLOWER_SHEET_DIRECTORY].n_sheet)
            {
                sunflower.frame = 0;
            }
            int col = sunflower.col;
            int row = sunflower.row;

            int frame = sunflower.frame / SUNFLOWER_FRAME;
            int scol = frame % all_img[SUNFLOWER_SHEET_DIRECTORY].c_sheet;
            int srow = frame / all_img[SUNFLOWER_SHEET_DIRECTORY].c_sheet;
            win.draw_png(sunflower.directory_num, SUNFLOWER_WIDTH * scol, SUNFLOWER_HEIGHT * srow,
                         SUNFLOWER_WIDTH, SUNFLOWER_HEIGHT,
                         cells[row][col].x1 - 22, cells[row][col].y1 - 28,
                         SUNFLOWER_G_WIDTH, SUNFLOWER_G_HEIGHT);

            if (check_status(game_state, IS_PAUSED) == false)
                if (++sunflower.frame >= SUNFLOWER_FRAME * all_img[SUNFLOWER_SHEET_DIRECTORY].n_sheet)
                {
                    sunflower.frame = 0;
                }
            if (sunflower.is_attacked)
            {
                win.draw_png(blink_of[sunflower.directory_num],
                             SUNFLOWER_WIDTH * scol, SUNFLOWER_HEIGHT * srow,
                             SUNFLOWER_WIDTH, SUNFLOWER_HEIGHT,
                             cells[row][col].x1 - 22, cells[row][col].y1 - 28,
                             SUNFLOWER_G_WIDTH, SUNFLOWER_G_HEIGHT);
                if (check_status(game_state, IS_PAUSED) == false)
                    sunflower.is_attacked--;
            }
        }
}
