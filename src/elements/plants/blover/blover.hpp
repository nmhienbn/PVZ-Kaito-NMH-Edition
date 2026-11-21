#pragma once
#include "../plants.hpp"

class Zombie;
class ZombiePart;

// Blover: single-use. Spins up (anim_blow), blasts once — wipes the fog for
// fogov::FOG_BLOWN_TICKS and pops every flying balloon zombie — then withers.
class Blover : public Plants
{
public:
    Blover(const int &_row, const int &_col);
    ~Blover();
    void display(const int &_row) override;
    void action() override;
    void blow(vector<Zombie *> &zombies, vector<ZombiePart> &zombie_parts);

private:
    bool blown = false; // the blast fired once already
};
