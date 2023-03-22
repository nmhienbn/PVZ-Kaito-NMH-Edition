#include "elements.h"

/*
If is_moving is false, then the zombie must be eating.
Else the zombie can't be eating.
*/
void Zombie::change_zombie_eating_status()
{
    if (is_moving == false)
    {
        if (directory_num == ZOMBIE_SHEET_DIRECTORY)
        {
            frame = 0;
            directory_num = ZOMBIE_EATING_DIRECTORY;
            blink_directory_num = ZOMBIE_EATING_BLINK_DIRECTORY;
            n_sheet = ZOMBIE_EATING_N_SHEET;
            c_sheet = ZOMBIE_EATING_C_SHEET;
        }
    }
    else
    {
        if (directory_num == ZOMBIE_EATING_DIRECTORY)
        {
            frame = 0;
            directory_num = ZOMBIE_SHEET_DIRECTORY;
            blink_directory_num = ZOMBIE_BLINK_SHEET_DIRECTORY;
            n_sheet = ZOMBIE_N_SHEET;
            c_sheet = ZOMBIE_C_SHEET;
        }
    }
}

bool Zombie::operator<(const Zombie &other) const
{
    return (row == other.row ? x_location < other.x_location : row > other.row);
}

Button::Button()
{
}

Button::Button(int _x1, int _x2, int _y1, int _y2)
{
    x1 = _x1;
    x2 = _x2;
    y1 = _y1;
    y2 = _y2;
}

Button::~Button()
{
}

bool Button::is_mouse_in(int mouse_x, int mouse_y) const
{
    if (x1 < mouse_x && mouse_x < x2 &&
        y1 < mouse_y && mouse_y < y2)
        return true;
    return false;
}
int rand(int L, int R)
{
    if (L > R)
        exit(-10);
    return rd() % (R - L + 1) + L;
}

Icons::Icons()
{
}
