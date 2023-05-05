#pragma once
#include <vector>
using namespace std;

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
};

void delete_plant(vector<Plants *> &plants, int &ind);
void update_plants_status(vector<Plants *> &plants);