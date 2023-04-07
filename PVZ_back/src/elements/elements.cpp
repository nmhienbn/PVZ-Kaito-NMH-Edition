#include "elements/elements.hpp"

Icons::Icons()
{
    for (int i = 0; i < PLANT_COUNT; i++)
    {
        is_plant_chosen[i] = 0;
        plant_remaining_time[i] = 0;
    }
}

void Icons::reset_is_chosen()
{
    for (int i = 0; i < PLANT_COUNT; i++)
    {
        is_plant_chosen[i] = 0;
    }
}

/*Check if this wave is huge*/
bool Level::is_huge_wave()
{
    int z_cnt = 0;
    for (int i = NORMAL_TYPE; i < COUNT_ZOMBIE_TYPE; i++)
    {
        z_cnt += wave_zombie_count[i][cur_wave];
    }
    return z_cnt >= 5;
}

void Level::reset()
{
    wave_count = 0;
    cur_wave = 0;
    cur_sec = 0;
    waves_finished = false;
    zombie_has_coming = false;
    zombie_count = 0;
    used_zombie_count = 0;

    announce_directory = NULL_DIRECTORY;

    for (int i = NORMAL_TYPE; i < COUNT_ZOMBIE_TYPE; i++)
    {
        zombie_distr_for_wave[i].clear();
        wave_zombie_count[i].clear();
    }
    wave_duration.clear();
}

/*
Create 5 x 9 tiles
*/
Map create_a_collection_of_blocks()
{
    Map result;
    vector<Block> temps;
    Block temp;
    for (int y = 0; y < VERT_BLOCK_COUNT; y++)
    {
        temps.clear();
        for (int x = 0; x < HORIZ_BLOCK_COUNT; x++)
        {
            temp.x1 = X_UPPER_LEFT + (x * BLOCK_WIDTH);
            temp.x2 = temp.x1 + BLOCK_WIDTH;
            temp.y1 = Y_UPPER_LEFT + (y * BLOCK_HEIGHT);
            temp.y2 = temp.y1 + BLOCK_HEIGHT;

            temps.push_back(temp);
        }
        result.push_back(temps);
    }
    return result;
}

void Elements::reset()
{
    peashooters.clear();
    sunflowers.clear();
    walnuts.clear();
    snowpeas.clear();
    cherrybombs.clear();

    suns.clear();
    peas.clear();

    zombies.clear();
    dead_zombies.clear();
}
