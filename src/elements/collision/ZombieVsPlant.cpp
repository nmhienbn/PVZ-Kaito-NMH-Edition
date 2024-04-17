#include "ZombieVsPlant.hpp"

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
        PotatoMine *potatoMinePtr = dynamic_cast<PotatoMine *>(plants[p_ind]);
        if (potatoMinePtr == nullptr || !potatoMinePtr->is_armed())
        {
            play_sound_effect(ZOMBIE_EATING_MUSIC_DIRECTORY);
        }

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
