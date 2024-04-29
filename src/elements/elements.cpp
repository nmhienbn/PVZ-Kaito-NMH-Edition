#include "elements.hpp"

/*Clear all elements*/
void Elements::reset()
{
    for (auto &plant : plants)
    {
        delete plant;
    }
    plants.clear();

    suns.clear();
    bullets.clear();

    zombies.clear();
    zombie_parts.clear();
}