#include "sun.h"

/*
Random final row (0-4) and column (0-8) for sun from the sky
Then add it into vector<Sun>
*/
void gen_random_sun_from_sky(Elements &elements)
{
    srand(time(NULL));
    Sun temp;
    temp.final_row = rand() % 5;
    temp.final_col = rand() % 9;
    temp.y_location = INIT_SUN_Y;
    temp.wait_seconds = 0;
    elements.suns.push_back(temp);
}

/*
Remove the suns that are exist a long time.
*/
void remove_suns(Elements &elements)
{
    for (int i = 0; i < elements.suns.size(); i++)
        if (elements.suns[i].wait_seconds >= 100)
            elements.suns.erase(elements.suns.begin() + i);
}

/*
For all the sun:
    + If the sun has not reached the ground: move it.
    + Else increase sun-wait time.
*/
void move_suns(vector<Sun> &suns, Map &map)
{
    for (int i = 0; i < suns.size(); i++)
    {
        int row = suns[i].final_row;
        int col = suns[i].final_col;
        int lower_limit = map[row][col].y1 + 30;
        if (suns[i].y_location + SUN_DY < lower_limit)
            suns[i].y_location += SUN_DY;
        else
            suns[i].wait_seconds++;
    }
}