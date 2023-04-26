#include "mower.hpp"
#define MOWER_WIDTH 70
#define MOWER_HEIGHT 57
#define MOWER_FRAME 5
#define MOWER_DX 3

extern int game_state;
extern window win;
extern Map cells;
vector<Mower> mowers;

/*
Init the mower from l_row to r_row
*/
void init_mower(int l_row, int r_row)
{
    mowers.clear();
    for (int row = l_row; row <= r_row; row++)
    {
        Mower tmp(cells[0][0].x1 - 55 - row * 2, row, 0, MOWER_INACTIVE);
        mowers.push_back(tmp);
    }
}

/*
Display the mower inactive/active
*/
void display_mower(const int &_row)
{
    for (int i = 0, _ = mowers.size(); i < _; i++)
        if (mowers[i].row == _row)
        {
            int frame = mowers[i].frame / MOWER_FRAME;
            int scol = frame % all_img[MOWER_DIRECTORY].c_sheet;
            int srow = frame / all_img[MOWER_DIRECTORY].c_sheet;
            win.draw_png(MOWER_DIRECTORY, MOWER_WIDTH * scol, MOWER_HEIGHT * srow, MOWER_WIDTH, MOWER_HEIGHT,
                         mowers[i].x, cells[mowers[i].row][0].y1 + 40, MOWER_WIDTH, MOWER_HEIGHT);
            if (mowers[i].status == MOWER_ACTIVE && check_status(game_state, IS_PAUSED) == false)
            {
                if (++mowers[i].frame >= MOWER_FRAME * all_img[MOWER_DIRECTORY].n_sheet)
                {
                    mowers[i].frame = 0;
                }
            }
        }
}

/*
Handle mower vs zombie collision
*/
void handle_mower_zombie_encounter(vector<Zombie *> &zombies,
                                   vector<ZombiePart> &zombie_parts)
{
    if (check_status(game_state, IS_PAUSED) == true)
        return;
    for (auto &zombie : zombies)
    {
        if (zombie->x_location + ZOMBIE_EXACT_LOCATION < X_UPPER_LEFT - 30)
        {
            active_mower(zombie->row);
        }
    }
    for (int i = 0; i < (int)mowers.size(); i++)
        if (mowers[i].status == MOWER_ACTIVE)
        {
            mowers[i].x += MOWER_DX;

            if (++mowers[i].x >= WINDOW_WIDTH)
            {
                mowers.erase(mowers.begin() + i);
                i--;
                continue;
            }
            for (int j = 0; j < (int)zombies.size();)
            {
                if (zombies[i]->x_location + ZOMBIE_EXACT_LOCATION < X_UPPER_LEFT - 30)
                {
                    active_mower(zombies[i]->row);
                }
                if (!apply_mower_hitting_zombie(zombies, j, mowers[i], zombie_parts))
                {
                    j++;
                }
            }
        }
}

/*
Apply mower explode the zombie. (chang zombie into burnt one)
*/
bool apply_mower_hitting_zombie(vector<Zombie *> &zombies, const int &z_ind,
                                Mower &mower,
                                vector<ZombiePart> &zombie_parts)
{
    if (is_mower_hit_zombie(mower, *zombies[z_ind]))
    {
        zombies[z_ind]->add_zombie_die(zombie_parts);
        if (zombies[z_ind]->type == CONE_TYPE)
            zombie_parts.push_back(ZombiePart(CONE_DROP_DIRECTORY, HEAD_ZOMBIE_FRAME,
                                              zombies[z_ind]->row, zombies[z_ind]->x_location + 80,
                                              HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT));
        if (zombies[z_ind]->type == BUCKET_TYPE)
            zombie_parts.push_back(ZombiePart(BUCKET_DROP_DIRECTORY, HEAD_ZOMBIE_FRAME,
                                              zombies[z_ind]->row, zombies[z_ind]->x_location + 80,
                                              HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT));
        delete zombies[z_ind];
        zombies.erase(zombies.begin() + z_ind);
        return true;
    }
    return false;
}

/*
@return 'true' if mower hit the zombie
*/
bool is_mower_hit_zombie(Mower &mower, Zombie &zombie)
{
    if (zombie.row == mower.row &&
        zombie.x_location <= mower.x)
        return true;
    return false;
}

/*
Return true if can active a mower in given row
*/
bool active_mower(int row)
{
    for (auto &mower : mowers)
    {
        if (mower.row == row && mower.status == MOWER_INACTIVE)
        {
            play_sound_effect(MOWER_MUSIC_DIRECTORY);
            mower.status = MOWER_ACTIVE;
            return true;
        }
    }
    return false;
}

/*
Mower constructor
*/
Mower::Mower()
{
}

/*
Mower constructor with details
*/
Mower::Mower(int _x, int _row, int _frame, Mower_status _status)
{
    x = _x;
    row = _row;
    frame = _frame;
    status = _status;
}
