#include "cherrybomb.hpp"
#define CHERRYBOMB_FRAME 5
#define CHERRYBOMB_WIDTH 105
#define CHERRYBOMB_HEIGHT 95
#define CHERRYBOMB_G_WIDTH 105
#define CHERRYBOMB_G_HEIGHT 95

extern int game_state;
extern Map cells;
extern window win;

/*
Cherry Bomb constructor
*/
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

/*
Cherry Bomb Destructor
*/
CherryBomb::~CherryBomb()
{
}
/*
@return 'true' if cherrybomb hit the zombie
*/

/*
@return true if Cherry Bomb can hit the zombie
*/
bool CherryBomb::is_hit_zombie(Zombie &zombie)
{
    int right_limit = cells[row][col].x2 + BLOCK_WIDTH;
    int left_limit = cells[row][col].x1 - BLOCK_WIDTH;
    if (is_in(row - 1, zombie.row, row + 1) &&
        is_in(left_limit, zombie.x_location + ZOMBIE_EXACT_LOCATION + 15, right_limit))
        return true;
    return false;
}

/*
@return true if current frame cherry bomb can explode
*/
bool CherryBomb::is_blow()
{
    return frame == CHERRYBOMB_FRAME * 22;
}

/*
@return if cherry bomb can now disappear
*/
void CherryBomb::disappear()
{
    if (frame >= CHERRYBOMB_FRAME * all_img[CHERRYBOMB_SHEET_DIRECTORY].n_sheet)
    {
        health = 0;
    }
}

/*
Make cherry bomb hit all the zombies of area 3x3
*/
void CherryBomb::hit_all_zombies(vector<Zombie *> &zombies,
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
bool CherryBomb::apply_hitting_zombie(vector<Zombie *> &zombies, const int &z_ind,
                                      vector<ZombiePart> &zombie_parts)
{
    if (is_hit_zombie(*zombies[z_ind]))
    {
        zombie_parts.push_back(ZombiePart(ZOMBIE_BURNT_DIRECTORY, ZOMBIE_DIE_FRAME,
                                          zombies[z_ind]->row, zombies[z_ind]->x_location,
                                          ZOMBIE_WIDTH, ZOMBIE_HEIGHT));
        delete zombies[z_ind];
        zombies.erase(zombies.begin() + z_ind);
        return true;
    }
    return false;
}

/*
Display cherry bomb in row
*/
void CherryBomb::display(const int &_row)
{
    if (row == _row)
    {
        // current frame
        int sframe = frame / CHERRYBOMB_FRAME;
        // current column in source image
        int scol = sframe % all_img[directory_num].c_sheet;
        // current row in source image
        int srow = sframe / all_img[directory_num].c_sheet;
        // Cherry Bomb
        win.draw_png(directory_num, CHERRYBOMB_WIDTH * scol,
                     CHERRYBOMB_HEIGHT * srow, CHERRYBOMB_WIDTH, CHERRYBOMB_HEIGHT,
                     cells[row][col].x1, cells[row][col].y1 + 5,
                     CHERRYBOMB_G_WIDTH, CHERRYBOMB_G_HEIGHT);
        // Blink
        if (attacked_time)
        {
            win.draw_png(blink_of[directory_num], CHERRYBOMB_WIDTH * scol, CHERRYBOMB_HEIGHT * srow,
                         CHERRYBOMB_WIDTH, CHERRYBOMB_HEIGHT,
                         cells[row][col].x1, cells[row][col].y1 + 5,
                         CHERRYBOMB_G_WIDTH, CHERRYBOMB_G_HEIGHT);
            if (check_status(game_state, IS_PAUSED) == false)
                attacked_time--;
        }
        // Next frame
        if (check_status(game_state, IS_PAUSED) == false)
            ++frame;
    }
}