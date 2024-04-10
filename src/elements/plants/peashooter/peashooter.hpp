#pragma once
#include "../plants.hpp"
#include "elements/bullets/pea/pea.hpp"

class Peashooter : public Plants
{
public:
    Peashooter(const int &_row, const int &_col);
    ~Peashooter();
    void determine_appearance(vector<Zombie *> &zombies);
    void display(const int &_row) override;
    void action() override;
    void fire_pea(vector<Zombie *> &zombies, vector<Bullet *> &bullets);
};