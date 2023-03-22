#pragma once
#include "elements.h"

bool has_zombie_reached_element(Zombie zombie, int row, int col, Map &cells);
template <class vector_plant>
bool has_zombie_reached_plant(Zombie &zombie, vector_plant &plants, Map &cells);
bool has_zombie_reached_any_plant(Elements &elements, Zombie &zombie, Map &cells);
bool can_zombie_move(Zombie &zombie, Elements &elements, Map &cells);
void update_moving_status_for_zombies(Elements &elements, Map &cells);
void move_zombies(vector<Zombie> &zombies, Elements &elements, Map &cells);

//------------------------------Zombie bite--------------------------------------
template <class plant_type>
void apply_zombie_bite_on_plant(vector<Zombie> &zombies, vector<plant_type> &plants, int &z_ind, int &p_ind, Map &cells, const int bite_limit);

template <class plant_type>
void handle_zombie_plant_encounter(vector<Zombie> &zombies, vector<plant_type> &plants, Map &cells, const int bite_limit);

/*------------------------------------------------------------------------------------------------------------------
--------------------------------------------Template implementation-------------------------------------------------
------------------------------------------------------------------------------------------------------------------*/
/*
If the zombie reached plant_type, apply its bites on the plant_type:
    + bite++
    + if plant_type's bite == LIMIT: reset zombie moving and delete that plant_type
*/
template <class plant_type>
void apply_zombie_bite_on_plant(vector<Zombie> &zombies, vector<plant_type> &plants, int &z_ind, int &p_ind, Map &cells, const int bite_limit)
{
    if (has_zombie_reached_element(zombies[z_ind], plants[p_ind].row, plants[p_ind].col, cells))
    {
        plants[p_ind].bite++;
        plants[p_ind].is_attacked = MAX_TIME_BLINK;
        if (plants[p_ind].bite == bite_limit)
        {
            zombies[z_ind].is_moving = true;
            cells[plants[p_ind].row][plants[p_ind].col].is_planted = 0;
            plants.erase(plants.begin() + p_ind);
        }
    }
}

/* Note to optimize after
For all zombie and all plant_type: apply zombie's bite
*/
template <class plant_type>
void handle_zombie_plant_encounter(vector<Zombie> &zombies, vector<plant_type> &plants, Map &cells, const int bite_limit)
{
    for (int i = 0; i < plants.size(); i++)
        for (int j = 0; j < zombies.size(); j++)
            apply_zombie_bite_on_plant(zombies, plants, j, i, cells, bite_limit);
}

template <class vector_plant>
bool has_zombie_reached_plant(Zombie &zombie, vector_plant &plants, Map &cells)
{
    for (auto &plant : plants)
    {
        if (has_zombie_reached_element(zombie, plant.row, plant.col, cells))
        {
            zombie.is_moving = false;
            zombie.change_zombie_eating_status();
            return true;
        }
    }
    return false;
}