#pragma once
#include "game_stats.hpp"
#include "music/music.hpp"
#include "draw/rsdl.hpp"
#include "zombies/zombie_struct.hpp"

extern Map cells;

bool has_zombie_reached_element(const Zombie &zombie, const int &row, const int &col);
bool has_zombie_reached_plant(Zombie &zombie);
bool has_zombie_reached_any_plant(Zombie &zombie);
bool can_zombie_move(Zombie &zombie);
void update_moving_status_for_zombies(vector<Zombie> &zombies);
void move_zombies(vector<Zombie> &zombies);
void display_zombies(vector<Zombie> &zombies, const int &_row);
void display_dead_zombies(vector<DeadZombie> &dead_zombies, const int &_row);

//------------------------------Zombie bite--------------------------------------
void update_zombie_next_bite(vector<Zombie> &zombies);

template <class plant_type>
bool apply_zombie_bite_on_plant(Zombie &zombie, vector<plant_type> &plants, int &p_ind);

template <class plant_type>
void handle_zombie_plant_encounter(vector<Zombie> &zombies, vector<plant_type> &plants);

/*------------------------------------------------------------------------------------------------------------------
--------------------------------------------Template implementation-------------------------------------------------
------------------------------------------------------------------------------------------------------------------*/
/*
If the zombie reached plant_type, apply its bites on the plant_type:
    + bite++
    + if plant_type's health == 0: reset zombie moving and delete that plant_type
*/
template <class plant_type>
bool apply_zombie_bite_on_plant(Zombie &zombie, vector<plant_type> &plants, int &p_ind)
{
    if (has_zombie_reached_element(zombie, plants[p_ind].get_row(), plants[p_ind].get_col()))
    {
        play_sound_effect(ZOMBIE_EATING_MUSIC_DIRECTORY);
        zombie.bite_time = BITE_CLK_COUNT;
        if (zombie.cold_time)
            zombie.bite_time += BITE_CLK_COUNT;
        plants[p_ind].set_attacked_time(MAX_TIME_BLINK);
        if (plants[p_ind].decrease_health())
        {
            zombie.is_moving = true;
            zombie.bite_time = BITE_CLK_COUNT;
            cells[plants[p_ind].get_row()][plants[p_ind].get_col()].is_planted = false;
            plants.erase(plants.begin() + p_ind);
            p_ind--;
        }
        return true;
    }
    return false;
}

/* Note to optimize after
For all zombie and all plant_type: apply zombie's bite
*/
template <class plant_type>
void handle_zombie_plant_encounter(vector<Zombie> &zombies, vector<plant_type> &plants)
{
    for (auto &zombie : zombies)
        if (zombie.bite_time == 0)
            for (int i = 0; i < (int)plants.size(); i++)
                if (apply_zombie_bite_on_plant(zombie, plants, i))
                {
                    break;
                }
}