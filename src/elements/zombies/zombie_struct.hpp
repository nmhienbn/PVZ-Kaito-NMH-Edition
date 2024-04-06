#pragma once
#include "draw/rsdl.hpp"
#include "elements/Map/Map.hpp"
#include "game_stats.hpp"
#include "music/music.hpp"

extern const int ZOMBIE_FRAME;
extern const int ZOMBIE_EXACT_LOCATION;
extern const int ZOMBIE_WIDTH;
extern const int ZOMBIE_HEIGHT;

extern const int ZOMBIE_EATING_FRAME;
extern const int ZOMBIE_DIE_FRAME;
extern const int ZOMBIE_BURNT_FRAME;

extern const int HEAD_ZOMBIE_FRAME;
extern const int HEAD_ZOMBIE_WIDTH;
extern const int HEAD_ZOMBIE_HEIGHT;
extern const int ARM_ZOMBIE_FRAME;

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
    /*Init a zombie*/
    virtual void init(){};
    /*Make zombie for credit*/
    virtual void make_credit(){};
    /*Make zombie's armor drop*/
    virtual void armor_drop(vector<ZombiePart> &zombie_parts){};

    int get_health();
    void change_zombie_eating_status();
    void determine_appearance(vector<ZombiePart> &zombie_parts);
    bool decrease_health(vector<ZombiePart> &zombie_parts);
    void add_zombie_die(vector<ZombiePart> &zombie_parts);
    void display(const int &_row);
    void display_credited(const int &_minus_x);

    friend Zombie *init(int _type, int level_num);
};