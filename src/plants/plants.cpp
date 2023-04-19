#include "plants.hpp"

Plants::Plants()
{
}

Plants::~Plants()
{
}

void Plants::display(const int &_row)
{
}

int Plants::get_row()
{
    return row;
}

int Plants::get_col()
{
    return col;
}

void Plants::set_attacked_time(const int &_attacked_time)
{
    attacked_time = _attacked_time;
}

bool Plants::decrease_health()
{
    return --health == 0;
}
