#include "elements/elements.hpp"
#include "plants.hpp"

extern Elements game_characters;
extern Map cells;

/*
Peashooter's action: fire pea
*/
void Peashooter::action()
{
    fire_pea(game_characters.zombies, game_characters.peas);
}

/*
Sunflower's action: gen sun
*/
void Sunflower::action()
{
    gen_sun_from_a_sunflower(game_characters.suns);
}

/*
Wall-nut's action: nothing
*/
void Walnut::action()
{
}

/*
Snow Pea's action: fire snow pea
*/
void Snowpea::action()
{
    fire_pea(game_characters.zombies, game_characters.peas);
}

/*
Cherry Bomb's action: blow up zombies
*/
void CherryBomb::action()
{
    hit_all_zombies(game_characters.zombies, game_characters.zombie_parts);
    disappear();
}

/*
Delete idx-th plant in vector<plants*>
*/
void delete_plant(vector<Plants *> &plants, int &idx)
{
    cells[plants[idx]->get_row()][plants[idx]->get_col()].is_planted = false;
    delete plants[idx];
    plants.erase(plants.begin() + idx);
    idx--;
}

/*
Move plants to next status.
Do all plants' actions.
*/
void update_plants_status(vector<Plants *> &plants)
{
    for (int i = 0; i < (int)game_characters.plants.size(); i++)
    {
        auto plant = game_characters.plants[i];
        plant->action();
        if (plant->is_died())
        {
            delete_plant(game_characters.plants, i);
        }
    }
}