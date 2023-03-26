#include "peashooter.h"

/*Updated
For all peashooter: If there is a zombie in that peashooter's row:
Generate a new pea at that peashooter's position.
Old version: all peas are generated at a time.
Updated: all peas are generated not at a time.
*/
void fire_peas(Elements &elements, Map &cells)
{
    for (Peashooter &peashooter : elements.peashooters)
    {
        int row = peashooter.row;
        int col = peashooter.col;
        if (are_there_zombies_in_peashooter_row(peashooter, elements, cells))
        {
            if (peashooter.directory_num == PEASHOOTER_SHEET_DIRECTORY)
            {
                if (peashooter.frame == 0)
                {
                    peashooter.directory_num = PEASHOOTER_ATTACK_DIRECTORY;
                    peashooter.blink_directory_num = PEASHOOTER_ATTACK_BLINK_DIRECTORY;
                }
            }
            else if (peashooter.frame == 32)
            {
                play_sound_effect(FIRE_PEA_MUSIC_DIRECTORY);
                Pea temp(1, row, cells[row][col].x2 - 10);
                elements.peas.push_back(temp);
            }
        }
        else
        {
            if (peashooter.directory_num == PEASHOOTER_ATTACK_DIRECTORY)
            {
                if (peashooter.frame == 0)
                {
                    peashooter.directory_num = PEASHOOTER_SHEET_DIRECTORY;
                    peashooter.blink_directory_num = PEASHOOTER_SHEET_BLINK_DIRECTORY;
                }
            }
        }
    }
}

/*
Check if a peashooter need to attack or not.
Peashooter is attack only if there are some zombies in the row.
Updated: Zombie position > peashooter position
*/
bool are_there_zombies_in_peashooter_row(Peashooter &peashooter, Elements &elements, Map &cells)
{
    for (Zombie zombie : elements.zombies)
        if (peashooter.row == zombie.row &&
            zombie.x_location < ZOMBIE_INIT_X &&
            zombie.x_location > cells[0][peashooter.col].x2 - 160)
            return true;
    return false;
}

/*Updated
Change to sprite sheet
*/
void display_peashooters(window &win, vector<Peashooter> &peashooters, Map &cells, bool is_pause)
{
    for (int i = 0; i < peashooters.size(); i++)
    {
        int col = peashooters[i].col;
        int row = peashooters[i].row;
        int frame = peashooters[i].frame;
        int scol = frame % C_SHEET[peashooters[i].directory_num];
        int srow = frame / C_SHEET[peashooters[i].directory_num];
        win.draw_png(peashooters[i].directory_num, PEASHOOTER_WIDTH * scol, PEASHOOTER_HEIGHT * srow, PEASHOOTER_WIDTH, PEASHOOTER_HEIGHT, cells[row][col].x1, cells[row][col].y1 + 5, ELEMENT_WIDTH, ELEMENT_HEIGHT);
        if (peashooters[i].is_attacked)
        {
            win.draw_png(peashooters[i].blink_directory_num, PEASHOOTER_WIDTH * scol, PEASHOOTER_HEIGHT * srow, PEASHOOTER_WIDTH, PEASHOOTER_HEIGHT, cells[row][col].x1, cells[row][col].y1 + 5, ELEMENT_WIDTH, ELEMENT_HEIGHT);
            peashooters[i].is_attacked--;
        }
        if (is_pause == false)
            if (++peashooters[i].frame >= N_SHEET[peashooters[i].directory_num])
            {
                peashooters[i].frame = 0;
            }
    }
}
