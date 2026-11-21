/*
Wall-nut Bowling projectile (level 1-5): thrown with a packet picked from the
belt, the nut rolls right along its lane from wherever it was dropped on the
lawn. On zombie contact a Wall-nut deals the original armor-aware damage
(Plant::UpdateBowling: 90 to a bare zombie or a rolling screen door, 45 capped
to the helm of cone/bucket/football, 20 capped to the shield of
newspaper/ladder -- so cone/pail/football fall in 2/3/3 hits, plain ones in 1),
then ricochets into an adjacent playable lane; the nut survives every hit and
only leaves via the right edge. An Explode-o-nut blows up (3x3) on first
contact instead.
*/
#pragma once
#include <vector>
using namespace std;

struct RollingNut
{
    int plant;         // WALLNUT_TYPE or CHERRYBOMB_TYPE (cherrybomb = Explode-o-nut)
    int row;           // collision lane
    float x, y;        // center location (y drifts during a ricochet)
    float vx = 2.5f, vy = 0.0f;
    int angle = 0;       // rolling spin (rendering)
    int vdir = 0;        // vertical ricochet direction: +1 down, -1 up, 0 = no hit yet
    int explode_clk = 0; // > 0: mid-explosion, ticks of blast visual left
    int boom_row = 0, boom_col = 0; // tile the blast visual is anchored to
};

extern vector<RollingNut> rolling_nuts;

void reset_rolling_nuts();
void launch_rolling_nut(const int &plant, const int &row, const int &x);
void update_rolling_nuts();
void display_rolling_nuts();
