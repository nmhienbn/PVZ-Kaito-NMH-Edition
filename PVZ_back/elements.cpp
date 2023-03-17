#include "elements.h"

/*
If is_moving is false, then the zombie must be eating.
Else the zombie can't be eating.
*/
void Zombie::change_zombie_eating_status()
{
    if (is_moving == false)
    {
        if (directory_num == ZOMBIE_SHEET_DIRECTORY)
        {
            frame = 0;
            directory_num = ZOMBIE_EATING_DIRECTORY;
            blink_directory_num = ZOMBIE_EATING_BLINK_DIRECTORY;
            n_sheet = ZOMBIE_EATING_N_SHEET;
            c_sheet = ZOMBIE_EATING_C_SHEET;
        }
    }
    else
    {
        if (directory_num == ZOMBIE_EATING_DIRECTORY)
        {
            frame = 0;
            directory_num = ZOMBIE_SHEET_DIRECTORY;
            blink_directory_num = ZOMBIE_BLINK_SHEET_DIRECTORY;
            n_sheet = ZOMBIE_N_SHEET;
            c_sheet = ZOMBIE_C_SHEET;
        }
    }
}