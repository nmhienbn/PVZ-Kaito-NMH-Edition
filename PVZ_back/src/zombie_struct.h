#pragma once
#include "music.h"
#include "rsdl.hpp"
#include "gameInfo.h"
#include "Map.h"

enum ZombieType
{
    NORMAL_TYPE,
    CONE_TYPE,
    BUCKET_TYPE,

    COUNT_ZOMBIE_TYPE,
};

struct DeadZombie
{
    int row;
    int x_location;
    int body = ZOMBIE_DIE_DIRECTORY;
    int head = ZOMBIE_HEAD_DIRECTORY;
    int frame = 0;
    bool is_cold = false;

    DeadZombie(int _r, int _x, bool _cold);
    DeadZombie(int _r, int _x, int _body, int _head);
};
/*
@param row(int): The row of the zombie
@param x_location(int): The begin column of zombie
@param health(int): remaining health of zombie.
@param is_moving(bool): true if zombie is moving, otherwise false.
@param directory_num: order of string that is the path to image.
@param blinkdirectory_num: order of string that is the path to blink image.
@param n_sheet: number of sprites.
@param c_sheet: number of column in sprite sheet
@param directory(string): decide what image to be render.
*/
struct Zombie
{
    int type;

    int row, x_location;
    int health;
    bool is_moving;
    int directory_num;
    int frame;
    int is_attacked, cold_time;
    int bite_time;

    Zombie();
    Zombie(int _type, int level_num);
    Zombie(int _type, int level_num, int _directory_num);
    Zombie(int _type, int level_num, int _row, int _x);
    void change_zombie_eating_status();
    void determine_appearance(vector<DeadZombie> &dead_zombies);
    bool decrease_health(vector<DeadZombie> &dead_zombies);
    bool operator<(const Zombie &other) const;
};