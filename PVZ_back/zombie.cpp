#include "zombie.h"

/*
Check if Zombie is in query tile or not.
@param zombie: query zombie
@param row: query row
@param col: query column
@return true if zombie is not in query tile.
*/
bool has_zombie_reached_element(Zombie zombie, int row, int col, Map &map)
{
    int right_limit = map[row][col].x2 - TILE_WIDTH / 2;
    int left_limit = map[row][col].x1 - TILE_WIDTH / 2;
    int zombie_new_location = zombie.x_location - ZOMBIE_DX;
    if (zombie.row == row &&
        left_limit < zombie_new_location && zombie_new_location < right_limit)
        return true;
    return false;
}

/*
Check if zombie reach any plant or not.
Update zombies' moving status.
@return true if zombie reach any plant
*/
bool has_zombie_reached_any_plant(Elements &elements, Zombie &zombie, Map &map)
{
    if (has_zombie_reached_plant(zombie, elements.peashooters, map))
        return true;
    if (has_zombie_reached_plant(zombie, elements.sunflowers, map))
        return true;
    if (has_zombie_reached_plant(zombie, elements.walnuts, map))
        return true;
    zombie.is_moving = true;
    zombie.change_zombie_eating_status();
    return false;
}

/*
Check if zombie can move or not:
    + If zombie go out background: false (because you have lose)
    + If zombie.is_moving is false: false (of course)
    + If zombie reached any element: has_zombie_reached_any_plant will decide it
*/
bool can_zombie_move(Zombie &zombie, Elements &elements, Map &map)
{
    int left_bound = map[0][0].x1 - 40;
    int zombie_new_location = zombie.x_location - ZOMBIE_DX;
    if (zombie_new_location < left_bound)
        return false;
    if (!zombie.is_moving)
        return false;
    if (has_zombie_reached_any_plant(elements, zombie, map))
        return false;
    return true;
}

/* I have changed this for better performance
Update all zombies' moving status.
*/
void update_moving_status_for_zombies(Elements &elements, Map &map)
{
    for (int i = 0; i < elements.zombies.size(); i++)
    {
        has_zombie_reached_any_plant(elements, elements.zombies[i], map);
    }
}

/*
Move the zombie: For all zombie:
    + If zombie can move, its location -= their speed.
*/
void move_zombies(vector<Zombie> &zombies, Elements &elements, Map &map)
{
    for (int i = 0; i < zombies.size(); i++)
    {
        if (can_zombie_move(zombies[i], elements, map))
            zombies[i].x_location -= ZOMBIE_DX;
    }
}
