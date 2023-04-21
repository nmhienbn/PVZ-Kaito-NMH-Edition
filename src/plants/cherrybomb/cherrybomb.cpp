#include "cherrybomb.hpp"
#define CHERRYBOMB_FRAME 5
#define CHERRYBOMB_WIDTH 136
#define CHERRYBOMB_HEIGHT 123

extern int game_state;
extern Map cells;
extern window win;

CherryBomb::CherryBomb(const int &_row, const int &_col)
{
    type = CHERRYBOMB_TYPE;
    row = _row;
    col = _col;
    health = PLANT_HEALTH_LIMIT[CHERRYBOMB_TYPE];
    directory_num = CHERRYBOMB_SHEET_DIRECTORY;
    frame = 0;
    attacked_time = 0;
}
CherryBomb::~CherryBomb()
{
}
/*
@return 'true' if cherrybomb hit the zombie
*/
bool CherryBomb::is_hit_zombie(Zombie &zombie)
{
    int right_limit = cells[row][col].x2 + BLOCK_WIDTH + 10;
    int left_limit = cells[row][col].x1 - BLOCK_WIDTH - 25;
    if (is_in(row - 1, zombie.row, row + 1) &&
        is_in(left_limit, zombie.x_location + ZOMBIE_EXACT_LOCATION, right_limit))
        return true;
    return false;
}

bool CherryBomb::is_blow()
{
    return frame == CHERRYBOMB_FRAME * 22;
}

bool CherryBomb::is_disappeared()
{
    return frame >= CHERRYBOMB_FRAME * all_img[CHERRYBOMB_SHEET_DIRECTORY].n_sheet;
}

void CherryBomb::hit_all_zombies(vector<Zombie> &zombies,
                                 vector<ZombiePart> &zombie_parts)
{
    if (is_blow())
    {
        play_sound_effect(CHERRYBOMB_MUSIC_DIRECTORY);
        for (int j = 0; j < (int)zombies.size();)
            if (!apply_hitting_zombie(zombies, j, zombie_parts))
            {
                j++;
            }
    }
}

/*
Apply cherrybomb explode the zombie. (change zombie into burnt one)
*/
bool CherryBomb::apply_hitting_zombie(vector<Zombie> &zombies, const int &z_ind,
                                      vector<ZombiePart> &zombie_parts)
{
    if (is_hit_zombie(zombies[z_ind]))
    {
        zombie_parts.push_back(ZombiePart(ZOMBIE_BURNT_DIRECTORY, ZOMBIE_DIE_FRAME,
                                          zombies[z_ind].row, zombies[z_ind].x_location,
                                          ZOMBIE_WIDTH, ZOMBIE_HEIGHT));
        zombies.erase(zombies.begin() + z_ind);
        return true;
    }
    return false;
}

/*
Display cherry bomb
*/
void CherryBomb::display(const int &_row)
{
    if (row == _row)
    {
        int sframe = frame / CHERRYBOMB_FRAME;
        int scol = sframe % all_img[directory_num].c_sheet;
        int srow = sframe / all_img[directory_num].c_sheet;
        win.draw_png(directory_num, CHERRYBOMB_WIDTH * scol,
                     CHERRYBOMB_HEIGHT * srow, CHERRYBOMB_WIDTH, CHERRYBOMB_HEIGHT,
                     cells[row][col].x1, cells[row][col].y1 + 5,
                     ELEMENT_WIDTH + 15, ELEMENT_HEIGHT);
        if (attacked_time)
        {
            win.draw_png(blink_of[directory_num], CHERRYBOMB_WIDTH * scol, CHERRYBOMB_HEIGHT * srow,
                         CHERRYBOMB_WIDTH, CHERRYBOMB_HEIGHT,
                         cells[row][col].x1, cells[row][col].y1 + 5,
                         ELEMENT_WIDTH + 15, ELEMENT_HEIGHT);
            if (check_status(game_state, IS_PAUSED) == false)
                attacked_time--;
        }
        if (check_status(game_state, IS_PAUSED) == false)
            ++frame;
    }
}