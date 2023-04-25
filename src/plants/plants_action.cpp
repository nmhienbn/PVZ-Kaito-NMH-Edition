#include "elements/elements.hpp"
#include "plants.hpp"

extern Elements game_characters;
extern Map cells;

void Peashooter::action()
{
    fire_pea(game_characters.zombies, game_characters.peas);
}

void Sunflower::action()
{
    gen_sun_from_a_sunflower(game_characters.suns);
}

void Walnut::action()
{
}

void Snowpea::action()
{
    fire_pea(game_characters.zombies, game_characters.peas);
}

void CherryBomb::action()
{
    hit_all_zombies(game_characters.zombies, game_characters.zombie_parts);
    disappear();
}

void delete_plant(vector<Plants *> &plants, int &idx)
{
    cells[plants[idx]->get_row()][plants[idx]->get_col()].is_planted = false;
    delete plants[idx];
    plants.erase(plants.begin() + idx);
    idx--;
}

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