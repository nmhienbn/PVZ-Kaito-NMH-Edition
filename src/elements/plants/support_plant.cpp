#include "support_plant.hpp"

/*
A support plant is planted UNDER the main plant on the same tile: the zombie
eats the main plant first, and the tile stays planted/blocked while either of
them is alive.
*/
bool is_support_plant(const int &type)
{
    return type == LILYPAD_TYPE || type == FLOWERPOT_TYPE;
}

Plants *find_support_plant(vector<Plants *> &plants, const int &row, const int &col)
{
    for (auto &plant : plants)
        if (plant->get_row() == row && plant->get_col() == col &&
            is_support_plant(plant->get_type()))
            return plant;
    return nullptr;
}

Plants *find_main_plant(vector<Plants *> &plants, const int &row, const int &col)
{
    for (auto &plant : plants)
        if (plant->get_row() == row && plant->get_col() == col &&
            !is_support_plant(plant->get_type()))
            return plant;
    return nullptr;
}

/*
The tile's main plant counting slot occupancy: the pumpkin shell wraps the
plant rather than filling the slot, so it does not count here (original
Board::CanPlantAt treats SEED_PUMPKINSHELL as a separate cell slot).
*/
Plants *find_inner_plant(vector<Plants *> &plants, const int &row, const int &col)
{
    for (auto &plant : plants)
        if (plant->get_row() == row && plant->get_col() == col &&
            !is_support_plant(plant->get_type()) && plant->get_type() != PUMPKIN_TYPE)
            return plant;
    return nullptr;
}
