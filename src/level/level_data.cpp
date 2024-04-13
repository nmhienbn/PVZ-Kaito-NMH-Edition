#include "level/level_data.hpp"

extern Level level;
extern Elements game_characters;
extern Icons icons;
extern Player player;
extern Map cells;

static const map<string, int> zombie_type_map = {
    {"normal", NORMAL_TYPE},
    {"flag", FLAG_TYPE},
    {"conehead", CONE_TYPE},
    {"buckethead", BUCKET_TYPE},
    {"door", DOOR_TYPE}};

/*Reset all things of level.*/
void reset_level()
{
    level.reset();
    game_characters.reset();

    for (int y = 0; y < VERT_BLOCK_COUNT; y++)
    {
        for (int x = 0; x < HORIZ_BLOCK_COUNT; x++)
        {
            cells[y][x].is_planted = false;
            cells[y][x].is_potate_mine = false;
        }
    }
    icons = Icons();
}

/*
Read level's information from level files:
    + Map type
    + Number of wave
    + Number of zombie in each wave
    + Duration of each wave
Updated: number and type of zombie each wave.
*/
void read_level()
{
    string map_typ, wave_cnt, zombie_seq, wave_dur, temp;
    string file_name = LEVELS_DIRECTORY;
    file_name += "level";                    // levels folder
    file_name += to_string(level.level_num); // level num
    file_name += ".json";                    // level extension
    ifstream myfile(file_name);
    auto j = json::parse(myfile);

    // Map type
    auto objects = j["objects"];
    level.map_type = objects["map_type"];
    --level.map_type;
    level.has_sun_from_sky = objects["has_sun_from_sky"];

    // Number of wave
    auto jwaves = j["waves"];
    level.wave_count = jwaves.size();

    // Number of zombie in each wave
    decide_zombie_each_wave(level.waves, jwaves);

    // Reset cur wave
    level.cur_wave = 0;
    level.waves_finished = false;
}

#include "elements/zombies/wave.hpp"
/*Random number of zombie appear in each second of the wave.*/
void decide_zombie_each_wave(vector<Wave> &waves, json &jwaves)
{
    waves.assign(level.wave_count, Wave());
    level.count_zombie_type.assign(COUNT_ZOMBIE_TYPE, 0);
    for (int i = 0; i < level.wave_count; i++)
    {
        waves[i].has_flag = jwaves[i].count("has_flag");
        waves[i].delay_time = jwaves[i]["delay_time"];

        int x_bias_zombie = 0;
        if (waves[i].has_flag)
        {
            Zombie *zombie = init_zombie(FLAG_TYPE, level.level_num);
            waves[i].add_zombie(zombie);
            level.final_wave = i;

            // Lui zombie phia sau
            x_bias_zombie = 100;
        }
        for (const auto &z : jwaves[i]["zombies"])
        {
            for (int j = 0; j < z["count"]; j++)
            {
                int typ = zombie_type_map.at(z["type"]);
                Zombie *zombie = init_zombie(typ, level.level_num, x_bias_zombie);
                level.count_zombie_type[typ]++;
                waves[i].add_zombie(zombie);
            }
        }

        // Count total zombie used for this level - for progress bar.
        level.zombie_count += waves[i].zombie_count();

        // First wave of zombie
        level.next_wave_clk = waves[0].delay_time;
    }
}

/*
Reset level.
Read level.
Random zombie appear order.
Init mower.
*/
void load_level()
{
    reset_level();
    read_level();
    // cout << level.level_num << '\n';

    if (level.level_num == 1)
        init_mower(game_characters.mowers, 2, 2);
    else if (level.level_num == 2)
        init_mower(game_characters.mowers, 1, 3);
    else
        init_mower(game_characters.mowers, 0, 4);
    player.sun_count = INIT_SUN_COUNT;
    player.is_choosing_a_plant = false;
    player.is_shoveling = false;
    level.zombie_has_coming = false;
}
