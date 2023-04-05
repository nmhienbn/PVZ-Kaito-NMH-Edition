#include "plants/peashooter/peashooter.hpp"

extern bool is_paused;
extern Map cells;
extern window win;

/*Updated
For all peashooter: If there is a zombie in that peashooter's row:
Generate a new pea at that peashooter's position.
Old version: all peas are generated at a time.
Updated: all peas are generated not at a time.
*/
void fire_peas(vector<Peashooter> &peashooters, vector<Zombie> &zombies, vector<Pea> &peas)
{
    for (Peashooter &peashooter : peashooters)
    {
        int row = peashooter.row;
        int col = peashooter.col;
        if (peashooter.directory_num == PEASHOOTER_ATTACK_DIRECTORY && peashooter.frame == 32 * PEASHOOTER_FRAME)
        {
            play_sound_effect(FIRE_PEA_MUSIC_DIRECTORY);
            Pea temp(1, row, cells[row][col].x2 - 10);
            peas.push_back(temp);
        }
        if (are_there_zombies_in_peashooter_row(peashooter, zombies))
        {
            if (peashooter.directory_num == PEASHOOTER_SHEET_DIRECTORY)
            {
                if (peashooter.frame == 0)
                {
                    peashooter.directory_num = PEASHOOTER_ATTACK_DIRECTORY;
                }
            }
        }
        else
        {
            if (peashooter.directory_num == PEASHOOTER_ATTACK_DIRECTORY)
            {
                if (peashooter.frame == 0)
                {
                    peashooter.directory_num = PEASHOOTER_SHEET_DIRECTORY;
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
bool are_there_zombies_in_peashooter_row(Peashooter &peashooter, vector<Zombie> &zombies)
{
    for (Zombie zombie : zombies)
        if (peashooter.row == zombie.row &&
            is_in(cells[0][peashooter.col].x2 - 140, zombie.x_location, ZOMBIE_INIT_X - 70))
            return true;
    return false;
}

/*Updated
Change to sprite sheet
*/
void display_peashooters(vector<Peashooter> &peashooters)
{
    for (auto &peashooter : peashooters)
    {
        int col = peashooter.col;
        int row = peashooter.row;
        int frame = peashooter.frame / PEASHOOTER_FRAME;
        int scol = frame % C_SHEET[peashooter.directory_num];
        int srow = frame / C_SHEET[peashooter.directory_num];
        win.draw_png(peashooter.directory_num, PEASHOOTER_WIDTH * scol, PEASHOOTER_HEIGHT * srow, PEASHOOTER_WIDTH, PEASHOOTER_HEIGHT, cells[row][col].x1, cells[row][col].y1 + 5, ELEMENT_WIDTH, ELEMENT_HEIGHT);
        if (peashooter.is_attacked)
        {
            win.draw_png(blink_of[peashooter.directory_num], PEASHOOTER_WIDTH * scol, PEASHOOTER_HEIGHT * srow, PEASHOOTER_WIDTH, PEASHOOTER_HEIGHT, cells[row][col].x1, cells[row][col].y1 + 5, ELEMENT_WIDTH, ELEMENT_HEIGHT);
            peashooter.is_attacked--;
        }
        if (is_paused == false)
            if (++peashooter.frame >= PEASHOOTER_FRAME * N_SHEET[peashooter.directory_num])
            {
                peashooter.frame = 0;
            }
    }
}
