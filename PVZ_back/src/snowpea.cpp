#include "snowpea.h"

/*Updated
For all snowpea: If there is a zombie in that snowpea's row:
Generate a new pea at that snowpea's position.
Old version: all peas are generated at a time.
Updated: all peas are generated not at a time.
*/
void fire_snowz_peas(Elements &elements, Map &cells)
{
    for (Snowpea &snowpea : elements.snowpeas)
    {
        int row = snowpea.row;
        int col = snowpea.col;
        if (snowpea.directory_num == SNOWPEA_ATTACK_DIRECTORY && snowpea.frame == 32 * SNOWPEA_FRAME)
        {
            play_sound_effect(FIRE_PEA_MUSIC_DIRECTORY);
            Pea temp(2, row, cells[row][col].x2 - 10);
            elements.peas.push_back(temp);
        }
        if (are_there_zombies_in_snowpea_row(snowpea, elements, cells))
        {
            if (snowpea.directory_num == SNOWPEA_SHEET_DIRECTORY)
            {
                if (snowpea.frame == 0)
                {
                    snowpea.directory_num = SNOWPEA_ATTACK_DIRECTORY;
                }
            }
        }
        else
        {
            if (snowpea.directory_num == SNOWPEA_ATTACK_DIRECTORY)
            {
                if (snowpea.frame == 0)
                {
                    snowpea.directory_num = SNOWPEA_SHEET_DIRECTORY;
                }
            }
        }
    }
}

/*
Check if a snowpea need to attack or not.
Snowpea is attack only if there are some zombies in the row.
Updated: Zombie position > snowpea position
*/
bool are_there_zombies_in_snowpea_row(Snowpea &snowpea, Elements &elements, Map &cells)
{
    for (Zombie zombie : elements.zombies)
        if (snowpea.row == zombie.row &&
            is_in(cells[0][snowpea.col].x2 - 140, zombie.x_location, ZOMBIE_INIT_X - 70))
            return true;
    return false;
}

/*Updated
Change to sprite sheet
*/
void display_snowpeas(window &win, vector<Snowpea> &snowpeas, Map &cells, bool is_pause)
{
    for (auto &snowpea : snowpeas)
    {
        int col = snowpea.col;
        int row = snowpea.row;
        int frame = snowpea.frame / SNOWPEA_FRAME;
        int scol = frame % C_SHEET[snowpea.directory_num];
        int srow = frame / C_SHEET[snowpea.directory_num];
        win.draw_png(snowpea.directory_num, PEASHOOTER_WIDTH * scol, PEASHOOTER_HEIGHT * srow, PEASHOOTER_WIDTH, PEASHOOTER_HEIGHT, cells[row][col].x1, cells[row][col].y1 + 5, ELEMENT_WIDTH, ELEMENT_HEIGHT);
        if (snowpea.is_attacked)
        {
            win.draw_png(blink_of[snowpea.directory_num], PEASHOOTER_WIDTH * scol, PEASHOOTER_HEIGHT * srow, PEASHOOTER_WIDTH, PEASHOOTER_HEIGHT, cells[row][col].x1, cells[row][col].y1 + 5, ELEMENT_WIDTH, ELEMENT_HEIGHT);
            snowpea.is_attacked--;
        }
        if (is_pause == false)
            if (++snowpea.frame >= SNOWPEA_FRAME * N_SHEET[snowpea.directory_num])
            {
                snowpea.frame = 0;
            }
    }
}
