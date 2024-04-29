#include "AttackPlantVsZombie.hpp"
#include "elements/Map/Map.hpp"

extern Map cells;

bool check_in_attack_range(const Plants &plant, const Zombie &zombie)
{
    switch (plant.get_attack_range())
    {
    case ONE_ROW_AHEAD:
    {
        if (plant.get_row() == zombie.get_row() &&
            is_in(cells[0][plant.get_col()].x2 - 140, zombie.x_location, ZOMBIE_INIT_X - ZOMBIE_EXACT_LOCATION - 20))
            return true;
        else
            return false;
    }
    case THREE_X_THREE:
    {
        int right_limit = cells[plant.get_row()][plant.get_col()].x2 + BLOCK_WIDTH;
        int left_limit = cells[plant.get_row()][plant.get_col()].x1 - BLOCK_WIDTH - 50;
        if (is_in(plant.get_row() - 1, zombie.row, plant.get_row() + 1) &&
            is_in(left_limit, zombie.x_location + ZOMBIE_EXACT_LOCATION + 15, right_limit))
            return true;
        return false;
    }
    case ONE_X_ONE:
    {
        int right_limit = cells[plant.get_row()][plant.get_col()].x2 - 25;
        int left_limit = cells[plant.get_row()][plant.get_col()].x1 - 50;
        if (plant.get_row() == zombie.get_row() &&
            is_in(left_limit, zombie.x_location + ZOMBIE_EXACT_LOCATION + 15, right_limit))
            return true;
        return false;
    }
    default:
        return false;
    }
    return false;
}

bool check_in_attack_range(const Plants &plant, const vector<Zombie *> &zombies)
{
    for (const auto &zombie : zombies)
        if (check_in_attack_range(plant, *zombie))
            return true;
    return false;
}
