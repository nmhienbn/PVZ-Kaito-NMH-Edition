#pragma once
#include "elements/plants/Plants.hpp"

extern const int level_unlock_new_plant[];

class SeedPacket
{

private:
    int x, y;
    int sun_cost;
    int loading_time;

public:
    PlantType plant_name;
    int remaining_time;

    static PlantType chosen_plant;

    SeedPacket(int plant_name);
    void display(int x, int y, int player_sun_count);
    void reset_remaining_time();
    void blink(int x, int y) const;
};