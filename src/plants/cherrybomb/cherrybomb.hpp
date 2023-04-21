#pragma once
#include "draw/rsdl.hpp"
#include "music/music.hpp"
#include "zombies/zombie_struct.hpp"
#include "plants/plants.hpp"

class CherryBomb : public Plants
{
public:
    CherryBomb(const int &_row, const int &_col);
    ~CherryBomb();
    void display(const int &_row) override;
    bool is_hit_zombie(Zombie &zombie);
    bool is_blow();
    bool is_disappeared();
    bool apply_hitting_zombie(vector<Zombie> &zombies, const int &z_ind,
                              vector<ZombiePart> &zombie_parts);
    void hit_all_zombies(vector<Zombie> &zombies,
                         vector<ZombiePart> &zombie_parts);
};