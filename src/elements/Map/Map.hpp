#pragma once
#include <vector>
using namespace std;

// Limited by x1-x2 and y1-y2
struct Block
{
    int x1, x2;
    int y1, y2;
    bool is_planted = false;
    bool is_block_zombie = false;
};
// Block[][]: which are limited by x1-x2 and y1-y2
typedef vector<vector<Block>> Map;
int get_block_col(const int &x);
int get_block_row(const int &y);

Map create_a_collection_of_blocks();