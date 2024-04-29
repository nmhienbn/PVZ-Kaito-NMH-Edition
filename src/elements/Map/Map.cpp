#include "Map.hpp"
#include "game_param.hpp"
#include "support.hpp"

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

/*Create 5 x 9 tiles
0 1 2 3 4 5 6 7 8
1
2
3
4
*/
Map create_a_collection_of_blocks()
{
    Map result;
    vector<Block> temps;
    Block temp;
    for (int y = 0; y < VERT_BLOCK_COUNT; y++)
    {
        temps.clear();
        for (int x = 0; x < HORIZ_BLOCK_COUNT; x++)
        {
            temp.x1 = X_UPPER_LEFT + (x * BLOCK_WIDTH);
            temp.x2 = temp.x1 + BLOCK_WIDTH;
            temp.y1 = Y_UPPER_LEFT + (y * BLOCK_HEIGHT);
            temp.y2 = temp.y1 + BLOCK_HEIGHT;

            temps.push_back(temp);
        }
        result.push_back(temps);
    }
    return result;
}