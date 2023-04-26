#pragma once
#include "music/music.hpp"
#include "draw/rsdl.hpp"
#include "game_stats.hpp"
#include "elements/Map/Map.hpp"

#define ZOMBIE_FRAME 3
#define ZOMBIE_EXACT_LOCATION 70
#define ZOMBIE_WIDTH 166
#define ZOMBIE_HEIGHT 144

#define ZOMBIE_EATING_FRAME 10

#define ZOMBIE_DIE_FRAME 4
#define ZOMBIE_BURNT_FRAME 6
#define DEAD_ZOMBIE_FRAMES 12

#define HEAD_ZOMBIE_FRAME 6
#define HEAD_ZOMBIE_WIDTH 125
#define HEAD_ZOMBIE_HEIGHT 156

class ZombiePart
{
private:
    int row;
    int x_location;
    int img_num;
    int frame = 0;
    int frame_clk;
    bool is_cold;
    int width, height;

public:
    ZombiePart(const int &_img_num, const int &_frame_clk,
               const int &_r, const int &_x,
               const int &_w, const int &_h, const bool &_cold = false);
    bool display(const int &_row);
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
protected:
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
    virtual ~Zombie();
    virtual void init(){};

    int get_health();
    void change_zombie_eating_status();
    virtual void armor_drop(vector<ZombiePart> &zombie_parts);
    void determine_appearance(vector<ZombiePart> &zombie_parts);
    bool decrease_health(vector<ZombiePart> &zombie_parts);
    void add_zombie_die(vector<ZombiePart> &zombie_parts);
    void display(const int &_row);
    void display2(const int &_minus_x);
    virtual void make_credit(){};

    friend Zombie *init(int _type, int level_num);
};