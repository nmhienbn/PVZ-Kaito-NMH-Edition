#pragma once

class Plants
{
protected:
    int row, col;
    int health;
    int sec_for_prepare;
    int directory_num;
    int frame = 0;
    int attacked_time = 0;

public:
    Plants();
    ~Plants();
    virtual void display(const int &_row);
    int get_row();
    int get_col();
    void set_attacked_time(const int &_attacked_time);
    bool decrease_health();
};