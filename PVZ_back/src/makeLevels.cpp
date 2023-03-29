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
    std::ifstream myfile(SAVED_DATA_DIRECTORY);
    if (myfile.is_open())
    {
        getline(myfile, line);
        player.name = line;
        getline(myfile, line);
        player.unlocked_level = stoi(line);
        myfile.close();
    }
    else
        std::cout << "Unable to open saved data file!\n";
}

void update_unlocked_level(Player &player, Level &level)
{
    // Read data
    string line;
    ifstream infile(SAVED_DATA_DIRECTORY);
    if (infile.is_open())
    {
        getline(infile, line);
        player.name = line;
        getline(infile, line);
        player.unlocked_level = stoi(line);
        infile.close();
    }
    else
        cout << "Unable to open saved data file!\n";

    // Write data
    ofstream outfile(SAVED_DATA_DIRECTORY);
    if (outfile.is_open())
    {
        int num_level = std::max(1, player.unlocked_level - 1);
        if (num_level < level.level_num)
            num_level = level.level_num;
        player.unlocked_level = num_level + 1;
        outfile << player.name << '\n'
                << player.unlocked_level;
        outfile.close();
    }
    else
        cout << "Unable to open saved data file!\n";
}

void reset_unlocked_level(Player &player)
{
    // Read data
    string line;
    ifstream infile(SAVED_DATA_DIRECTORY);
    if (infile.is_open())
    {
        getline(infile, line);
        player.name = line;
        getline(infile, line);
        player.unlocked_level = stoi(line);
        infile.close();
    }
    else
        cout << "Unable to open saved data file!\n";

    // Write data
    ofstream outfile(SAVED_DATA_DIRECTORY);
    if (outfile.is_open())
    {
        player.unlocked_level = 1;
        outfile << player.name << '\n'
                << player.unlocked_level;
        outfile.close();
    }
    else
        cout << "Unable to open saved data file!\n";
}

/*Need update: number and type of zombie each wave.
Read level's information from level files:
    + Number of wave
    + Number of zombie in each wave
    + Duration of each wave
*/
void read_level(Level &level)
{
    string map_typ, wave_cnt, zombie_seq, wave_dur, temp;
    string file_name = LEVELS_DIRECTORY;          // levels folder
    file_name += std::to_string(level.level_num); // level num
    file_name += ".level.txt";                    // level extension
    std::ifstream myfile(file_name);

    getline(myfile, map_typ);
    level.map_type = map_typ.back() - '1';
    if (level.map_type == 3)
        level.is_night = true;
    else
        level.is_night = false;

    getline(myfile, wave_cnt);
    getline(myfile, wave_dur);

    convert_wave_cnt_str_into_int(level, wave_cnt);
    convert_wave_dur_str_into_int_vect(level, wave_dur);
    for (int typ = NORMAL_TYPE; typ < COUNT_ZOMBIE_TYPE; typ++)
    {
        zombie_seq = "";
        getline(myfile, zombie_seq);
        convert_zombie_seq_str_into_int_vect(level, zombie_seq, typ);
    }
    level.cur_wave = 0;
    level.cur_sec = 0;
    level.waves_finished = false;
}

/*
Get information of number of waves in file.
*/
void convert_wave_cnt_str_into_int(Level &level, const string &wave_cnt)
{
    string temp;
    int num_ind = wave_cnt.find(":") + 2;
    temp = wave_cnt.substr(num_ind, wave_cnt.size() - num_ind);
    level.wave_count = stoi(temp);
}

