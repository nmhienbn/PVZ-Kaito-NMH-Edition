#pragma once
#include "elements.h"
#include "music.h"
#include "rsdl.hpp"

bool has_zombie_reached_element(const Zombie &zombie, const int &row, const int &col, Map &cells);
template <class vector_plant>
bool has_zombie_reached_plant(Zombie &zombie, vector_plant &plants, Map &cells);
bool has_zombie_reached_any_plant(Elements &elements, Zombie &zombie, Map &cells);
bool can_zombie_move(Zombie &zombie, Elements &elements, Map &cells);
void update_moving_status_for_zombies(Elements &elements, Map &cells);
void move_zombies(vector<Zombie> &zombies, Elements &elements, Map &cells);
void display_zombies(window &win, vector<Zombie> &zombies, Map &cells, bool is_pause = false);
void display_dead_zombies(window &win, vector<DeadZombie> &dead_zombies, Map &cells, bool is_pause = false);

//------------------------------Zombie bite--------------------------------------
void update_zombie_next_bite(vector<Zombie> &zombies);

template <class plant_type>
void apply_zombie_bite_on_plant(Zombie &zombie, vector<plant_type> &plants, int &p_ind, Map &cells, const int bite_limit);

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
void apply_zombie_bite_on_plant(Zombie &zombie, vector<plant_type> &plants, int &p_ind, Map &cells, const int bite_limit)
{
    if (has_zombie_reached_element(zombie, plants[p_ind].row, plants[p_ind].col, cells))
    {
        play_sound_effect(ZOMBIE_EATING_MUSIC_DIRECTORY);
        zombie.bite_time = BITE_CLK_COUNT;
        if (zombie.cold_time)
            zombie.bite_time += BITE_CLK_COUNT;
        plants[p_ind].bite++;
        plants[p_ind].is_attacked = MAX_TIME_BLINK;
        if (plants[p_ind].bite == bite_limit)
        {
            zombie.is_moving = true;
            zombie.bite_time = 0;
            cells[plants[p_ind].row][plants[p_ind].col].is_planted = false;
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
    for (auto &zombie : zombies)
        if (zombie.bite_time == 0)
            for (int i = 0; i < (int)plants.size(); i++)
                apply_zombie_bite_on_plant(zombie, plants, i, cells, bite_limit);
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