#include "peashooter.h"

/*Updated
For all peashooter: If there is a zombie in that peashooter's row:
Generate a new pea at that peashooter's position.
Old version: all peas are generated at a time.
Updated: all peas are generated not at a time.
*/
void fire_peas(Elements &elements, Map &map)
{
    for (Peashooter &peashooter : elements.peashooters)
    {
        int row = peashooter.row;
        int col = peashooter.col;
        if (are_there_zombies_in_peashooter_row(peashooter, elements))
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
                Pea temp;
                temp.row = row;
                temp.x_location = map[row][col].x2 - 10;
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
*/
bool are_there_zombies_in_peashooter_row(Peashooter peashooter, Elements &elements)
{
    for (Zombie zombie : elements.zombies)
        if (peashooter.row == zombie.row && zombie.x_location < ZOMBIE_INIT_X)
            return true;
    return false;
}
