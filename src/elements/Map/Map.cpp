#include "Map.hpp"
#include "game_param.hpp"

/*
random int in [L;R]
*/
int rand(int L, int R)
{
    if (L > R)
        exit(-10);
    return rd() % (R - L + 1) + L;
}

/*
Check if x in [L; R]
*/
bool is_in(const int &L, const int &x, const int &R)
{
    return L <= x && x <= R;
}

/*
@param x query x-coordinate
@return row corresponding to x. -1 if x is not in playground
*/
int get_block_col(const int &x)
{
    int res = (x - X_UPPER_LEFT) / BLOCK_WIDTH;
    if (!is_in(0, res, HORIZ_BLOCK_COUNT - 1))
        res = -1;
    return res;
}

/*
@param y query y-coordinate
@return column corresponding to y. -1 if y is not in playground
*/
int get_block_row(const int &y)
{
    int res = (y - Y_UPPER_LEFT) / BLOCK_HEIGHT;
    if (!is_in(0, res, VERT_BLOCK_COUNT - 1))
        res = -1;
    return res;
}
