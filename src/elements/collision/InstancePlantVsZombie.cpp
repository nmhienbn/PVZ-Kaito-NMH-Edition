#include "InstancePlantVsZombie.hpp"

void apply_instance_plant_hit_zombie(Plants &plant, vector<Zombie *> &zombies,
                                     vector<ZombiePart> &zombie_parts)
{
    for (int j = 0; j < (int)zombies.size(); j++)
        if (check_in_attack_range(plant, *zombies[j]))
        {
            zombie_parts.push_back(ZombiePart(ZOMBIE_BURNT_DIRECTORY, ZOMBIE_DIE_FRAME,
                                              zombies[j]->row, zombies[j]->x_location,
                                              ZOMBIE_WIDTH, ZOMBIE_HEIGHT));
            delete zombies[j];
            zombies.erase(zombies.begin() + j);
            j--;
        }
}