/*
Get information of duration of wave in file.
*/
void convert_wave_dur_str_into_int_vect(Level &level, const string &wave_dur)
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
Get information of number of zombie in each wave in file.
*/
void convert_zombie_seq_str_into_int_vect(Level &level, const string &zombie_seq, const int &typ)
{
    string temp;
    int num_ind = zombie_seq.find(":") + 2;
    int space_ind = zombie_seq.find(" ", num_ind);
    for (int i = 0; i < level.wave_count; i++)
    {
        temp = zombie_seq.substr(num_ind, space_ind - num_ind);
        level.wave_zombie_count[typ].push_back(stoi(temp));
        num_ind = space_ind + 1;
        space_ind = zombie_seq.find(" ", num_ind);
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
void init_game(window &win, Level &level, Player &player, Map &cells)
{
    display_starting_screen(win);
    read_savedata(player, level);
    cells = create_a_collection_of_blocks();
}

/*
Random number of zombie appear in each second of the wave.
*/
void decide_zombie_cnt_for_each_sec(Level &level)
{
    bool enough_zombies = false;
    int z_cnt, sum;
    for (int typ = NORMAL_TYPE; typ < COUNT_ZOMBIE_TYPE; typ++)
    {
        for (int wave = 0; wave < level.wave_count; wave++)
        {
            vector<int> temp(level.wave_duration[wave], 0);
            enough_zombies = false;

            sum = 0;
            for (int sec = 0; sec < level.wave_duration[wave]; sec++)
            {
                z_cnt = rand(1, 5);

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
                sum += temp[sec];
            }
            level.zombie_distr_for_wave[typ].push_back(temp);
        }
    }
}

/*
Check if player has lost: Any zombie go to the house
*/
bool has_player_lost(Elements &elements)
{
    for (auto &zombie : elements.zombies)
    {
        if (zombie.x_location < X_UPPER_LEFT - 30)
            return true;
    }
    return false;
}

/*
Check if player won: no wave or zombie left.
*/
bool has_player_won(Level &level, Elements &elements)
{
    if (level.waves_finished && elements.zombies.empty() && elements.dead_zombies.empty())
        return true;
    return false;
}

/*
DISPLAY STARTING SCREEN AND CHOOSING LEVEL SCREEN
*/

/*Need update: Fake loading screen
Display starting screen
Updated: Now player can exit game from here.
*/
void display_starting_screen(window &win)
{
    bool game_started = false;
    bool quit = false;
    win.draw_png_scale(STARTING_SCREEN_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    play_music(OPENING_MUSIC_DIRECTORY);
    while (!quit && !game_started)
    {
        HANDLE(
            QUIT(quit = true; exit(0););
            // KEY_PRESS(q, quit = true);
            LCLICK({
                if (TAP_TO_START.is_mouse_in(mouse_x, mouse_y))
                    game_started = true;
            });

        );

        win.update_screen();
        DELAY(10);
    }
    win.fade_out();
    win.clear_renderer();
}

void load_level(Player &player, Level &level)
{
    read_level(level);
    decide_zombie_cnt_for_each_sec(level);
    player.sun_count = INIT_SUN_COUNT;
    player.is_choosing_a_plant = false;
    player.is_shoveling = false;
}

/*New function: Display choosing level
Display choosing level screen.
*/
void display_choosing_level_screen(window &win, Level &level, Player &player, bool &level_chosen, bool &quit)
{
    win.clear_renderer();
    win.draw_png_scale(CHOOSE_LEVELS_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    display_button(win, RENAME_BUTTON, RENAME_DIRECTORY);
    display_button(win, RESET_LEVEL_BUTTON, RESET_LEVEL_DIRECTORY);
    display_button(win, QUIT_BUTTON, QUIT_DIRECTORY);
    for (int i = 1; i <= LEVEL_COUNT; i++)
    {
        if (player.unlocked_level >= i)
        {
            win.show_text("Level " + to_string(i), LEVEL_BUTTON[i].x1 + 60, LEVEL_BUTTON[i].y1 + 130);
        }
        else
        {
            win.show_text("Level " + to_string(i), LEVEL_BUTTON[i].x1 + 60, LEVEL_BUTTON[i].y1 + 130, BLACK);

            display_level_is_locked(win, LEVEL_BUTTON[i]);
        }
    }
    HANDLE(
        QUIT(quit = true; exit(0););
        // KEY_PRESS(q, quit = true);
        LCLICK({
            if (RESET_LEVEL_BUTTON.is_mouse_in(mouse_x, mouse_y))
            {
                reset_unlocked_level(player);
            }
            if (RENAME_BUTTON.is_mouse_in(mouse_x, mouse_y))
            {
                display_rename_player(win, player, quit);
            }
            if (QUIT_BUTTON.is_mouse_in(mouse_x, mouse_y))
            {
                quit = true;
                exit(0);
            }
            for (int i = 1; i <= LEVEL_COUNT; i++)
                if (player.unlocked_level >= i && LEVEL_BUTTON[i].is_mouse_in(mouse_x, mouse_y))
                {
                    level.level_num = i;
                    load_level(player, level);
                    level.background_directory = BACKGROUND_LV1_DIRECTORY + level.map_type;
                    level_chosen = true;
                }
        });

    );
    win.update_screen();
}
/*New function (Need update):
Reset all things after a level finished.
*/
void reset_level(Elements &elements, Map &cells, Icons &icons)
{

    elements.peashooters.clear();
    elements.sunflowers.clear();
    elements.walnuts.clear();
    elements.snowpeas.clear();

    elements.suns.clear();
    elements.peas.clear();

    elements.zombies.clear();
    elements.dead_zombies.clear();

    for (int y = 0; y < VERT_BLOCK_COUNT; y++)
    {
        for (int x = 0; x < HORIZ_BLOCK_COUNT; x++)
        {
            cells[y][x].is_planted = false;
        }
    }
    icons = Icons();
}