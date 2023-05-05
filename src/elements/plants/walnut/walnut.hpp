#pragma once
#include "draw/rsdl.hpp"
#include "game_stats.hpp"
#include "elements/Map/Map.hpp"
#include "../plants.hpp"

class Walnut : public Plants
{
public:
    Walnut(const int &_row, const int &_col);
    ~Walnut();
    void determine_appearance();
    void display(const int &_row) override;
    void action() override;
};
