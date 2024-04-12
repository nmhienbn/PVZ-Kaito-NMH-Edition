#include "mower.hpp"

extern int game_state;
extern Window win;
extern Map cells;

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

void Mower::display()
{
    // current frame
    int sframe = frame / MOWER_FRAME;
    // current column in source image
    int scol = sframe % all_img[MOWER_DIRECTORY].c_sheet;
    // current row in source image
    int srow = sframe / all_img[MOWER_DIRECTORY].c_sheet;
    // display
    win.draw_png(MOWER_DIRECTORY, MOWER_WIDTH * scol, MOWER_HEIGHT * srow, MOWER_WIDTH, MOWER_HEIGHT,
                 x, cells[row][0].y1 + 40, MOWER_WIDTH, MOWER_HEIGHT);
    // move to next frame
    if (status == MOWER_ACTIVE && check_status(game_state, IS_PAUSED) == false)
    {
        if (++frame >= MOWER_FRAME * all_img[MOWER_DIRECTORY].n_sheet)
        {
            frame = 0;
        }
    }
}

bool Mower::active()
{
    if (status == MOWER_INACTIVE)
    {
        status = MOWER_ACTIVE;
        play_sound_effect(MOWER_MUSIC_DIRECTORY);
        return true;
    }
    return false;
}

/*
Init the mower from l_row to r_row
*/
void init_mower(vector<Mower *> &mowers, int l_row, int r_row)
{
    mowers.assign(VERT_BLOCK_COUNT, nullptr);
    for (int row = l_row; row <= r_row; row++)
    {
        mowers[row] = new Mower(cells[0][0].x1 - 55 - row * 2, row, 0, MOWER_INACTIVE);
    }
}

/*
Display the mowers (both inactive and active)
*/
void display_mowers(const vector<Mower *> &mowers)
{
    for (auto &mower : mowers)
        if (mower != nullptr)
            mower->display();
}

/*
Return true if can active a mower in given row
*/
bool active_mower(vector<Mower *> &mowers, int row)
{
    if (mowers[row] != nullptr)
        return mowers[row]->active();
    return false;
}