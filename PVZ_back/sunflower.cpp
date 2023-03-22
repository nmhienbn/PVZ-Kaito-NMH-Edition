#include "sunflower.h"

/**
Generate suns from sunflowers not at the same time.
Old version: gen all sun at a time.
**/
void gen_sun_from_all_sunflowers(Elements &elements, Map &cells)
{
    for (auto &sunflower : elements.sunflowers)
        if (--sunflower.sec_for_another_sun <= 0)
        {
            if (sunflower.directory_num == SUNFLOWER_SHEET_DIRECTORY)
            {
                if (sunflower.frame == 0)
                {
                    sunflower.directory_num = SUNFLOWER_HAPPY_DIRECTORY;
                    sunflower.blink_directory_num = SUNFLOWER_HAPPY_BLINK_DIRECTORY;
                }
            }
            else if (sunflower.frame == 13 * SUNFLOWER_F_SHEET)
            {
                gen_sun_from_a_sunflower(elements, sunflower, cells);
            }
        }
        else
        {
            if (sunflower.directory_num == SUNFLOWER_HAPPY_DIRECTORY)
            {
                if (sunflower.frame == 0)
                {
                    sunflower.directory_num = SUNFLOWER_SHEET_DIRECTORY;
                    sunflower.blink_directory_num = SUNFLOWER_SHEET_BLINK_DIRECTORY;
                }
            }
        }
}

/*
Generate sun from the sunflowers
Then add it into vector<Sun>
*/
void gen_sun_from_a_sunflower(Elements &elements, Sunflower &sunflower, Map &cells)
{
    sunflower.sec_for_another_sun = SUN_GEN_SUNFLOWER_CLK_COUNT;
    Sun temp;
    temp.final_col = sunflower.col;
    temp.final_row = sunflower.row;
    temp.y_location = cells[sunflower.row][sunflower.col].y1 - 20;
    temp.wait_seconds = 0;
    elements.suns.push_back(temp);
}
