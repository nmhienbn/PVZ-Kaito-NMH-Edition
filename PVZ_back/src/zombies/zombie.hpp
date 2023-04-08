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
void display_zombies(vector<Zombie> &zombies);
void display_dead_zombies(vector<DeadZombie> &dead_zombies);

//------------------------------Zombie bite--------------------------------------
void update_zombie_next_bite(vector<Zombie> &zombies);

template <class plant_type>
void apply_zombie_bite_on_plant(Zombie &zombie, vector<plant_type> &plants, int &p_ind, const int bite_limit);

template <class plant_type>
void handle_zombie_plant_encounter(vector<Zombie> &zombies, vector<plant_type> &plants, const int bite_limit);

/*------------------------------------------------------------------------------------------------------------------
--------------------------------------------Template implementation-------------------------------------------------
------------------------------------------------------------------------------------------------------------------*/
/*
If the zombie reached plant_type, apply its bites on the plant_type:
    + bite++
    + if plant_type's bite == LIMIT: reset zombie moving and delete that plant_type
*/
template <class plant_type>
void apply_zombie_bite_on_plant(Zombie &zombie, vector<plant_type> &plants, int &p_ind, const int bite_limit)
{
    if (has_zombie_reached_element(zombie, plants[p_ind].row, plants[p_ind].col))
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
void handle_zombie_plant_encounter(vector<Zombie> &zombies, vector<plant_type> &plants, const int bite_limit)
{
    for (auto &zombie : zombies)
        if (zombie.bite_time == 0)
            for (int i = 0; i < (int)plants.size(); i++)
                apply_zombie_bite_on_plant(zombie, plants, i, bite_limit);
}