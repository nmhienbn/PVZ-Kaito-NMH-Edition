#include "elements.h"

Zombie::Zombie()
{
}

/*
If is_moving is false, then the zombie must be eating.
Else the zombie can't be eating.
*/
Zombie::Zombie(ZombieType _type)
{
    type = _type;
    if (type == NORMAL_TYPE)
    {
        // Random appear row
        row = rand(0, 4);
        // Random delay time to appear
        x_location = WINDOW_WIDTH + rand(0, 100);
        health = ZOMBIE_NORMAL_HEALTH_LIMIT;
        is_moving = true;
        directory_num = ZOMBIE_SHEET_DIRECTORY;
        blink_directory_num = blink_of[directory_num];

        frame = rand(0, ZOMBIE_FRAME * N_SHEET[directory_num] - 1);
        is_attacked = 0;
    }
    else if (type == CONE_TYPE)
    {
        // Random appear row
        row = rand(0, 4);
        // Random delay time to appear
        x_location = WINDOW_WIDTH + rand(0, 100);
        health = ZOMBIE_NORMAL_HEALTH_LIMIT;
        is_moving = true;
        directory_num = CONE_ZOMBIE_WALK_DIRECTORY;
        blink_directory_num = CONE_ZOMBIE_WALK_BLINK_DIRECTORY;

        frame = rand(0, ZOMBIE_FRAME * N_SHEET[directory_num] - 1);
        is_attacked = 0;
    }
    else if (type == BUCKET_TYPE)
    {
        // Random appear row
        row = rand(0, 4);
        // Random delay time to appear
        x_location = WINDOW_WIDTH + rand(0, 100);
        health = ZOMBIE_NORMAL_HEALTH_LIMIT * 2;
        is_moving = true;
        directory_num = BUCKET_ZOMBIE_WALK_DIRECTORY;
        blink_directory_num = BUCKET_ZOMBIE_WALK_BLINK_DIRECTORY;

        frame = rand(0, ZOMBIE_FRAME * N_SHEET[directory_num] - 1);
        is_attacked = 0;
    }
}

/*
If is_moving is false, then the zombie must be eating.
Else the zombie can't be eating.
*/
Zombie::Zombie(ZombieType _type, int _row, int _x)
{
    type = _type;
    if (type == NORMAL_TYPE)
    {
        // Random appear row
        row = _row;
        // Random delay time to appear
        x_location = _x;
        health = ZOMBIE_NORMAL_HEALTH_LIMIT;
        is_moving = true;
        directory_num = ZOMBIE_SHEET_DIRECTORY;
        blink_directory_num = blink_of[directory_num];

        frame = rand(0, ZOMBIE_FRAME * N_SHEET[directory_num] - 1);
        is_attacked = 0;
    }
    else if (type == CONE_TYPE)
    {
        // Random appear row
        row = _row;
        // Random delay time to appear
        x_location = _x;
        health = ZOMBIE_NORMAL_HEALTH_LIMIT;
        is_moving = true;
        directory_num = CONE_ZOMBIE_WALK_DIRECTORY;
        blink_directory_num = CONE_ZOMBIE_WALK_BLINK_DIRECTORY;

        frame = rand(0, ZOMBIE_FRAME * N_SHEET[directory_num] - 1);
        is_attacked = 0;
    }
    else if (type == BUCKET_TYPE)
    {
        // Random appear row
        row = _row;
        // Random delay time to appear
        x_location = _x;
        health = ZOMBIE_NORMAL_HEALTH_LIMIT * 2;
        is_moving = true;
        directory_num = BUCKET_ZOMBIE_WALK_DIRECTORY;
        blink_directory_num = BUCKET_ZOMBIE_WALK_BLINK_DIRECTORY;

        frame = rand(0, ZOMBIE_FRAME * N_SHEET[directory_num] - 1);
        is_attacked = 0;
    }
}

void Zombie::change_zombie_eating_status()
{
    if (is_moving == false)
    {
        if (type == NORMAL_TYPE)
        {
            if (directory_num != ZOMBIE_EATING_DIRECTORY)
            {
                frame = 0;
                directory_num = ZOMBIE_EATING_DIRECTORY;
                blink_directory_num = ZOMBIE_EATING_BLINK_DIRECTORY;
            }
        }
        else if (type == CONE_TYPE)
        {
            if (directory_num != CONE_ZOMBIE_EATING_DIRECTORY)
            {
                frame = 0;
                directory_num = CONE_ZOMBIE_EATING_DIRECTORY;
                blink_directory_num = CONE_ZOMBIE_EATING_BLINK_DIRECTORY;
            }
        }
        else if (type == BUCKET_TYPE)
        {
            if (directory_num != BUCKET_ZOMBIE_EATING_DIRECTORY)
            {
                frame = 0;
                directory_num = BUCKET_ZOMBIE_EATING_DIRECTORY;
                blink_directory_num = BUCKET_ZOMBIE_EATING_BLINK_DIRECTORY;
            }
        }
    }
    else
    {
        if (type == NORMAL_TYPE)
        {
            if (directory_num == ZOMBIE_EATING_DIRECTORY)
            {
                frame = 0;
                directory_num = ZOMBIE_SHEET_DIRECTORY;
                blink_directory_num = blink_of[directory_num];
            }
        }
        else if (type == CONE_TYPE)
        {
            if (directory_num == CONE_ZOMBIE_EATING_DIRECTORY)
            {
                frame = 0;
                directory_num = CONE_ZOMBIE_WALK_DIRECTORY;
                blink_directory_num = directory_num + 1;
            }
        }
        else if (type == BUCKET_TYPE)
        {
            if (directory_num == BUCKET_ZOMBIE_EATING_DIRECTORY)
            {
                frame = 0;
                directory_num = BUCKET_ZOMBIE_WALK_DIRECTORY;
                blink_directory_num = directory_num + 1;
            }
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

Pea::Pea()
{
}
Pea::Pea(int _type, int _row, int _x)
{
    type = _type;
    row = _row;
    x_location = _x;
    if (type == 1)
    {
        directory_num = PEA_DIRECTORY;
        explode = PEA_EXPLODE_TIME;
    }
    else if (type == 2)
    {
        directory_num = SNOWZ_PEA_DIRECTORY;
        explode = PEA_EXPLODE_TIME;
    }
}

bool Level::is_huge_wave()
{
    return wave_zombie_count[cur_wave] >= 5;
}
