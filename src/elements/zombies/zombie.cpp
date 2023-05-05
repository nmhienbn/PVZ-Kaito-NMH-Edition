#include <algorithm>
#include "zombie.hpp"

extern int game_state;
extern Map cells;
extern window win;
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
        if (cells[zombie.row][col].is_planted && has_zombie_reached_block(zombie, zombie.row, col))
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

/*
Update time for all zombies' next bite.
*/
void update_zombie_next_bite(vector<Zombie *> &zombies)
{
    for (auto &zombie : zombies)
        if (zombie->bite_time)
            zombie->bite_time--;
}
/*
If the zombie reached plant_type, apply its bites on the plant_type:
    + bite++
    + if plant_type's health == 0: reset zombie moving and delete that plant_type
*/
bool apply_zombie_bite_on_plant(Zombie &zombie, vector<Plants *> &plants, int &p_ind)
{
    if (has_zombie_reached_block(zombie, plants[p_ind]->get_row(), plants[p_ind]->get_col()))
    {
        play_sound_effect(ZOMBIE_EATING_MUSIC_DIRECTORY);
        zombie.bite_time = BITE_CLK_COUNT;
        if (zombie.cold_time)
            zombie.bite_time += BITE_CLK_COUNT;
        plants[p_ind]->set_attacked_time(MAX_TIME_BLINK);
        plants[p_ind]->decrease_health();
        if (plants[p_ind]->is_died())
        {
            zombie.is_moving = true;
            zombie.bite_time = BITE_CLK_COUNT;
            delete_plant(plants, p_ind);
        }
        return true;
    }
    return false;
}

/* Note to optimize after
For all zombie and all plant_type: apply zombie's bite
*/
void handle_zombie_plant_encounter(vector<Zombie *> &zombies, vector<Plants *> &plants)
{
    for (auto &zombie : zombies)
        if (zombie->bite_time == 0)
            for (int i = 0; i < (int)plants.size(); i++)
                if (apply_zombie_bite_on_plant(*zombie, plants, i))
                {
                    break;
                }
}

/*
    + Update all zombies' moving status.
    + Make zombies' bite to plants and update the next time they can bite them.
*/
void update_zombies_status(vector<Zombie *> &zombies, vector<Plants *> &plants)
{
    // Update all zombies' moving status
    update_moving_status_for_zombies(zombies);

    // Zombie bite plant
    handle_zombie_plant_encounter(zombies, plants);

    // Update next time for each zombie to bite plant
    update_zombie_next_bite(zombies);
}
