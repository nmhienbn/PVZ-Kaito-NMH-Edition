#include "cherrybomb.h"

/*New
 */
void display_cherrybombs(window &win, vector<CherryBomb> &cherrybombs, Map &cells, bool is_pause)
{
    for (auto &cherrybomb : cherrybombs)
    {
        int col = cherrybomb.col;
        int row = cherrybomb.row;
        int frame = cherrybomb.frame / CHERRYBOMB_FRAME;
        int scol = frame % C_SHEET[cherrybomb.directory_num];
        int srow = frame / C_SHEET[cherrybomb.directory_num];
        win.draw_png(cherrybomb.directory_num, CHERRYBOMB_WIDTH * scol, CHERRYBOMB_HEIGHT * srow, CHERRYBOMB_WIDTH, CHERRYBOMB_HEIGHT, cells[row][col].x1, cells[row][col].y1 + 5, ELEMENT_WIDTH, ELEMENT_HEIGHT);
        if (cherrybomb.is_attacked)
        {
            win.draw_png(blink_of[cherrybomb.directory_num], CHERRYBOMB_WIDTH * scol, CHERRYBOMB_HEIGHT * srow, CHERRYBOMB_WIDTH, CHERRYBOMB_HEIGHT, cells[row][col].x1, cells[row][col].y1 + 5, ELEMENT_WIDTH, ELEMENT_HEIGHT);
            cherrybomb.is_attacked--;
        }
        if (is_pause == false)
            ++cherrybomb.frame;
    }
}

/*
@return 'true' if cherrybomb hit the zombie
*/
bool is_cherrybomb_hit_zombie(CherryBomb &cherrybomb, Zombie &zombie, Map &cells)
{
    int right_limit = cells[cherrybomb.row][cherrybomb.col].x2 + BLOCK_WIDTH + 10;
    int left_limit = cells[cherrybomb.row][cherrybomb.col].x1 - BLOCK_WIDTH - 25;
    if (is_in(cherrybomb.row - 1, zombie.row, cherrybomb.row + 1) &&
        is_in(left_limit, zombie.x_location + ZOMBIE_EXACT_LOCATION, right_limit))
        return true;
    return false;
}

void handle_cherrybomb_zombie_encounter(Elements &elements, Map &cells)
{
    for (int i = 0; i < (int)elements.cherrybombs.size(); i++)
    {
        if (elements.cherrybombs[i].frame >= CHERRYBOMB_FRAME * N_SHEET[CHERRYBOMB_SHEET_DIRECTORY])
        {
            cells[elements.cherrybombs[i].row][elements.cherrybombs[i].col].is_planted = false;
            elements.cherrybombs.erase(elements.cherrybombs.begin() + i);
        }
        if (elements.cherrybombs[i].frame == CHERRYBOMB_FRAME * 22)
        {
            play_sound_effect(CHERRYBOMB_MUSIC_DIRECTORY);
            for (int j = 0; j < (int)elements.zombies.size();)
                if (!apply_cherrybomb_hitting_zombie(elements, elements.cherrybombs[i], j, cells))
                {
                    j++;
                }
        }
    }
}

bool apply_cherrybomb_hitting_zombie(Elements &elements, CherryBomb &cherrybomb, int z_ind, Map &cells)
{
    if (is_cherrybomb_hit_zombie(cherrybomb, elements.zombies[z_ind], cells))
    {
        DeadZombie tmp(elements.zombies[z_ind].row, elements.zombies[z_ind].x_location, ZOMBIE_BURNT_DIRECTORY, NULL_DIRECTORY);
        elements.dead_zombies.push_back(tmp);
        elements.zombies.erase(elements.zombies.begin() + z_ind);
        return true;
    }
    return false;
}