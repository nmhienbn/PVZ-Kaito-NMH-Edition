#include "elements.h"

extern Elements game_characters;

Zombie::Zombie()
{
}

/*
If is_moving is false, then the zombie must be eating.
Else the zombie can't be eating.
*/
Zombie::Zombie(int _type)
{
    type = _type;
    // Random appear row
    row = rand(0, 4);
    // Random delay time to appear
    x_location = WINDOW_WIDTH + rand(0, 100);

    is_moving = true;
    is_attacked = 0;
    cold_time = 0;

    if (type == NORMAL_TYPE)
    {
        health = ZOMBIE_NORMAL_HEALTH_LIMIT;
        directory_num = ZOMBIE_SHEET_DIRECTORY;
    }
    else if (type == CONE_TYPE)
    {
        health = ZOMBIE_NORMAL_HEALTH_LIMIT * 3;
        directory_num = CONE_ZOMBIE_WALK_1_DIRECTORY;
    }
    else if (type == BUCKET_TYPE)
    {
        health = ZOMBIE_NORMAL_HEALTH_LIMIT * 6;
        directory_num = BUCKET_ZOMBIE_WALK_1_DIRECTORY;
    }
    frame = rand(0, ZOMBIE_FRAME * N_SHEET[directory_num] - 1);
}

/*
If is_moving is false, then the zombie must be eating.
Else the zombie can't be eating.
*/
Zombie::Zombie(int _type, int _row, int _x)
{
    type = _type;
    // Random appear row
    row = _row;
    // Random delay time to appear
    x_location = _x;

    is_moving = true;
    is_attacked = 0;
    cold_time = 0;

    if (type == NORMAL_TYPE)
    {
        health = ZOMBIE_NORMAL_HEALTH_LIMIT;
        directory_num = ZOMBIE_SHEET_DIRECTORY;
    }
    else if (type == CONE_TYPE)
    {
        health = ZOMBIE_NORMAL_HEALTH_LIMIT * 3;
        directory_num = CONE_ZOMBIE_WALK_1_DIRECTORY;
    }
    else if (type == BUCKET_TYPE)
    {
        health = ZOMBIE_NORMAL_HEALTH_LIMIT * 6;
        directory_num = BUCKET_ZOMBIE_WALK_1_DIRECTORY;
    }
    frame = rand(0, ZOMBIE_FRAME * N_SHEET[directory_num] - 1);
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
            }
        }
        else if (type == CONE_TYPE)
        {
            auto it = eat_of.find(directory_num);
            if (it != eat_of.end())
            {
                frame = 0;
                directory_num = it->second;
            }
        }
        else if (type == BUCKET_TYPE)
        {
            auto it = eat_of.find(directory_num);
            if (it != eat_of.end())
            {
                frame = 0;
                directory_num = it->second;
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
            }
        }
        else if (type == CONE_TYPE)
        {
            if (directory_num == CONE_ZOMBIE_EATING_1_DIRECTORY)
            {
                frame = 0;
                directory_num = CONE_ZOMBIE_WALK_1_DIRECTORY;
            }
        }
        else if (type == BUCKET_TYPE)
        {
            auto it = walk_of.find(directory_num);
            if (it != walk_of.end())
            {
                frame = 0;
                directory_num = it->second;
            }
        }
    }
}

void Zombie::determine_appearance()
{
    if (type == BUCKET_TYPE)
    {
        if (health == ZOMBIE_NORMAL_HEALTH_LIMIT * 4)
        {
            if (is_moving)
                directory_num = BUCKET_ZOMBIE_WALK_2_DIRECTORY;
            else
                directory_num = BUCKET_ZOMBIE_EATING_2_DIRECTORY;
        }
        else if (health == ZOMBIE_NORMAL_HEALTH_LIMIT * 2)
        {
            if (is_moving)
                directory_num = BUCKET_ZOMBIE_WALK_3_DIRECTORY;
            else
                directory_num = BUCKET_ZOMBIE_EATING_3_DIRECTORY;
        }
        else if (health == ZOMBIE_NORMAL_HEALTH_LIMIT)
        {
            type = NORMAL_TYPE;
            DeadZombie tmp(row, x_location, NULL_DIRECTORY, BUCKET_DROP_DIRECTORY);
            game_characters.dead_zombies.push_back(tmp);
            if (is_moving)
                directory_num = ZOMBIE_WALK1_DIRECTORY;
            else
                directory_num = ZOMBIE_EATING_DIRECTORY;
        }
    }
    if (type == CONE_TYPE)
    {
        if (health == ZOMBIE_NORMAL_HEALTH_LIMIT * 7 / 3)
        {
            if (is_moving)
                directory_num = CONE_ZOMBIE_WALK_2_DIRECTORY;
            else
                directory_num = CONE_ZOMBIE_EATING_2_DIRECTORY;
        }
        else if (health == ZOMBIE_NORMAL_HEALTH_LIMIT * 5 / 3)
        {
            if (is_moving)
                directory_num = CONE_ZOMBIE_WALK_3_DIRECTORY;
            else
                directory_num = CONE_ZOMBIE_EATING_3_DIRECTORY;
        }
        else if (health == ZOMBIE_NORMAL_HEALTH_LIMIT)
        {
            type = NORMAL_TYPE;
            DeadZombie tmp(row, x_location, NULL_DIRECTORY, CONE_DROP_DIRECTORY);
            game_characters.dead_zombies.push_back(tmp);
            if (is_moving)
                directory_num = ZOMBIE_WALK1_DIRECTORY;
            else
                directory_num = ZOMBIE_EATING_DIRECTORY;
        }
    }
}

bool Zombie::decrease_health()
{
    health--;
    is_attacked = MAX_TIME_BLINK;
    determine_appearance();
    if (health == 0)
    {
        return true;
    }
    return false;
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
    int z_cnt = 0;
    for (int i = NORMAL_TYPE; i < COUNT_ZOMBIE_TYPE; i++)
    {
        z_cnt += wave_zombie_count[i][cur_wave];
    }
    return z_cnt >= 5;
}

DeadZombie::DeadZombie(int _r, int _x, bool _cold)
{
    row = _r;
    x_location = _x;
    is_cold = _cold;
}

DeadZombie::DeadZombie(int _r, int _x, int _body, int _head)
{
    row = _r;
    x_location = _x;
    body = _body;
    head = _head;
}
