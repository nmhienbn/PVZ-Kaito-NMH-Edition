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
    bool has_grave = false;  // night grave stone (blocks planting, source of surprise zombies)
    bool has_crater = false; // doom-shroom crater (blocks planting)
    // Crater heal countdown in ticks (original GridItem counter = 18000cs =
    // 180 s -> 10800 ticks at 60 tps); at 0 the crater closes. 0 = no timer.
    int crater_clk = 0;
    bool is_water = false;   // pool/fog water tile (aquatic plants only)
    bool has_ladder = false; // ladder zombie's ladder: zombies climb over this tile
};
// Doom-shroom crater: full heal time 180 s, "fading" art over the last 90 s
// (original DrawCrater: fading = mGridItemCounter < 9000cs).
const int CRATER_HEAL_TICKS = 10800;
const int CRATER_FADING_TICKS = 5400;
// Block[][]: which are limited by x1-x2 and y1-y2
typedef vector<vector<Block>> Map;
int get_block_col(const int &x);
int get_block_row(const int &x, const int &y);

Map create_a_collection_of_blocks();