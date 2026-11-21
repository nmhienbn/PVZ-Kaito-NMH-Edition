#pragma once
#include "../bullet.hpp"
#include <set>

/*
Fume-shroom spore cloud: a slow cloud that passes straight through every
zombie in its path (shields included -- door/newspaper give no cover)
and fades out about 4 tiles from the muzzle. Each zombie takes the hit
at most once per cloud.
*/
class Fume : public Bullet
{
    int spawn_x;
    std::set<void *> hit_already;

public:
    Fume(int _row, int _x);
    void move();
    void display();
    void crash_effect();
    void make_explode(); // the cloud disperses at the end of its range
    /*True when the cloud reaches 'zombie' for the first time and bites.
    Caller handles the zombie's death.*/
    bool try_hit(class Zombie &zombie);
};
