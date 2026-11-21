#pragma once
#include "../bullet.hpp"

class SnowzPea : public Bullet
{
public:
    SnowzPea(int _row, int _x);
    void move();
    void display();
    void crash_effect();
    void make_explode();

private:
    // Authoritative float position (see Pea::x_pos); x_location is the
    // lround()'d copy collision/render consume.
    float x_pos;
};