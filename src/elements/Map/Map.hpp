#pragma once
#include <random>
#include <chrono>
#include <ctime>
using namespace std;

static mt19937 rd(chrono::high_resolution_clock::now().time_since_epoch().count());

int rand(int L, int R);
bool is_in(const int &L, const int &x, const int &R);

// Limited by x1-x2 and y1-y2
struct Block
{
    int x1, x2;
    int y1, y2;
    bool is_planted = false;
};
// Block[][]: which are limited by x1-x2 and y1-y2
typedef vector<vector<Block>> Map;
int get_block_col(const int &x);
int get_block_row(const int &y);