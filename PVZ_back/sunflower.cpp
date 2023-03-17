#include "sunflower.h"

/**
Generate suns from sunflowers not at the same time.
Old version: gen all sun at a time.
**/
void gen_sun_from_all_sunflowers(Elements &elements, Map &map)
{
    for (int i = 0; i < elements.sunflowers.size(); i++)
        if (--elements.sunflowers[i].sec_for_another_sun == 0)
        {
            gen_sun_from_a_sunflower(elements, elements.sunflowers[i], map);
        }
}

/*
Generate sun from the sunflowers
Then add it into vector<Sun>
*/
void gen_sun_from_a_sunflower(Elements &elements, Sunflower sunflower, Map &map)
{
    sunflower.sec_for_another_sun = SUN_GEN_SUNFLOWER_CLK_COUNT;
    Sun temp;
    temp.final_col = sunflower.col;
    temp.final_row = sunflower.row;
    temp.y_location = map[sunflower.row][sunflower.col].y1 - 20;
    temp.wait_seconds = 0;
    elements.suns.push_back(temp);
}
