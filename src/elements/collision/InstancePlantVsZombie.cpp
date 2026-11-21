#include "InstancePlantVsZombie.hpp"

void apply_instance_plant_hit_zombie(Plants &plant, vector<Zombie *> &zombies,
                                     vector<ZombiePart> &zombie_parts)
{
    /*Burn aftermath (repvz KillAllZombiesInRadius' theBurn,
    Board.cpp:9852-9858): the cherry bomb (Plant.cpp:4376-4386) and the
    doom-shroom (Plant.cpp:4388-4400) char their victims; the potato mine's
    mine-pop does NOT (Plant.cpp:4437-4452, theBurn = false -> plain
    TakeDamage(1800) -- the victim plays its ordinary death, leaves no ash).*/
    const bool burn = plant.get_attack_range() != ONE_X_ONE;
    for (int j = 0; j < (int)zombies.size(); j++)
        if (check_in_attack_range(plant, *zombies[j]))
        {
            // 90 peas ~= the 1800 raw instant-kill damage of GOTY explosions;
            // zombies that can't be hit right now are skipped, and anything
            // tough enough to survive (armored, vulnerable boss) stays alive
            if (!zombies[j]->take_hit(90, zombie_parts))
                continue;
            if (burn)
                push_burned_corpse(zombie_parts, zombies[j]);
            else
                zombies[j]->add_zombie_die(zombie_parts); // mine pop: ordinary death
            delete zombies[j];
            zombies.erase(zombies.begin() + j);
            j--;
        }
}
