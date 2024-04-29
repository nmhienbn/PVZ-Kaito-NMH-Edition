#include "zombie.hpp"
#include "draw/rsdl.hpp"
#include "elements/Map/Map.hpp"
#include "music/music.hpp"
#include <algorithm>

extern int game_state;
extern Map cells;
extern Window win;
/*
Check if Zombie is in query tile or not to change its status.
@param zombie: query zombie
@param row: query row
@param col: query column
@return true if zombie is not in query tile.
*/
bool has_zombie_reached_block(const Zombie &zombie, const int &row, const int &col)
{
    int right_limit = cells[row][col].x2 - 1;
    int left_limit = cells[row][col].x1;
    int zombie_new_location = zombie.x_location - ZOMBIE_DX + 95;
    if (zombie.row == row &&
        is_in(left_limit, zombie_new_location, right_limit))
        return true;
    return false;
}

/*
Check if zombie reached a planted tile.
If true, Update zombie status.
*/
bool has_zombie_reached_plant(Zombie &zombie)
{
    int col = get_block_col(zombie.x_location - ZOMBIE_DX + 95);
    if (col >= 0)
    {
        if (cells[zombie.row][col].is_block_zombie && has_zombie_reached_block(zombie, zombie.row, col))
        {
            zombie.is_moving = false;
            zombie.change_zombie_eating_status();
            return true;
        }
    }
    return false;
}

/*
Check if zombie reach any plant or not.
Update zombies' moving status.
@return true if zombie reach any plant
*/
bool has_zombie_reached_any_plant(Zombie &zombie)
{
    if (has_zombie_reached_plant(zombie))
        return true;
    zombie.is_moving = true;
    zombie.change_zombie_eating_status();
    return false;
}

/*
Check if zombie can move or not:
    + If zombie go out background: false (because you have lose)
    But we don't need to do this because the has_player_lose will check it and end the game.
    + If zombie.is_moving is false: false (of course)
    + If zombie reached any element: has_zombie_reached_any_plant will decide it
*/
bool can_zombie_move(Zombie &zombie)
{
    if (!zombie.is_moving)
        return false;
    if (has_zombie_reached_any_plant(zombie))
        return false;
    return true;
}

/*
Update all zombies' moving status.
*/
void update_moving_status_for_zombies(vector<Zombie *> &zombies)
{
    for (auto &zombie : zombies)
    {
        has_zombie_reached_any_plant(*zombie);
    }
}

/*
Move the zombie: For all zombie:
    If zombie can move, its location -= their speed.
    Note that a zombie can be slow if it's cold.
*/
void move_zombies(vector<Zombie *> &zombies)
{
    for (auto &zombie : zombies)
    {
        if (zombie->next_step_time)
        {
            zombie->next_step_time--;
            continue;
        }
        zombie->next_step_time = ZOMBIE_CLK_COUNT;
        if (zombie->cold_time)
            zombie->next_step_time += ZOMBIE_CLK_COUNT;
        if (can_zombie_move(*zombie))
        {
            zombie->x_location -= ZOMBIE_DX;
        }
    }
}

/*
Display zombie of exactly status.
*/
void display_zombies(vector<Zombie *> &zombies, const int &_row)
{
    for (auto &zombie : zombies)
    {
        zombie->display(_row);
    }
}

void display_zombie_parts(vector<ZombiePart> &zombie_parts, const int &_row)
{
    for (int i = 0; i < (int)zombie_parts.size();)
    {
        if (zombie_parts[i].display(_row))
        {
            zombie_parts.erase(zombie_parts.begin() + i);
        }
        else
        {
            i++;
        }
    }
}