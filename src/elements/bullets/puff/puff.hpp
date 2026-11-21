#pragma once
#include "../bullet.hpp"

// Spore fizzle range from the muzzle (repvz Projectile::CheckForCollision,
// Projectile.cpp:244: MOTION_PUFF dies at mProjectileAge >= 75 hundredths of
// a second; at 3.33 px/cs that is ~250 px -> 45 ticks at 60 tps).
constexpr int PUFF_RANGE_PX = 250;

class Puff : public Bullet
{
    int spawn_x;  // range cap: the spore fizzles PUFF_RANGE_PX from the muzzle
    int range_px; // scaredy-shroom shoots its spores across the whole row
    // Authoritative float position (see Pea::x_pos); x_location is the
    // lround()'d copy collision/render consume.
    float x_pos;

public:
    Puff(int _row, int _x, int _range_px = -1);
    void move();
    void display();
    void crash_effect();
    void make_explode();
};
