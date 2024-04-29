#pragma once
#include "elements/SeedPacket/SeedPacket.hpp"

struct Player
{
    string name;
    int sun_count;
    int level;
    bool is_shoveling;
    int unlocked_level = 0;
    vector<SeedPacket> seed_packets;

    int sun_count_change_color_time = 0;
    int sun_count_change_color_times = 0;

    bool is_choosing_a_plant();
};