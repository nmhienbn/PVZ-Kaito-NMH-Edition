#include "zombie_struct.hpp"

/*Zombie constructor.
 */
Zombie::Zombie()
{
}

/*Zombie constructor.
Random row and delay time appear.
Set init status, heath and image to render.
Random first frame.
*/
Zombie::Zombie(int _type, int level_num)
{
    type = _type;
    // Random appear row
    if (level_num == 1)
        row = 2;
    else if (level_num == 2)
        row = rand(1, 3);
    else
        row = rand(0, 4);
    // Random delay time to appear
    x_location = WINDOW_WIDTH - 70 + rand(0, 100);
    // Set init status
    is_moving = true;
    is_attacked = cold_time = 0;
    bite_time = BITE_CLK_COUNT;

    if (type == NORMAL_TYPE)
    {
        health = ZOMBIE_NORMAL_HEALTH_LIMIT;
        directory_num = ZOMBIE_SHEET_DIRECTORY;
    }
    else if (type == CONE_TYPE)
    {
        health = ZOMBIE_CONE1_HEALTH_LIMIT;
        directory_num = CONE_ZOMBIE_WALK_1_DIRECTORY;
    }
    else if (type == BUCKET_TYPE)
    {
        health = ZOMBIE_BUCKET1_HEALTH_LIMIT;
        directory_num = BUCKET_ZOMBIE_WALK_1_DIRECTORY;
    }
    // Random first frame.
    frame = rand(0, ZOMBIE_FRAME * N_SHEET[directory_num] - 1);
}

/*
Zombie construction: Specific type (Flag) and image of zombie
Zombie appear at once.
*/
Zombie::Zombie(int _type, int level_num, int _directory_num)
{
    type = _type;
    // Random appear row
    if (level_num == 1)
        row = 2;
    else if (level_num == 2)
        row = rand(1, 3);
    else
        row = rand(0, 4);
    // Fix delay time to appear
    x_location = WINDOW_WIDTH - 50;
    // Set init status
    is_moving = true;
    is_attacked = cold_time = 0;
    bite_time = BITE_CLK_COUNT;
    health = ZOMBIE_NORMAL_HEALTH_LIMIT;
    directory_num = _directory_num;
    // Random first frame.
    frame = rand(0, ZOMBIE_FRAME * N_SHEET[directory_num] - 1);
}

/*Zombie constructor with fixed _row and _x.
Set init status, heath and image to render.
Random first frame.
*/
Zombie::Zombie(int _type, int level_num, int _row, int _x)
{
    type = _type;
    row = _row;
    x_location = _x;

    // Set init status
    is_moving = true;
    is_attacked = cold_time = 0;
    bite_time = BITE_CLK_COUNT;

    if (type == NORMAL_TYPE)
    {
        health = ZOMBIE_NORMAL_HEALTH_LIMIT;
        directory_num = ZOMBIE_SHEET_DIRECTORY;
    }
    else if (type == CONE_TYPE)
    {
        health = ZOMBIE_CONE1_HEALTH_LIMIT;
        directory_num = CONE_ZOMBIE_WALK_1_DIRECTORY;
    }
    else if (type == BUCKET_TYPE)
    {
        health = ZOMBIE_BUCKET1_HEALTH_LIMIT;
        directory_num = BUCKET_ZOMBIE_WALK_1_DIRECTORY;
    }
    // Random first frame.
    frame = rand(0, ZOMBIE_FRAME * N_SHEET[directory_num] - 1);
}

