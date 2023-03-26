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
        if (are_there_zombies_in_snowpea_row(snowpea, elements, cells))
        {
            if (snowpea.directory_num == SNOWPEA_SHEET_DIRECTORY)
            {
                if (snowpea.frame == 0)
                {
                    snowpea.directory_num = SNOWPEA_ATTACK_DIRECTORY;
                    snowpea.blink_directory_num = PEASHOOTER_ATTACK_BLINK_DIRECTORY;
                }
            }
            else if (snowpea.frame == 32)
            {
                play_sound_effect(FIRE_PEA_MUSIC_DIRECTORY);
                Pea temp(2, row, cells[row][col].x2 - 10);
                elements.peas.push_back(temp);
                // cout << elements.peas.size() << '\n';
            }
        }
        else
        {
            if (snowpea.directory_num == SNOWPEA_ATTACK_DIRECTORY)
            {
                if (snowpea.frame == 0)
                {
                    snowpea.directory_num = SNOWPEA_SHEET_DIRECTORY;
                    snowpea.blink_directory_num = PEASHOOTER_SHEET_BLINK_DIRECTORY;
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
            zombie.x_location < ZOMBIE_INIT_X &&
            zombie.x_location > cells[0][snowpea.col].x2 - 160)
            return true;
    return false;
}

/*Updated
Change to sprite sheet
*/
void display_snowpeas(window &win, vector<Snowpea> &snowpeas, Map &cells, bool is_pause)
{
    for (int i = 0; i < snowpeas.size(); i++)
    {
        int col = snowpeas[i].col;
        int row = snowpeas[i].row;
        int frame = snowpeas[i].frame;
        int scol = frame % C_SHEET[snowpeas[i].directory_num];
        int srow = frame / C_SHEET[snowpeas[i].directory_num];
        win.draw_png(snowpeas[i].directory_num, PEASHOOTER_WIDTH * scol, PEASHOOTER_HEIGHT * srow, PEASHOOTER_WIDTH, PEASHOOTER_HEIGHT, cells[row][col].x1, cells[row][col].y1 + 5, ELEMENT_WIDTH, ELEMENT_HEIGHT);
        if (snowpeas[i].is_attacked)
        {
            win.draw_png(snowpeas[i].blink_directory_num, PEASHOOTER_WIDTH * scol, PEASHOOTER_HEIGHT * srow, PEASHOOTER_WIDTH, PEASHOOTER_HEIGHT, cells[row][col].x1, cells[row][col].y1 + 5, ELEMENT_WIDTH, ELEMENT_HEIGHT);
            snowpeas[i].is_attacked--;
        }
        if (is_pause == false)
            if (++snowpeas[i].frame >= N_SHEET[snowpeas[i].directory_num])
            {
                snowpeas[i].frame = 0;
            }
    }
}
