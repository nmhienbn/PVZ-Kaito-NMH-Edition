#pragma once
#include <vector>
using namespace std;
#define MOWER_WIDTH 70
#define MOWER_HEIGHT 57
#define MOWER_FRAME 5
/*repvz LawnMower::Update (LawnMower.cpp:227-233): the lawn cleaner rolls at
3.33 px/cs, the pool cleaner at 2.5 px/cs; at the clone's 60 ticks/s that is
x5/3 (CS_VEL scaling, see zombie_struct.hpp). Carried as float -- survived
codata: a lone mower needs ~2.5 s to sweep the lane, not the old ~4.5 s.*/
#define MOWER_LAWN_DX (3.33f * (100.0f / 60.0f))
#define MOWER_POOL_DX (2.5f * (100.0f / 60.0f))

enum Mower_status
{
    MOWER_INACTIVE,
    MOWER_ACTIVE,
};

struct Mower
{
    float x; // float: keeps the 3.33 px/cs roll across the 60-tps grid
    int row;
    int frame;
    float rframe = 0.0f; // reanim doc-local frame (replaces `frame` when the 2Pak docs render)
    Mower_status status;
    /*F2 layout-editor transient offsets (px): the display paths add them to
    the drawn anchor (scale rides the rsdl debug registry). Render-only,
    default 0 -> gameplay untouched; the editor zeroes them on close.*/
    int layout_dx = 0, layout_dy = 0;

    Mower();
    Mower(int _x, int _row, int _frame, Mower_status _status);
    void display();
    bool display_reanim(); // 2Pak cleaner docs; false = use the old sheet
    bool active();
};

void init_mower(vector<Mower *> &mowers, int l_row, int r_row);
void display_mowers(const vector<Mower *> &mowers);

/*Vertical anchor of the lane surface under a mower (bottom-anchored): flat
lanes park at cells[row][0].y1 + 97, roof lanes on the tuned per-row ground
plus the slope delta while rolling. Shared by the renderer and the layout
editor's mower rect so both read one source of truth.*/
int mower_ground_y(int x, int row);
bool active_mower(vector<Mower *> &mowers, int row);