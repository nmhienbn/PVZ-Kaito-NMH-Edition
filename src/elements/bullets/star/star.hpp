#pragma once
#include "../bullet.hpp"

// Starfruit star: constant velocity (vx, vy), drifts across lanes as it
// crosses row boundaries (row is re-derived from y). Pops when it leaves the
// board or lands on a zombie (BulletVsZombie.cpp STAR window).
class Star : public Bullet
{
public:
    Star(int _row, int _x, int _y, float _vx, float _vy);
    void move();
    void display();
    void crash_effect();
    void make_explode();

private:
    float vx, vy;      // px/tick; diagonal rays run at non-integer speeds
    float x_pos, y_pos; // authoritative float position; x_location/y_location
                        // are the lround()'d copies collision/render consume
};
