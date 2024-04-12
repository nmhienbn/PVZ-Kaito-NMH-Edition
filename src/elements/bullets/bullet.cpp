#include "bullet.hpp"
#include <algorithm>

extern Map cells;
extern Window win;

Bullet::Bullet()
{
}

/*Construct new pea*/
Bullet::Bullet(int _type, int _row, int _x)
{
    if (_type >= BULLET_TYPE_NUM)
        throw "Invalid bullet type";
    type = _type;
    row = _row;
    x_location = _x;
    explode = INF;
    slow_down_rate = 1;
}

Bullet::~Bullet()
{
}

/*
x_location > other.x_location
*/
bool Bullet::operator<(const Bullet &other) const
{
    return x_location > other.x_location;
}

/*
check if a pea has exploded or not.
initialized pea will explode after BULLET_EXPLODE_TIME.
*/
bool Bullet::has_exploded()
{
    return explode <= BULLET_EXPLODE_TIME;
}

/*
Check if a pea disappeared
A pea will disappear if it exploded or moved out the screen
*/
bool Bullet::is_disappeared()
{
    int right_bound = WINDOW_WIDTH;
    return --explode == 0 || x_location > right_bound;
}

/*
Display the shadow under the pea
*/
void Bullet::display_shadow()
{
    win.draw_png(BULLET_SHADOW_DIRECTORY, x_location, cells[row][0].y2 - 13, 21, 9);
}

void move_bullets(std::vector<Bullet *> &bullets)
{
    for (auto &bullet : bullets)
    {
        bullet->move();
    }
}