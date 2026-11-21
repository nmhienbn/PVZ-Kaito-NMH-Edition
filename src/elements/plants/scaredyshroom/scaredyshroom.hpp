#pragma once
#include "../plants.hpp"
#include "elements/bullets/puff/puff.hpp"

class Zombie;

class ScaredyShroom : public Plants
{
public:
    ScaredyShroom(const int &_row, const int &_col);
    ~ScaredyShroom();
    bool is_night() const;
    bool is_scared_now(const vector<Zombie *> &zombies);
    void fire_puff(vector<Bullet *> &bullets);
    void next_frame(const PlantAssets &assets) override;
    void determine_appearance(bool check_zombie_in_attack_range, bool scared);
    bool is_down() const; // cowering/ducking: no shooting, still edible
    void display(const int &_row) override;
    void action() override;

private:
    int state_ticks; // countdown for the one-shot scared/raise segments
};
