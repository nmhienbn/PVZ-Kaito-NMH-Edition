#include "elements.hpp"
#include "plants/jalapeno/jalapeno_fire.hpp"
#include "reanim/one_shot_fx.hpp"

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

    reanimfx::clear();
    jalapeno_fx::clear();
}