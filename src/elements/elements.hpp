/*
This file include game elements: Plants, Zombies, Player, Level (Plant seeds)
*/
#pragma once
#include "elements/mower/mower.hpp"
#include "elements/plants/plant_type.hpp"
#include "elements/zombies/zombie_struct.hpp"

/*
Contain vector of all plants, zombies, bullets, mowers, suns
*/
struct Elements
{
    vector<Plants *> plants;
    vector<Bullet *> bullets;
    vector<Mower *> mowers;

    vector<Zombie *> zombies;
    vector<ZombiePart> zombie_parts;

    vector<Sun> suns;

    void reset();
};
