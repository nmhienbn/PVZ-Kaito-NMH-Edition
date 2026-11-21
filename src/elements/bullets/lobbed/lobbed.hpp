#pragma once
#include "../bullet.hpp"

// What the pult basket launches.
enum LobbedKind
{
    LOBBED_CABBAGE,
    LOBBED_CORN,
    LOBBED_BUTTER,
    LOBBED_MELON
};

/*
Lobbed shot (cabbage/kernel/melon pults): the projectile rides a fixed
72-tick flight toward the x of the nearest zombie down the lane picked at
launch: x advances linearly while y follows the original's gravity arc
(repvz MOTION_LOBBED: vz0 -11.667 px/tick, g 0.319px/tick^2, ~213 px apex).
Damage is resolved by the shot itself on touchdown (see
land()); the generic pea sweep never scores hits on a flying lobbed shot
(BulletVsZombie ignores type LOBBED until it is spent).
*/
class Lobbed : public Bullet
{
public:
    Lobbed(const int &_kind, const int &_row, const int &_x);
    void move() override;
    void display() override;

private:
    int kind;
    int from_x, from_y;
    int target_x;
    int t = 0;
    int t_total;
    void land(); // splash damage + butter stun at the arc's end
};
