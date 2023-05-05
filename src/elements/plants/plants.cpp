#include "plants.hpp"

/*
Plant constructor
*/
Plants::Plants()
{
}

/*
Plant destructor
*/
Plants::~Plants()
{
}

/*
Plant display: nothing
*/
void Plants::display(const int &_row)
{
}

/*
Plant geter
@return plant's type
*/
int Plants::get_type()
{
    return type;
}

/*
Plant action virtual
*/
void Plants::action()
{
}

/*
Plant geter
@return plant's row
*/
int Plants::get_row()
{
    return row;
}

/*
Plant geter
@return plant's column
*/
int Plants::get_col()
{
    return col;
}

/*
Plant seter
Set plant's attacked time
*/
void Plants::set_attacked_time(const int &_attacked_time)
{
    attacked_time = _attacked_time;
}

/*
Decrease plant's health
*/
void Plants::decrease_health()
{
    --health;
}

/*
@return true if plant died (health <= 0)
*/
bool Plants::is_died()
{
    return health <= 0;
}