/*Change zombie eating status
If is_moving is false, then the zombie must be eating.
Else the zombie can't be eating.
*/
void Zombie::change_zombie_eating_status()
{
    if (is_moving == false)
    {
        if (type == NORMAL_TYPE)
        {
            if (directory_num == FLAG_ZOMBIE_WALK_DIRECTORY)
            {
                directory_num = FLAG_ZOMBIE_EATING_DIRECTORY;
            }
            else if (directory_num != ZOMBIE_EATING_DIRECTORY)
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
            else if (directory_num == FLAG_ZOMBIE_EATING_DIRECTORY)
            {
                directory_num = FLAG_ZOMBIE_WALK_DIRECTORY;
            }
        }
        else if (type == CONE_TYPE)
        {
            auto it = walk_of.find(directory_num);
            if (it != walk_of.end())
            {
                frame = 0;
                directory_num = it->second;
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

/*Determine zombies's appearance depend on their health:
Let x = normal zombie's health limit.
Bucket-head:
    + ZOMBIE_BUCKET1_HEALTH_LIMIT: degrade 1.
    + ZOMBIE_BUCKET2_HEALTH_LIMIT: degrade 2.
    + ZOMBIE_BUCKET3_HEALTH_LIMIT: degrade 3.
    + x: turn into normal.
Cone-head:
    + ZOMBIE_CONE1_HEALTH_LIMIT: degrade 1.
    + ZOMBIE_CONE2_HEALTH_LIMIT: degrade 2.
    + ZOMBIE_CONE3_HEALTH_LIMIT: degrade 3.
    + x: turn into normal.
*/
void Zombie::determine_appearance(vector<DeadZombie> &dead_zombies)
{
    if (type == BUCKET_TYPE)
    {
        if (health == ZOMBIE_BUCKET2_HEALTH_LIMIT)
        {
            if (is_moving)
                directory_num = BUCKET_ZOMBIE_WALK_2_DIRECTORY;
            else
                directory_num = BUCKET_ZOMBIE_EATING_2_DIRECTORY;
        }
        else if (health == ZOMBIE_BUCKET3_HEALTH_LIMIT)
        {
            if (is_moving)
                directory_num = BUCKET_ZOMBIE_WALK_3_DIRECTORY;
            else
                directory_num = BUCKET_ZOMBIE_EATING_3_DIRECTORY;
        }
        else if (health == ZOMBIE_NORMAL_HEALTH_LIMIT)
        {
            type = NORMAL_TYPE;
            dead_zombies.push_back(DeadZombie(row, x_location, NULL_DIRECTORY, BUCKET_DROP_DIRECTORY));
            if (is_moving)
                directory_num = ZOMBIE_WALK1_DIRECTORY;
            else
                directory_num = ZOMBIE_EATING_DIRECTORY;
        }
    }
    if (type == CONE_TYPE)
    {
        if (health == ZOMBIE_CONE2_HEALTH_LIMIT)
        {
            if (is_moving)
                directory_num = CONE_ZOMBIE_WALK_2_DIRECTORY;
            else
                directory_num = CONE_ZOMBIE_EATING_2_DIRECTORY;
        }
        else if (health == ZOMBIE_CONE3_HEALTH_LIMIT)
        {
            if (is_moving)
                directory_num = CONE_ZOMBIE_WALK_3_DIRECTORY;
            else
                directory_num = CONE_ZOMBIE_EATING_3_DIRECTORY;
        }
        else if (health == ZOMBIE_NORMAL_HEALTH_LIMIT)
        {
            type = NORMAL_TYPE;
            dead_zombies.push_back(DeadZombie(row, x_location, NULL_DIRECTORY, CONE_DROP_DIRECTORY));
            if (is_moving)
                directory_num = ZOMBIE_WALK1_DIRECTORY;
            else
                directory_num = ZOMBIE_EATING_DIRECTORY;
        }
    }
}

/*
Decrease zombie health and change their appearance if necessary.
@return true if zombie die (health = 0).
*/
bool Zombie::decrease_health(vector<DeadZombie> &dead_zombies)
{
    health--;
    is_attacked = MAX_TIME_BLINK;
    determine_appearance(dead_zombies);
    if (health == 0)
    {
        return true;
    }
    return false;
}

/*
Compare zombie < zombie:
    x < x,
    cold_time < cold_time
*/
bool Zombie::operator<(const Zombie &other) const
{
    return (x_location == other.x_location ? x_location < other.x_location
                                           : cold_time < other.cold_time);
}

/*Dead constructor*/
DeadZombie::DeadZombie(int _r, int _x, bool _cold)
{
    row = _r;
    x_location = _x;
    is_cold = _cold;
}

/*Dead constructor with other body and head*/
DeadZombie::DeadZombie(int _r, int _x, int _body, int _head)
{
    row = _r;
    x_location = _x;
    body = _body;
    head = _head;
}