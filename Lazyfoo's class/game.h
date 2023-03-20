// In the name of God
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include "rsdl.hpp"

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Speed properties: increase or decrease the following 9 define values to speed up or speed down entities in the game
// CLK meaning : clock
#define CLK_LENGTH 5
#define SUN_GEN_SKY_CLK_COUNT 2000
#define SUN_GEN_SUNFLOWER_CLK_COUNT 4000
#define ZOMBIE_CREATE_CLK_COUNT 1200
#define FIRE_PEA_CLK_COUNT 300
#define BITE_CLK_COUNT 100
#define ZOMBIE_CLK_COUNT 22
#define SUN_CLK_COUNT 8
#define PEA_CLK_COUNT 4
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Alter the 4 define values bellow to increase or decrease the price of plants and the initial sun count
#define INIT_SUN_COUNT 200
#define PEASHOOTER_100_PRICE 100
#define SUNFLOWER_PRICE 50
#define WALNUT_PRICE 50
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

#define ZOMBIE_INIT_X 900
#define ZOMBIE_DX 1
#define PEA_DX 15
#define SUN_DY 1
#define WALNUT_BITE_LIMIT 72
#define SUNFLOWER_BITE_LIMIT 5
#define PEASHOOTER_BITE_LIMIT 5

#define WINDOW_WIDTH 1050
#define WINDOW_HEIGHT 600
#define BLOCK_WIDTH 81
#define BLOCK_HEIGHT 101
#define HORIZ_BLOCK_COUNT 9
#define VERT_BLOCK_COUNT 5

#define X_UPPER_LEFT 250
#define Y_UPPER_LEFT 70
#define TAP_TO_START_X1 230
#define TAP_TO_START_X2 796
#define TAP_TO_START_Y1 520
#define TAP_TO_START_Y2 585
#define ICON_BAR_X1 20
#define ICON_BAR_X2 113
#define ICON_BAR_Y1 102
#define ICON_BAR_Y2 292

#define INIT_SUN_Y 5
#define SUNFLOWER_ICON_Y1 103
#define PEASHOOTER_100_ICON_Y1 167
#define WALNUT_ICON_Y1 231

#define ICON_BAR_WIDTH 95
#define ICON_BAR_HEIGHT 194
#define ICON_WIDTH 89
#define ICON_HEIGHT 60
#define SUN_COUNT_WIDTH 70
#define SUN_COUNT_HEIGHT 30
#define SUN_WIDTH 80
#define SUN_HEIGHT 80
#define PEA_WIDTH 20
#define PEA_HEIGHT 20
#define ELEMENT_WIDTH 65
#define ELEMENT_HEIGHT 80

#define STARTING_SCREEN_DIRECTORY "./Image_Assets/Starting_Screen.png"
#define BACKGROUND_DIRECTORY "./Image_Assets/Frontyard.png"
#define BACKGROUND_DIM_DIRECTORY "./Image_Assets/Frontyard_dim.jpeg"
#define LOSING_MESSAGE_DIRECTORY "./Image_Assets/Losing_Message.png"
#define WINNING_MESSAGE_DIRECTORY "./Image_Assets/Winning_Pic.png"
#define BLACK_SCREEN_DIRECTORY "./Image_Assets/Black_Screen.png"
#define ZOMBIE_HEALTHY_DIRECTORY "./Image_Assets/Zombie_healthy.png"
#define ZOMBIE_INJURED_DIRECTORY "./Image_Assets/Zombie_Injured.png"
#define SUN_DIRECTORY "./Image_Assets/sun.png"
#define SUNFLOWER_DIRECTORY "./Image_Assets/Sunflower.png"
#define PEASHOOTER_DIRECTORY "./Image_Assets/Peashooter.png"
#define PEA_DIRECTORY "./Image_Assets/pea.png"
#define WALNUT_UNTOUCHED_DIRECTORY "./Image_Assets/Wallnut_body.png"
#define WALNUT_CRACKED_1_DIRECTORY "./Image_Assets/Wallnut_cracked1.png"
#define WALNUT_CRACKED_2_DIRECTORY "./Image_Assets/Wallnut_cracked2.png"
#define ICON_BAR_DIRECTORY "./Image_Assets/Item_Bar.png"
#define SUN_COUNT_TEXT_FIELD_DIRECTORY "./Image_Assets/sun_count_text_field.png"
#define PEASHOOTER_100_ICON_BRIGHT_DIRECTORY "./Image_Assets/icon_peashooter_100.png"
#define PEASHOOTER_100_ICON_DIM_DIRECTORY "./Image_Assets/icon_peashooter_100_dim.png"
#define SUNFLOWER_ICON_BRIGHT_DIRECTORY "./Image_Assets/icon_sunflower.png"
#define SUNFLOWER_ICON_DIM_DIRECTORY "./Image_Assets/icon_sunflower_dim.png"
#define WALNUT_ICON_BRIGHT_DIRECTORY "./Image_Assets/icon_walnut.png"
#define WALNUT_ICON_DIM_DIRECTORY "./Image_Assets/icon_walnut_dim.png"

using namespace std;

struct Block
{
    int x1, x2;
    int y1, y2;
};

struct Sun
{
    int final_col;
    int final_row;
    int y_location;
    int wait_seconds;
};

struct Sunflower
{
    int row, col;
    int bite;
};

struct Walnut
{
    int row, col;
    int bite;
    string directory;
};

struct Peashooter
{
    int row, col;
    int bite;
};

