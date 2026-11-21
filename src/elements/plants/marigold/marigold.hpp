#pragma once
#include "../plants.hpp"

/*
Marigold: the PvZ1 money flower. The clone has no coin/money system, so its
production is a money pickup worth 10 sun (a silver coin's worth, collected
through the usual sun-click pipeline) on the marigold bloom cycle, drawn
with the retail silver-dollar art + coin glow.
*/
class Marigold : public Plants
{
public:
    Marigold(const int &_row, const int &_col);
    ~Marigold();
    void display(const int &_row) override;
    void action() override;

private:
    bool producing() const { return directory_num == MARIGOLD_PRODUCE_DIRECTORY; }
};

// A silver coin's worth, paid as sun (no money HUD in this clone).
#define COIN_SUN_VALUE 10
// Bloom cycle between coin drops (~24 s, the original marigold cadence).
// First bloom runs on a quicker clock after planting.
#define MARIGOLD_PRODUCE_CLK 1440
