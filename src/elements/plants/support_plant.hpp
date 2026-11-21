#pragma once
// Support plants: one plant that carries another on the same tile.
//   - Lily Pad (pool): lives on water, lets a regular ("main") plant ride on
//     top; zombies eat the rider first, the pad last.
//   - Flower Pot (roof, roof agent reuses this): same stacking on roof tiles.
//   - Pumpkin shell: wraps AROUND the tile's plants. It is its own slot (the
//     original keeps it as the cell's mPumpkinPlant): zombies chew it first,
//     it never blocks planting the missing main plant into a shell, and a
//     second shell on the same tile is refused.
// A tile can therefore hold at most ONE support + ONE main plant + ONE shell.
// Everything keys on the helpers below -- planting rules (player_click.cpp),
// the zombie bite target (ZombieVsPlant.cpp) and the tile-flag cleanup
// (plants_action.cpp).
#include "plants.hpp"
#include "game_param.hpp"

bool is_support_plant(const int &type);

Plants *find_support_plant(vector<Plants *> &plants, const int &row, const int &col);
Plants *find_main_plant(vector<Plants *> &plants, const int &row, const int &col);
Plants *find_inner_plant(vector<Plants *> &plants, const int &row, const int &col);

inline bool cell_has_support(vector<Plants *> &plants, const int &row, const int &col)
{
    return find_support_plant(plants, row, col) != nullptr;
}
inline bool cell_has_main(vector<Plants *> &plants, const int &row, const int &col)
{
    return find_main_plant(plants, row, col) != nullptr;
}
// The "real" plant inside the shell: any main plant that is not a pumpkin.
inline bool cell_has_inner(vector<Plants *> &plants, const int &row, const int &col)
{
    return find_inner_plant(plants, row, col) != nullptr;
}
