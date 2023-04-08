#include "cherrybomb.hpp"
#define CHERRYBOMB_FRAME 5
#define CHERRYBOMB_WIDTH 136
#define CHERRYBOMB_HEIGHT 123

extern bool is_paused;
extern Map cells;
extern window win;

/*
@return 'true' if cherrybomb hit the zombie
*/
bool is_cherrybomb_hit_zombie(CherryBomb &cherrybomb, Zombie &zombie)
{
    int right_limit = cells[cherrybomb.row][cherrybomb.col].x2 + BLOCK_WIDTH + 10;
    int left_limit = cells[cherrybomb.row][cherrybomb.col].x1 - BLOCK_WIDTH - 25;
    if (is_in(cherrybomb.row - 1, zombie.row, cherrybomb.row + 1) &&
        is_in(left_limit, zombie.x_location + ZOMBIE_EXACT_LOCATION, right_limit))
        return true;
    return false;
}

/*
For all zombies and hit them by cherrybombs if they are in that area.
*/
void handle_cherrybomb_zombie_encounter(vector<CherryBomb> &cherrybombs,
                                        vector<Zombie> &zombies,
                                        vector<DeadZombie> &dead_zombies)
{
    for (int i = 0; i < (int)cherrybombs.size(); i++)
    {
        if (cherrybombs[i].frame >= CHERRYBOMB_FRAME * N_SHEET[CHERRYBOMB_SHEET_DIRECTORY])
        {
            cells[cherrybombs[i].row][cherrybombs[i].col].is_planted = false;
            cherrybombs.erase(cherrybombs.begin() + i);
        }
        if (cherrybombs[i].frame == CHERRYBOMB_FRAME * 22)
        {
            play_sound_effect(CHERRYBOMB_MUSIC_DIRECTORY);
            for (int j = 0; j < (int)zombies.size();)
                if (!apply_cherrybomb_hitting_zombie(zombies, j, cherrybombs[i], dead_zombies))
                {
                    j++;
                }
        }
    }
}

/*
Apply cherrybomb explode the zombie. (chang zombie into burnt one)
*/
bool apply_cherrybomb_hitting_zombie(vector<Zombie> &zombies, const int &z_ind,
                                     CherryBomb &cherrybomb,
                                     vector<DeadZombie> &dead_zombies)
{
    if (is_cherrybomb_hit_zombie(cherrybomb, zombies[z_ind]))
    {
        DeadZombie tmp(zombies[z_ind].row, zombies[z_ind].x_location, ZOMBIE_BURNT_DIRECTORY, NULL_DIRECTORY);
        dead_zombies.push_back(tmp);
        zombies.erase(zombies.begin() + z_ind);
        return true;
    }
    return false;
}

/*
Display cherry bomb
*/
void display_cherrybombs(vector<CherryBomb> &cherrybombs)
{
    for (auto &cherrybomb : cherrybombs)
    {
        int col = cherrybomb.col;
        int row = cherrybomb.row;
        int frame = cherrybomb.frame / CHERRYBOMB_FRAME;
        int scol = frame % C_SHEET[cherrybomb.directory_num];
        int srow = frame / C_SHEET[cherrybomb.directory_num];
        win.draw_png(cherrybomb.directory_num, CHERRYBOMB_WIDTH * scol,
                     CHERRYBOMB_HEIGHT * srow, CHERRYBOMB_WIDTH, CHERRYBOMB_HEIGHT,
                     cells[row][col].x1, cells[row][col].y1 + 5,
                     ELEMENT_WIDTH + 15, ELEMENT_HEIGHT);
        if (cherrybomb.is_attacked)
        {
            win.draw_png(blink_of[cherrybomb.directory_num], CHERRYBOMB_WIDTH * scol, CHERRYBOMB_HEIGHT * srow,
                         CHERRYBOMB_WIDTH, CHERRYBOMB_HEIGHT,
                         cells[row][col].x1, cells[row][col].y1 + 5,
                         ELEMENT_WIDTH + 15, ELEMENT_HEIGHT);
            cherrybomb.is_attacked--;
        }
        if (is_paused == false)
            ++cherrybomb.frame;
    }
}