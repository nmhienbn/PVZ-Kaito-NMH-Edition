#include "level/level_data.hpp"

extern Level level;
extern Elements game_characters;
extern Icons icons;
extern Player player;
extern Map cells;

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
    string file_name = LEVELS_DIRECTORY;     // levels folder
    file_name += to_string(level.level_num); // level num
    file_name += ".level.txt";               // level extension
    ifstream myfile(file_name);

    // Map type
    getline(myfile, map_typ);
    level.map_type = map_typ.back() - '1';
    if (level.map_type == 3)
        level.is_night = true;
    else
        level.is_night = false;

    // Number of wave
    getline(myfile, wave_cnt);
    convert_wave_cnt_str_into_int(wave_cnt);

    // Number of zombie in each wave
    getline(myfile, wave_dur);
    convert_wave_dur_str_into_int_vect(wave_dur);

    // Duration of each wave
    for (int typ = NORMAL_TYPE; typ < COUNT_ZOMBIE_TYPE; typ++)
    {
        zombie_seq = "";
        if (typ != FLAG_TYPE)
        {
            getline(myfile, zombie_seq);
        }
        convert_zombie_seq_str_into_int_vect(zombie_seq, typ);
    }
    // Reset cur wave & cur sec
    level.cur_wave = 0;
    level.cur_sec = 0;
    level.waves_finished = false;
}

/*Get information of number of waves in file.*/
void convert_wave_cnt_str_into_int(const string &wave_cnt)
{
    string temp;
    int num_ind = wave_cnt.find(":") + 2;
    temp = wave_cnt.substr(num_ind, wave_cnt.size() - num_ind);
    level.wave_count = stoi(temp);
}

/*Get information of duration of wave in file.*/
void convert_wave_dur_str_into_int_vect(const string &wave_dur)
{
    string temp;
    int num_ind = wave_dur.find(":") + 2;
    int space_ind = wave_dur.find(" ", num_ind);
    for (int i = 0; i < level.wave_count; i++)
    {
        temp = wave_dur.substr(num_ind, space_ind - num_ind);
        level.wave_duration.push_back(stoi(temp));
        num_ind = space_ind + 1;
        space_ind = wave_dur.find(" ", num_ind);
    }
}

/*Get information of number of zombie in each wave in file.*/
void convert_zombie_seq_str_into_int_vect(const string &zombie_seq, const int &typ)
{
    level.wave_zombie_count[typ].assign(level.wave_count, 0);
    if (typ == FLAG_TYPE)
    {
        return;
    }
    string temp;
    int num_ind = zombie_seq.find(":") + 2;
    int space_ind = zombie_seq.find(" ", num_ind);
    for (int i = 0; i < level.wave_count; i++)
    {
        temp = zombie_seq.substr(num_ind, space_ind - num_ind);
        level.wave_zombie_count[typ][i] = stoi(temp);
        num_ind = space_ind + 1;
        space_ind = zombie_seq.find(" ", num_ind);
    }
}

/*Random number of zombie appear in each second of the wave.*/
void decide_zombie_cnt_for_each_sec()
{
    bool enough_zombies = false;
    int z_cnt, sum;
    for (int typ = NORMAL_TYPE; typ < COUNT_ZOMBIE_TYPE; typ++)
    {
        // skip flag type (which depend on wave is huge or not)
        if (typ == FLAG_TYPE)
        {
            for (int wave = 0; wave < level.wave_count; wave++)
            {
                vector<int> temp(level.wave_duration[wave], 0);
                level.zombie_distr_for_wave[typ].push_back(temp);
                level.wave_zombie_count[typ][wave] = 0;
            }
            continue;
        }
        // Loop each wave
        for (int wave = 0; wave < level.wave_count; wave++)
        {
            // assign vector of number of zombie
            vector<int> temp(level.wave_duration[wave], 0);
            // reset flag
            enough_zombies = false;
            // count number of used zombies
            sum = 0;
            // average zombie for each wave
            int aver = level.wave_zombie_count[typ][wave];
            if (level.wave_duration[wave])
                aver = max(1, aver / level.wave_duration[wave] + 1);

            for (int sec = 0; sec < level.wave_duration[wave]; sec++)
            {
                // random number of zombie for this second
                z_cnt = rand((typ == NORMAL_TYPE), aver);
                if (enough_zombies)
                    temp[sec] = 0;
                else
                {
                    if (z_cnt + sum <= level.wave_zombie_count[typ][wave])
                        temp[sec] = z_cnt;
                    else
                    {
                        temp[sec] = level.wave_zombie_count[typ][wave] - sum;
                        enough_zombies = true;
                    }
                }
                // update number of used
                sum += temp[sec];
            }
            // push a vector of number zombie for this wave
            level.zombie_distr_for_wave[typ].push_back(temp);
            level.wave_zombie_count[typ][wave] = sum;
        }
    }
    // Count total zombie used for this level - for progress bar.
    for (int i = 0; i < level.wave_count; i++)
    {
        int tmp = 0;
        for (int typ = NORMAL_TYPE; typ < COUNT_ZOMBIE_TYPE; typ++)
        {
            tmp += level.wave_zombie_count[typ][i];
        }
        level.zombie_count += tmp;
        if (tmp >= 5 || i == level.wave_count - 1)
        {
            level.zombie_count++;
        }
    }
}

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
        }
    }
    icons = Icons();
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
    decide_zombie_cnt_for_each_sec();
    if (level.level_num == 1)
        init_mower(2, 2);
    else if (level.level_num == 2)
        init_mower(1, 3);
    else
        init_mower(0, 4);
    player.sun_count = INIT_SUN_COUNT;
    player.is_choosing_a_plant = false;
    player.is_shoveling = false;
    level.zombie_has_coming = false;
    level.last_clk_zombie_appear = 0;
}