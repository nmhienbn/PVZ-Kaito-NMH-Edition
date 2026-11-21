#pragma once
#include "../plants.hpp"

// Garlic bite-count sturdiness (plants.json health = 8 bites).
#define GARLIC_STAGE2_HEALTH 5 // below 2/3 of the 8 bites: second look
#define GARLIC_STAGE3_HEALTH 2 // below 1/3: third look

/*
Garlic: standing wall with a gag reflex. Every surviving bite starts the
yucky beat on the biter (repvz Zombie.cpp:4851-4858 sets mYuckyFace, and
UpdateYuckyFace 4738-4824 runs it): the zombie keeps chewing ~0.7 s, gags in
place for ~1 s (in the original the head swaps to the gross-out image -- the
clone's art set ships no gross-out face, so the beat is mechanical only),
then slides into the adjacent lane and walks on; ~1 s later the face would
clear. The bulb itself survives 8 bites in three looks (this art set ships a
single garlic body, so the degrade reads as progressive greening, tinted per
stage in reanim_plant.cpp).
*/
class Garlic : public Plants
{
public:
    Garlic(const int &_row, const int &_col);
    ~Garlic();
    void display(const int &_row) override;
    void action() override;

private:
    int stage_dir() const;
    const PlantAssets &stage_assets() const;
};

// Garlic divert beat, driven from the zombie update loop (ZombieVsPlant.cpp):
//  - garlic_bite: arm the yucky timer on the biting zombie (no-op if a gag is
//    already in progress, like the original's `if (!mYuckyFace)`).
//  - update_garlic_yucky: tick every gagging zombie once per frame -- holds it
//    in place during the face beat, diverts the row at the turn, cleans up
//    dead entries.
class Zombie;
void garlic_bite(Zombie &zombie);
void update_garlic_yucky(vector<Zombie *> &zombies);
/*Test/debug hook: true while the zombie is in the held (gagging) window.*/
bool garlic_gagging(const Zombie *zombie);
