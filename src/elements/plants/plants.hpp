#pragma once
#include "draw/rsdl.hpp"
#include "elements/zombies/zombie_struct.hpp"
#include "music/music.hpp"
#include "plants.hpp"
#include <nlohmann/json.hpp>
#include <vector>
using namespace std;
using namespace nlohmann;

struct PlantAssets
{
    int frame;
    int img_width;
    int img_height;
    int game_width;
    int game_height;
    int x_alias;
    int y_alias;
    PlantAssets(int _frame, int img_width, int img_height, int game_width, int game_height, int x_alias, int y_alias);
};

class Plants
{
protected:
    int type;
    int row, col;
    int health;
    int sec_for_prepare;
    int directory_num;
    int frame = 0;
    int attacked_time = 0;

public:
    Plants();
    virtual ~Plants();
    virtual void display(const int &_row);
    virtual void action();
    int get_type();
    int get_row();
    int get_col();
    void set_attacked_time(const int &_attacked_time);
    void decrease_health();
    bool is_died();
    void display(const PlantAssets &assets);
    virtual void next_frame(const PlantAssets &assets);
};

void delete_plant(vector<Plants *> &plants, int &ind);
void update_plants_status(vector<Plants *> &plants);