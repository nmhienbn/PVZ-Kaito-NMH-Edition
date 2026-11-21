#include "Map.hpp"
#include "elements/Level/Level.hpp" // roof_tile_dy
#include "game_param.hpp"
#include "support.hpp"

/*
@param x query x-coordinate
@return column corresponding to x. -1 if x is not in playground
(repvz PixelToGridX, Board.cpp:9137: x < LAWN_XMIN -> -1, else 80 px columns)
*/
int get_block_col(const int &x)
{
    if (x < X_UPPER_LEFT)
        return -1;
    int res = (x - X_UPPER_LEFT) / BLOCK_WIDTH;
    if (!is_in(0, res, HORIZ_BLOCK_COUNT - 1))
        res = -1;
    return res;
}

/*
@param x, y query coordinate
@return row corresponding to (x, y). -1 if the point is not in playground
Roof branch (repvz PixelToGridY, Board.cpp:9162): the original subtracts
(4 - gridX) * 20 on the sloped half and bands from LAWN_YMIN (80), i.e. its
click bands sit 10 px above the tiles GridToPixelY paints (which subtracts
(5 - gridX) * 20 from tile base LAWN_YMIN-10) -- the bottom 10-px strip of a
sloped tile clicks into the row below, and the bottom sloped row's tail
falls off the band table entirely. The clone clicks where it DRAWS: subtract
the full tile offset (roof_tile_dy, the same fn the plant draw uses) and
band from the painted tile tops (ROOF_Y_UPPER_LEFT = 70), so every lit pixel
of a roof tile maps back to its own row. The y < LAWN_YMIN (80) guard stays,
as in the original.
*/
int get_block_row(const int &x, const int &y)
{
    if (get_block_col(x) == -1 || y < LAWN_YMIN_CLONE)
        return -1;
    int yy = y;
    if (GRID_ROOF)
    {
        yy -= roof_tile_dy(get_block_col(x));
        int res = (yy - ROOF_Y_UPPER_LEFT) / ROOF_BLOCK_HEIGHT;
        if (!is_in(0, res, GRID_ROWS - 1))
            res = -1;
        return res;
    }
    int res = (yy - GRID_Y_TOP) / GRID_BLOCK_H;
    if (!is_in(0, res, GRID_ROWS - 1))
        res = -1;
    return res;
}

/*Create the tile grid with the current level's geometry (5x9 classic, 6x9
pool/fog, or the 5x9 roof with 85-px rows). load_level calls this whenever
the row geometry changes. Tiles stay flat per row: the roof slope is a
render/placement offset (roof_tile_dy), mirroring the original where
GridToPixelY(8, row) is the lane base zombies walk on.*/
Map create_a_collection_of_blocks()
{
    Map result;
    vector<Block> temps;
    Block temp;
    for (int y = 0; y < GRID_ROWS; y++)
    {
        temps.clear();
        for (int x = 0; x < HORIZ_BLOCK_COUNT; x++)
        {
            temp.x1 = X_UPPER_LEFT + (x * BLOCK_WIDTH);
            temp.x2 = temp.x1 + BLOCK_WIDTH;
            temp.y1 = GRID_Y_TOP + (y * GRID_BLOCK_H);
            temp.y2 = temp.y1 + GRID_BLOCK_H;

            temps.push_back(temp);
        }
        result.push_back(temps);
    }
    return result;
}
