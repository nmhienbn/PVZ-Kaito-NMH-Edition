#pragma once
#include "draw/rsdl.hpp"
#include "elements/Map/Map.hpp"
#include "game_param.hpp"
#include "music/music.hpp"

#define BULLET_EXPLODE_TIME 15

enum BulletType
{
    PEA,
    SNOWZPEA,
    BULLET_TYPE_NUM
};

class Bullet
{
public:
    int type;
    int row;
    int x_location, y_location;
    int directory_num;
    int explode;
    int width, height;
    int angle;
    int slow_down_rate;

public:
    Bullet();
    Bullet(int _type, int _row, int _x);

    bool operator<(const Bullet &other) const;
    bool has_exploded();
    bool is_disappeared();
    void display_shadow();

    virtual ~Bullet();
    virtual void move() = 0;
    virtual void display() = 0;
    virtual void crash_effect(){};
    virtual void make_explode(){};
};

void move_bullets(std::vector<Bullet *> &bullets);