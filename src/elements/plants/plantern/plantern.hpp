#pragma once
#include "../plants.hpp"

// Plantern: pure utility — clears the fog around itself (draw/fog_overlay.cpp
// reads every PLANTERN_TYPE plant off game_characters.plants to punch holes
// in the fog bank). No attack.
class Plantern : public Plants
{
public:
    Plantern(const int &_row, const int &_col);
    ~Plantern();
    void display(const int &_row) override;
    void action() override {} // light only; the fog overlay polls plant positions
};
