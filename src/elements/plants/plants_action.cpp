#include "elements/collision/AttackPlantVsZombie.hpp"
#include "elements/collision/InstancePlantVsZombie.hpp"
#include "elements/elements.hpp"
#include "plants.hpp"

extern Elements game_characters;
extern Map cells;

/*
Peashooter's action: fire pea
*/
void Peashooter::action()
{
    fire_pea(game_characters.bullets);
    bool check_zombie_in_attack_range = check_in_attack_range(*this, game_characters.zombies);
    determine_appearance(check_zombie_in_attack_range);
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
void WallNut::action()
{
}

/*
Snow Pea's action: fire snow pea
*/
void SnowPea::action()
{
    fire_pea(game_characters.bullets);
    determine_appearance(check_in_attack_range(*this, game_characters.zombies));
}

/*
Cherry Bomb's action: blow up zombies
*/
void CherryBomb::action()
{
    if (is_blow())
    {
        play_sound_effect(CHERRYBOMB_MUSIC_DIRECTORY);
        apply_instance_plant_hit_zombie(*this, game_characters.zombies, game_characters.zombie_parts);
    }
    disappear();
}

void PotatoMine::action()
{
    if (state == ARMED && check_in_attack_range(*this, game_characters.zombies))
    {
        state = ATTACK;
        frame = 0;
    }
    if (is_blow())
    {
        play_sound_effect(POTATOMINE_MUSIC_DIRECTORY);
        apply_instance_plant_hit_zombie(*this, game_characters.zombies, game_characters.zombie_parts);
    }
    disappear();
}

/*
Delete idx-th plant in vector<plants*>
*/
void delete_plant(vector<Plants *> &plants, int &idx)
{
    cells[plants[idx]->get_row()][plants[idx]->get_col()].is_planted = false;
    cells[plants[idx]->get_row()][plants[idx]->get_col()].is_potate_mine = false;
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