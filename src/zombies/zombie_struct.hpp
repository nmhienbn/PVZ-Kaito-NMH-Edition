#pragma once
#include "music/music.hpp"
#include "draw/rsdl.hpp"
#include "game_stats.hpp"
#include "elements/Map/Map.hpp"

#define ZOMBIE_FRAME 3
#define ZOMBIE_EXACT_LOCATION 70
#define ZOMBIE_WIDTH 166
#define ZOMBIE_HEIGHT 144

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
class Zombie
{
private:
    int health;
    int directory_num;
    int frame;
    int dir_width, dir_height;
    int nZom;

public:
    int type;
    int row, x_location, y_location;
    bool is_moving;
    int attacked_time, cold_time;
    int bite_time;
    int next_step_time;

    Zombie();
    Zombie(int _type, int level_num);
    void change_zombie_eating_status();
    void determine_appearance(vector<DeadZombie> &dead_zombies);
    bool decrease_health(vector<DeadZombie> &dead_zombies);
    void display(const int &_row);
    void display2(const int &_minus_x);
    bool operator<(const Zombie &other) const;
    void make_credit();
};