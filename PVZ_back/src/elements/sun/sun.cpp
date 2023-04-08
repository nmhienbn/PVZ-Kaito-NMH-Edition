#include "sun.hpp"
#define INIT_SUN_Y 5

extern Map cells;
extern window win;

/*
Random final row (0-4) and column (0-8) for sun from the sky
Then add it into vector<Sun>
*/
void gen_random_sun_from_sky(vector<Sun> &suns)
{
    Sun temp;
    temp.final_row = rand(0, 4);
    temp.final_col = rand(0, 8);
    temp.y_location = INIT_SUN_Y;
    temp.wait_seconds = 0;
    suns.push_back(temp);
}

/*
Remove the suns that are exist a long time.
*/
void remove_suns(vector<Sun> &suns)
{
    for (int i = 0; i < (int)suns.size(); i++)
        if (suns[i].wait_seconds >= SUN_APPEAR_LIMIT)
            suns.erase(suns.begin() + i);
}

/*
For all the sun:
    If sun is clicked: move it to top left corner. Then remove it.
    Else
        If the sun has not reached the ground: move it.
        Else increase sun-wait time.
*/
void move_suns(vector<Sun> &suns)
{
    for (int i = 0; i < (int)suns.size(); i++)
    {
        if (suns[i].is_clicked)
        {
            suns[i].y_location -= CLICKED_SUN_D * suns[i].y_location / suns[i].x_location;
            suns[i].x_location -= CLICKED_SUN_D;
            if (suns[i].x_location <= 0)
            {
                suns.erase(suns.begin() + i);
            }
            continue;
        }
        int row = suns[i].final_row;
        int col = suns[i].final_col;
        int lower_limit = cells[row][col].y1 + 30;
        if (suns[i].y_location + SUN_DY < lower_limit)
            suns[i].y_location += SUN_DY;
        else
            suns[i].wait_seconds++;
    }
}

/*
Updated: If sun's waited seconds >= SUN_APPEAR_LIMIT * 3 / 4, then change its transparency.
*/
void display_suns(vector<Sun> &suns)
{
    for (auto &sun : suns)
    {
        if (!sun.is_clicked)
        {
            int col = sun.final_col;
            sun.x_location = cells[0][col].x1;
        }
        if (sun.wait_seconds >= SUN_APPEAR_LIMIT * 3 / 4)
            win.draw_png_scale(SUN2_DIRECTORY, sun.x_location, sun.y_location, ELEMENT_WIDTH, ELEMENT_HEIGHT);
        else
            win.draw_png_scale(SUN_DIRECTORY, sun.x_location, sun.y_location, ELEMENT_WIDTH, ELEMENT_HEIGHT);
    }
}