#include "makeLevels.h"

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
            temp.x2 = X_UPPER_LEFT + ((x + 1) * BLOCK_WIDTH);
            temp.y1 = Y_UPPER_LEFT + (y * BLOCK_HEIGHT);
            temp.y2 = Y_UPPER_LEFT + ((y + 1) * BLOCK_HEIGHT);

            temps.push_back(temp);
        }
        result.push_back(temps);
    }
    return result;
}

/*
Read player saved data:
    + Player name:
    + Unlocked levels:
*/
void read_savedata(Player &player, Level &level)
{
    string line;
    std::ifstream myfile("savedata.txt");
    if (myfile.is_open())
    {
        getline(myfile, line);
        player.name = line;
        getline(myfile, line);
        level.level_num = stoi(line);
        myfile.close();
    }
    else
        std::cout << "Unable to open saved data file";
}

/*Need update: number and type of zombie each wave.
Read level's information from level files:
    + Number of wave
    + Number of zombie in each wave
    + Duration of each wave
*/
void read_level(Level &level)
{
    string wave_cnt, zombie_seq, wave_dur, temp;
    string file_name = "";
    file_name.push_back(level.level_num + '0');
    file_name += ".level.txt";
    std::ifstream myfile(file_name);
    int num_ind = 0;
    getline(myfile, wave_cnt);
    getline(myfile, zombie_seq);
    getline(myfile, wave_dur);

    save_wave_cnt_str_as_int(level, wave_cnt);
    save_zombie_seq_str_as_int_vect(level, zombie_seq);
    save_wave_dur_str_as_int_vect(level, wave_dur);
    level.cur_wave = 0;
    level.cur_sec = 0;
    level.waves_finished = false;
}

/*
Get information of number of waves in file.
*/
void save_wave_cnt_str_as_int(Level &level, string wave_cnt)
{
    string temp;
    int num_ind = wave_cnt.find(":") + 2;
    temp = wave_cnt.substr(num_ind, wave_cnt.size() - num_ind);
    level.wave_count = stoi(temp);
}

/*
Get information of number of zombie in each wave in file.
*/
void save_zombie_seq_str_as_int_vect(Level &level, string zombie_seq)
{
    string temp;
    int num_ind = zombie_seq.find(":") + 2;
    int space_ind = zombie_seq.find(" ", num_ind);
    for (int i = 0; i < level.wave_count; i++)
    {
        temp = zombie_seq.substr(num_ind, space_ind - num_ind);
        level.wave_zombie_count.push_back(stoi(temp));
        num_ind = space_ind + 1;
        space_ind = zombie_seq.find(" ", num_ind);
    }
}

/*
Get information of duration of wave in file.
*/
void save_wave_dur_str_as_int_vect(Level &level, string wave_dur)
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

/*
Initialize game:
    Display loading and starting game.
    Read player data
    Read level data
    Random zombie appear time
    Create block on frontyard (playground)
    Init player's sun
*/
void init_game(window &win, Level &level, Player &player, Map &map)
{
    desplay_starting_screen(win);
    read_savedata(player, level);
    read_level(level);
    decide_zombie_cnt_for_each_sec(level);
    map = create_a_collection_of_blocks();
    player.sun_count = INIT_SUN_COUNT;
    player.is_choosing_a_plant = false;
}

/*
Random number of zombie appear in each second of the wave.
*/
void decide_zombie_cnt_for_each_sec(Level &level)
{
    srand(time(NULL));
    bool enough_zombies = false;
    int z_cnt, sum;

    for (int wave = 0; wave < level.wave_count; wave++)
    {
        vector<int> temp;
        enough_zombies = false;

        for (int sec = 0; sec < level.wave_duration[wave]; sec++)
        {
            z_cnt = (rand() % 5) + 1;
            sum = 0;
            for (int i = 0; i < temp.size(); i++)
                sum += temp[i];

            if (enough_zombies)
                temp.push_back(0);

            else
            {
                if (z_cnt + sum <= level.wave_zombie_count[wave])
                    temp.push_back(z_cnt);
                else
                {
                    temp.push_back(level.wave_zombie_count[wave] - sum);
                    enough_zombies = true;
                }
            }
        }
        level.zombie_distr_for_wave.push_back(temp);
    }
}

/*Need update: Fake loading screen
Display starting screen
Updated: Now player can exit game from here.
*/
void desplay_starting_screen(window &win)
{
    bool game_started = false;
    bool quit = false;
    while (!quit && !game_started)
    {
        win.draw_png_scale(STARTING_SCREEN_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        HANDLE(
            QUIT(quit = true; exit(0););
            // KEY_PRESS(q, quit = true);
            LCLICK({
                if (user_clicked_on_start(mouse_x, mouse_y))
                    game_started = true;
            });

        );

        win.update_screen();
        DELAY(10);
    }
}

/*
If player click on start game: return true
*/
bool user_clicked_on_start(int mouse_x, int mouse_y)
{
    if (mouse_x > TAP_TO_START_X1 && mouse_y < TAP_TO_START_X2 &&
        mouse_y > TAP_TO_START_Y1 && mouse_y < TAP_TO_START_Y2)
        return true;
    return false;
}

/*
Check if player has lost: Any zombie go to the house
*/
bool has_player_lost(Elements &elements)
{
    for (int i = 0; i < elements.zombies.size(); i++)
    {
        if (elements.zombies[i].x_location < X_UPPER_LEFT + 5)
            return true;
    }
    return false;
}

/*
Check if player won: no wave or zombie left.
*/
bool has_player_won(Level &level, Elements &elements)
{
    if (level.waves_finished && elements.zombies.size() == 0)
        return true;
    return false;
}
