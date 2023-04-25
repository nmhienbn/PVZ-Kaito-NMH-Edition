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

int Plants::get_type()
{
    return type;
}

void Plants::action()
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

void Plants::decrease_health()
{
    --health;
}

bool Plants::is_died()
{
    return health <= 0;
}
