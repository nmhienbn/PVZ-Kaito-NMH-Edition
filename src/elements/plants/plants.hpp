#pragma once
#include "draw/rsdl.hpp"
#include "music/music.hpp"

enum AttackRange
{
    NO_ATTACK,
    ONE_ROW_AHEAD,
    THREE_X_THREE,
    ONE_X_ONE
};

struct PlantAssets
{
    int frame; // How many frame for a state in sprite sheet
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
    AttackRange attack_range;

public:
    Plants();
    Plants(const int &_row, const int &_col);

    AttackRange get_attack_range() const;
    int get_type() const;
    int get_row() const;
    int get_col() const;
    void set_attacked_time(const int &_attacked_time);
    void decrease_health();
    bool is_died();
    void display(const PlantAssets &assets);

    virtual ~Plants();
    virtual void display(const int &_row);
    virtual void action();
    virtual void next_frame(const PlantAssets &assets);
};

void delete_plant(vector<Plants *> &plants, int &ind);
void update_plants_status(vector<Plants *> &plants);