#include "plants/sunflower/sunflower.hpp"

extern bool is_paused;
extern Map cells;
extern window win;

/**
Generate suns from sunflowers not at the same time.
Old version: gen all sun at a time.
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
            else if (sunflower.frame == 13 * SUNFLOWER_F_SHEET)
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
Generate sun from the sunflowers
Then add it into vector<Sun>
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

/*Updated
Change to sprite sheet
Add blink
*/
void display_sunflowers(vector<Sunflower> &sunflowers)
{
    for (auto &sunflower : sunflowers)
    {
        if (sunflower.directory_num == SUNFLOWER_SHEET_DIRECTORY)
        {
            int col = sunflower.col;
            int row = sunflower.row;
            // win.draw_png_scale(SUNFLOWER_DIRECTORY, cells[row][col].x1 + 9, cells[row][col].y1 + 9, ELEMENT_WIDTH, ELEMENT_HEIGHT);

            int frame = sunflower.frame;
            int scol = frame % C_SHEET[SUNFLOWER_SHEET_DIRECTORY];
            int srow = frame / C_SHEET[SUNFLOWER_SHEET_DIRECTORY];
            win.draw_png(sunflower.directory_num, SUNFLOWER_WIDTH * scol, SUNFLOWER_HEIGHT * srow, SUNFLOWER_WIDTH, SUNFLOWER_HEIGHT, cells[row][col].x1, cells[row][col].y1 - 15, ELEMENT_WIDTH, ELEMENT_HEIGHT);

            if (is_paused == false)
                if (++sunflower.frame >= N_SHEET[SUNFLOWER_SHEET_DIRECTORY])
                {
                    sunflower.frame = 0;
                }
            if (sunflower.is_attacked)
            {
                win.draw_png(blink_of[sunflower.directory_num], SUNFLOWER_WIDTH * scol, SUNFLOWER_HEIGHT * srow, SUNFLOWER_WIDTH, SUNFLOWER_HEIGHT, cells[row][col].x1, cells[row][col].y1 - 15, ELEMENT_WIDTH, ELEMENT_HEIGHT);
                sunflower.is_attacked--;
            }
        }
        else
        {
            int col = sunflower.col;
            int row = sunflower.row;
            // win.draw_png_scale(SUNFLOWER_DIRECTORY, cells[row][col].x1 + 9, cells[row][col].y1 + 9, ELEMENT_WIDTH, ELEMENT_HEIGHT);

            int frame = sunflower.frame / SUNFLOWER_F_SHEET;
            int scol = frame % C_SHEET[SUNFLOWER_HAPPY_DIRECTORY];
            int srow = frame / C_SHEET[SUNFLOWER_HAPPY_DIRECTORY];
            win.draw_png(sunflower.directory_num, SUNFLOWER_H_WIDTH * scol, SUNFLOWER_H_HEIGHT * srow, SUNFLOWER_H_WIDTH, SUNFLOWER_H_HEIGHT, cells[row][col].x1, cells[row][col].y1 - 15, ELEMENT_WIDTH, ELEMENT_HEIGHT);

            if (is_paused == false)
                if (++sunflower.frame >= SUNFLOWER_F_SHEET * N_SHEET[SUNFLOWER_HAPPY_DIRECTORY])
                {
                    sunflower.frame = 0;
                }
            if (sunflower.is_attacked)
            {
                win.draw_png(blink_of[sunflower.directory_num], SUNFLOWER_H_WIDTH * scol, SUNFLOWER_H_HEIGHT * srow, SUNFLOWER_H_WIDTH, SUNFLOWER_H_HEIGHT, cells[row][col].x1, cells[row][col].y1 - 15, ELEMENT_WIDTH, ELEMENT_HEIGHT);
                sunflower.is_attacked--;
            }
        }
    }
}
