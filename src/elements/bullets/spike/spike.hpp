#pragma once
#include "../bullet.hpp"

// Cactus spike: flies like a pea but is allowed to hit flying balloon
// zombies (see BulletVsZombie.cpp). y_bias raises the muzzle when the cactus
// stretches up for balloons.
class Spike : public Bullet
{
public:
    Spike(int _row, int _x, int _y_bias = 0);
    void move();
    void display();
    void crash_effect();
    void make_explode();

private:
    // Authoritative float position (see Pea::x_pos); x_location is the
    // lround()'d copy collision/render consume.
    float x_pos;
};
