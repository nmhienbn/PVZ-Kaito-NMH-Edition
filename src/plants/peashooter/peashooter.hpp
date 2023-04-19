#pragma once
#include "draw/rsdl.hpp"
#include "music/music.hpp"
#include "elements/bullets/pea/pea.hpp"
#include "plants/plants.hpp"

class Peashooter : public Plants
{
public:
    Peashooter(const int &_row, const int &_col);
    ~Peashooter();
    void determine_appearance(vector<Zombie> &zombies);
    void display(const int &_row) override;
    void fire_pea(vector<Zombie> &zombies, vector<Pea> &peas);
};

void fire_peas(vector<Peashooter> &peashooters, vector<Zombie> &zombies, vector<Pea> &peas);