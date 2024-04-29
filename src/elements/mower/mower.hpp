#pragma once
#include <vector>
using namespace std;
#define MOWER_WIDTH 70
#define MOWER_HEIGHT 57
#define MOWER_FRAME 5
#define MOWER_DX 3

enum Mower_status
{
    MOWER_INACTIVE,
    MOWER_ACTIVE,
};

struct Mower
{
    int x, row;
    int frame;
    Mower_status status;

    Mower();
    Mower(int _x, int _row, int _frame, Mower_status _status);
    void display();
    bool active();
};

void init_mower(vector<Mower *> &mowers, int l_row, int r_row);
void display_mowers(const vector<Mower *> &mowers);
bool active_mower(vector<Mower *> &mowers, int row);