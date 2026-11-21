#pragma once
// Shared tile/plant helpers for the roof zombie classes (bungee steal,
// gargantuar/catapult/boulder crush, ladder defensive-plant lookup).

#include "elements/Map/Map.hpp"
#include "elements/elements.hpp"
#include "elements/plants/plants.hpp"
#include "elements/plants/support_plant.hpp"

extern Elements game_characters;
extern Map cells;

// First plant index sitting on tile (row, col), preferring the main plant
// over a flower-pot/lily-pad support and over a pumpkin shell; -1 when the
// tile is empty (a lone pot/pad/shell can still be the victim).
inline int plant_index_at(const int &row, const int &col)
{
    int shell = -1, support = -1;
    for (int i = 0; i < (int)game_characters.plants.size(); i++)
    {
        if (game_characters.plants[i]->get_row() != row ||
            game_characters.plants[i]->get_col() != col)
            continue;
        if (game_characters.plants[i]->get_type() == PUMPKIN_TYPE)
            shell = i;
        else if (is_support_plant(game_characters.plants[i]->get_type()))
            support = i;
        else
            return i;
    }
    return shell >= 0 ? shell : support;
}

// Vehicle-style crush: destroy every plant on the tile (shell + content).
// Returns true when something was crushed.
inline bool crush_plants_at(const int &row, const int &col)
{
    bool crushed = false;
    for (int i = 0; i < (int)game_characters.plants.size();)
    {
        if (game_characters.plants[i]->get_row() == row &&
            game_characters.plants[i]->get_col() == col)
        {
            delete_plant(game_characters.plants, i);
            i++;
            crushed = true;
        }
        else
        {
            i++;
        }
    }
    return crushed;
}