struct Pea
{
    int row;
    int x_location;
};

struct Zombie
{
    int row;
    int x_location;
    int health;
    bool is_moving;
    string directory;
};

struct Elements
{
    vector<Sun> suns;
    vector<Zombie> zombies;
    vector<Peashooter> peashooters;
    vector<Pea> peas;
    vector<Sunflower> sunflowers;
    vector<Walnut> walnuts;
};

struct Player
{
    string name;
    int sun_count;
    int level;
    bool is_first_click_made;
};

struct Icons
{
    bool is_sunflower_chosen;
    bool is_peashooter_chosen;
    bool is_walnut_chosen;
    string sunflower_icon_directory;
    string peashooter_icon_directory;
    string walnut_icon_directory;
};

struct Level
{
    int level_num;
    int wave_count;
    int cur_wave;
    int cur_sec;
    bool waves_finished;

    vector<vector<int>> zombie_distr_for_wave;
    vector<int> wave_zombie_count;
    vector<int> wave_duration;
};

typedef vector<vector<Block>> Map;

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

/*Check if pea reached a zombie.*/
bool has_pea_reached_zombie(Pea pea, Zombie zombie);
/*Check if zombie reached a tile.*/
bool has_zombie_reached_element(Zombie zombie, int row, int col, Map &map);
bool has_zombie_reached_any_elements(Elements &elements, Zombie &zombie, Map &map);

void handle_changes(Elements &elements, Map &map, Level &level, int clk);
void update_moving_status_for_zombies(Elements &elements, Map &map);
void determine_walnut_appearance(Walnut &walnut);
void determine_zombie_appearanc(Zombie &zombie);
void handle_pea_zombie_encounter(Elements &elements, Map &map);
void apply_pea_hitting_zombie(Elements &elements, int p_ind, int z_ind);
void handle_zombie_peashooter_encounter(Elements &elements, Map &map);
void apply_zombie_bite_on_peashooter(Elements &elements, int z_ind, int p_ind, Map &map);
void handle_zombie_sunflower_encounter(Elements &elements, Map &map);
void apply_zombie_bite_on_sunflower(Elements &elements, int z_ind, int s_ind, Map &map);
void handle_zombie_walnut_encounter(Elements &elements, Map &map);
void apply_zombie_bite_on_walnut(Elements &elements, int z_ind, int w_ind, Map &map);
void gen_random_sun_from_sky(Elements &elements);
void gen_random_sun_from_sunflower(Elements &elements, Sunflower sunflower, Map &map);
void fire_peas(Elements &elements, Map &map);
void create_new_zombies(Elements &elements, Level &level);
bool are_there_zombies_in_peashooter_row(Peashooter peashooter, Elements &elements);
void remove_suns(Elements &elements);

void handle_movements(Elements &elements, Map &map, int clk);
void move_zombies(vector<Zombie> &zombies, Elements &elements, Map &map);
void move_suns(vector<Sun> &suns, Map &map);
void move_peas(vector<Pea> &peas, Elements &elements, Map &map);
bool can_pea_move(Pea &pea, Elements &elements, Map &map);
bool can_zombie_move(Zombie &zombie, Elements &elements, Map &map);

void display_game_elements(window &win, Elements &elements, Map &map);
void display_game_layout(window &win, Player player, Icons icons);
void display_icons_in_icon_bar(Icons icons, Player player, window &win);
void display_losing_message(window &win);
void display_suns(window &win, vector<Sun> suns, Map &map);
void display_zombies(window &win, vector<Zombie> &zombies, Map &map);
void display_peashooters(window &win, vector<Peashooter> &peashooters, Map &map);
void display_peas(window &win, vector<Pea> &peas, Map &map);
void display_sunflowers(window &win, vector<Sunflower> &sunflowers, Map &map);
void display_walnuts(window &win, vector<Walnut> &walnuts, Map &map);

void handle_user_click(Player &player, Icons &icons, Elements &elements, Map &map, int mouse_x, int mouse_y);
void remove_element_if_clicked_on(Map &map, Elements &elements, int mouse_x, int mouse_y);
void pick_sun_if_clicked_on(Elements &elements, Map &map, int mouse_x, int mouse_y, bool &sun_picked, Player &player);
bool is_an_icon_chosen(int mouse_x, int mouse_y);
void determine_icon_chosen(Icons &icons, int mouse_y);
bool click_is_in_frontyard(Map &map, int mouse_x, int mouse_y);
void determine_row_and_col_chosen_by_second_click(Map &map, int mouse_x, int mouse_y, int &row, int &col);
void create_new_plant(Player &player, Map &map, Elements &elements, Icons &icons, int mouse_x, int mouse_y);
bool is_sun_chosen(Elements &elements, int mouse_x, int mouse_y);
bool is_click_made_in_element_block(int row, int col, int mouse_x, int mouse_y, Map &map);

Map create_a_collection_of_blocks();
void read_savedata(Player &player, Level &level);
void read_level(Level &level);
void save_wave_cnt_str_as_int(Level &level, string wave_cnt);
void save_zombie_seq_str_as_int_vect(Level &level, string zombie_seq);
void save_wave_dur_str_as_int_vect(Level &level, string wave_dur);
void init_game(window &win, Level &level, Player &player, Map &map);
void decide_zombie_cnt_for_each_sec(Level &level);
void desplay_starting_screen(window &win);
bool user_clicked_on_start(int mouse_x, int mouse_y);
bool has_player_lost(Elements &elements);
bool has_player_won(Level &level, Elements &elements);