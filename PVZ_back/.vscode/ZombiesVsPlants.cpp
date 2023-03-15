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

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

void handle_changes(Elements &elements, Map &map, Level &level, int clk)
{
	update_moving_status_for_zombies(elements, map);

	handle_pea_zombie_encounter(elements, map);
	if (level.waves_finished == false && clk % ZOMBIE_CREATE_CLK_COUNT == 0)
		create_new_zombies(elements, level);
	if (clk % BITE_CLK_COUNT == 0)
	{
		handle_zombie_walnut_encounter(elements, map);
		handle_zombie_peashooter_encounter(elements, map);
		handle_zombie_sunflower_encounter(elements, map);
	}
	if (clk % FIRE_PEA_CLK_COUNT == 0)
		fire_peas(elements, map);
	if (clk % SUN_GEN_SKY_CLK_COUNT == 0)
		gen_random_sun_from_sky(elements);
	if (clk % SUN_GEN_SUNFLOWER_CLK_COUNT == 0)
		for (int i = 0; i < elements.sunflowers.size(); i++)
			gen_random_sun_from_sunflower(elements, elements.sunflowers[i], map);

	remove_suns(elements);
}

void update_moving_status_for_zombies(Elements &elements, Map &map)
{
	for (int i = 0; i < elements.zombies.size(); i++)
	{
		if (has_zombie_reached_any_elements(elements, elements.zombies[i], map))
			elements.zombies[i].is_moving = false;
		elements.zombies[i].is_moving = true;
	}
}

void handle_pea_zombie_encounter(Elements &elements, Map &map)
{
	for (int i = 0; i < elements.zombies.size(); i++)
		for (int j = 0; j < elements.peas.size(); j++)
			apply_pea_hitting_zombie(elements, j, i);
}

void apply_pea_hitting_zombie(Elements &elements, int p_ind, int z_ind)
{
	if (has_pea_reached_zombie(elements.peas[p_ind], elements.zombies[z_ind]))
	{
		elements.zombies[z_ind].health--;
		elements.peas.erase(elements.peas.begin() + p_ind);
		determine_zombie_appearanc(elements.zombies[z_ind]);
		if (elements.zombies[z_ind].health == 0)
			elements.zombies.erase(elements.zombies.begin() + z_ind);
	}
}

void handle_zombie_walnut_encounter(Elements &elements, Map &map)
{
	for (int i = 0; i < elements.walnuts.size(); i++)
		for (int j = 0; j < elements.zombies.size(); j++)
			apply_zombie_bite_on_walnut(elements, j, i, map);
}

void handle_zombie_peashooter_encounter(Elements &elements, Map &map)
{
	for (int i = 0; i < elements.peashooters.size(); i++)
		for (int j = 0; j < elements.zombies.size(); j++)
			apply_zombie_bite_on_peashooter(elements, j, i, map);
}

void handle_zombie_sunflower_encounter(Elements &elements, Map &map)
{
	for (int i = 0; i < elements.sunflowers.size(); i++)
		for (int j = 0; j < elements.zombies.size(); j++)
			apply_zombie_bite_on_sunflower(elements, j, i, map);
}

void apply_zombie_bite_on_sunflower(Elements &elements, int z_ind, int s_ind, Map &map)
{
	if (has_zombie_reached_element(elements.zombies[z_ind], elements.sunflowers[s_ind].row, elements.sunflowers[s_ind].col, map))
	{
		elements.sunflowers[s_ind].bite++;
		if (elements.sunflowers[s_ind].bite == SUNFLOWER_BITE_LIMIT)
		{
			elements.zombies[z_ind].is_moving = true;
			elements.sunflowers.erase(elements.sunflowers.begin() + s_ind);
		}
	}
}

void apply_zombie_bite_on_peashooter(Elements &elements, int z_ind, int p_ind, Map &map)
{
	if (has_zombie_reached_element(elements.zombies[z_ind], elements.peashooters[p_ind].row, elements.peashooters[p_ind].col, map))
	{
		elements.peashooters[p_ind].bite++;
		if (elements.peashooters[p_ind].bite == PEASHOOTER_BITE_LIMIT)
		{
			elements.zombies[z_ind].is_moving = true;
			elements.peashooters.erase(elements.peashooters.begin() + p_ind);
		}
	}
}

void apply_zombie_bite_on_walnut(Elements &elements, int z_ind, int w_ind, Map &map)
{
	if (has_zombie_reached_element(elements.zombies[z_ind], elements.walnuts[w_ind].row, elements.walnuts[w_ind].col, map))
	{
		elements.walnuts[w_ind].bite++;
		determine_walnut_appearance(elements.walnuts[w_ind]);
		if (elements.walnuts[w_ind].bite == WALNUT_BITE_LIMIT)
		{
			elements.zombies[z_ind].is_moving = true;
			elements.walnuts.erase(elements.walnuts.begin() + w_ind);
		}
	}
}

void determine_walnut_appearance(Walnut &walnut)
{
	if (walnut.bite <= 4)
		walnut.directory = WALNUT_UNTOUCHED_DIRECTORY;
	else if (walnut.bite > 4 && walnut.bite <= 7)
		walnut.directory = WALNUT_CRACKED_1_DIRECTORY;
	else
		walnut.directory = WALNUT_CRACKED_2_DIRECTORY;
}

void determine_zombie_appearanc(Zombie &zombie)
{
	if (zombie.health > 4)
		zombie.directory = ZOMBIE_HEALTHY_DIRECTORY;
	else
		zombie.directory = ZOMBIE_INJURED_DIRECTORY;
}

void gen_random_sun_from_sky(Elements &elements)
{
	srand(time(NULL));
	Sun temp;
	temp.final_row = rand() % 5;
	temp.final_col = rand() % 9;
	temp.y_location = INIT_SUN_Y;
	temp.wait_seconds = 0;
	elements.suns.push_back(temp);
}

void gen_random_sun_from_sunflower(Elements &elements, Sunflower sunflower, Map &map)
{
	Sun temp;
	temp.final_col = sunflower.col;
	temp.final_row = sunflower.row;
	temp.y_location = map[sunflower.row][sunflower.col].y1 - 20;
	temp.wait_seconds = 0;
	elements.suns.push_back(temp);
}

void fire_peas(Elements &elements, Map &map)
{
	for (int i = 0; i < elements.peashooters.size(); i++)
	{
		Pea temp;
		int row = elements.peashooters[i].row;
		int col = elements.peashooters[i].col;
		temp.row = row;
		temp.x_location = map[row][col].x2 - 10;
		if (are_there_zombies_in_peashooter_row(elements.peashooters[i], elements))
			elements.peas.push_back(temp);
	}
}

void create_new_zombies(Elements &elements, Level &level)
{
	srand(time(NULL));
	Zombie temp;
	temp.x_location = ZOMBIE_INIT_X;
	temp.health = 10;
	temp.is_moving = true;
	temp.directory = ZOMBIE_HEALTHY_DIRECTORY;

	if (level.waves_finished == false)
	{
		int zombie_cnt = level.zombie_distr_for_wave[level.cur_wave][level.cur_sec];
		temp.row = rand() % 5;
		for (int i = 0; i < zombie_cnt; i++)
		{
			elements.zombies.push_back(temp);
			temp.row = (temp.row + 1 < 4) ? temp.row + 1 : 0;
		}
		if (level.cur_sec + 1 < level.wave_duration[level.cur_wave])
			level.cur_sec++;
		else if (level.cur_wave + 1 < level.wave_count)
		{
			level.cur_sec = 0;
			level.cur_wave++;
		}
		else
			level.waves_finished = true;
	}
}

bool are_there_zombies_in_peashooter_row(Peashooter peashooter, Elements &elements)
{
	for (int i = 0; i < elements.zombies.size(); i++)
		if (peashooter.row == elements.zombies[i].row)
			return true;
	return false;
}

void remove_suns(Elements &elements)
{
	for (int i = 0; i < elements.suns.size(); i++)
		if (elements.suns[i].wait_seconds >= 100)
			elements.suns.erase(elements.suns.begin() + i);
}

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

void handle_movements(Elements &elements, Map &map, int clk)
{
	if (clk % ZOMBIE_CLK_COUNT == 0)
		move_zombies(elements.zombies, elements, map);
	if (clk % SUN_CLK_COUNT == 0)
		move_suns(elements.suns, map);
	if (clk % PEA_CLK_COUNT == 0)
		move_peas(elements.peas, elements, map);
}

void move_suns(vector<Sun> &suns, Map &map)
{
	for (int i = 0; i < suns.size(); i++)
	{
		int row = suns[i].final_row;
		int col = suns[i].final_col;
		int lower_limit = map[row][col].y1 + 30;
		if (suns[i].y_location + SUN_DY < lower_limit)
			suns[i].y_location += SUN_DY;
		else
			suns[i].wait_seconds++;
	}
}

bool has_pea_reached_zombie(Pea pea, Zombie zombie)
{
	if (zombie.row == pea.row &&
		pea.x_location + PEA_DX > zombie.x_location + 30)
		return true;
	return false;
}

bool has_zombie_reached_element(Zombie zombie, int row, int col, Map &map)
{
	int right_limit = map[row][col].x2 - 35;
	int left_limit = map[row][col].x1;
	int zombie_new_location = zombie.x_location - ZOMBIE_DX;
	if (zombie.row == row &&
		zombie_new_location < right_limit &&
		zombie_new_location > left_limit)
		return true;
	return false;
}

bool can_pea_move(Pea &pea, Elements &elements, Map &map)
{
	int right_bound = map[pea.row][8].x2 - 20;
	if (pea.x_location + PEA_DX > right_bound)
		return false;
	for (int i = 0; i < elements.zombies.size(); i++)
		if (has_pea_reached_zombie(pea, elements.zombies[i]))
			return false;
	return true;
}

bool can_zombie_move(Zombie &zombie, Elements &elements, Map &map)
{
	int left_bound = map[0][0].x1;
	int zombie_new_location = zombie.x_location - ZOMBIE_DX;
	if (zombie_new_location < left_bound)
		return false;
	if (!zombie.is_moving)
		return false;
	if (has_zombie_reached_any_elements(elements, zombie, map))
		return false;
	return true;
}

bool has_zombie_reached_any_elements(Elements &elements, Zombie &zombie, Map &map)
{
	for (int i = 0; i < elements.walnuts.size(); i++)
		if (has_zombie_reached_element(zombie, elements.walnuts[i].row, elements.walnuts[i].col, map))
		{
			zombie.is_moving = false;
			return true;
		}
	for (int i = 0; i < elements.peashooters.size(); i++)
		if (has_zombie_reached_element(zombie, elements.peashooters[i].row, elements.peashooters[i].col, map))
		{
			zombie.is_moving = false;
			return true;
		}
	for (int i = 0; i < elements.sunflowers.size(); i++)
		if (has_zombie_reached_element(zombie, elements.sunflowers[i].row, elements.sunflowers[i].col, map))
		{
			zombie.is_moving = false;
			return true;
		}
	zombie.is_moving = true;
	return false;
}

void move_zombies(vector<Zombie> &zombies, Elements &elements, Map &map)
{
	for (int i = 0; i < zombies.size(); i++)
	{
		if (can_zombie_move(zombies[i], elements, map))
			zombies[i].x_location -= ZOMBIE_DX;
	}
}

void move_peas(vector<Pea> &peas, Elements &elements, Map &map)
{
	for (int i = 0; i < peas.size(); i++)
	{
		if (can_pea_move(peas[i], elements, map))
			peas[i].x_location += PEA_DX;
	}
}

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

void display_game_layout(window &win, Player player, Icons icons)
{
	win.draw_bg(BLACK_SCREEN_DIRECTORY);
	win.draw_bg(BACKGROUND_DIRECTORY);
	win.draw_png(ICON_BAR_DIRECTORY, 20, 100, ICON_BAR_WIDTH, ICON_BAR_HEIGHT);
	win.draw_png(SUN_DIRECTORY, 5, 5, SUN_WIDTH, SUN_HEIGHT);
	win.draw_png(SUN_COUNT_TEXT_FIELD_DIRECTORY, 85, 33, SUN_COUNT_WIDTH, SUN_COUNT_HEIGHT);
	win.show_text(to_string(player.sun_count), 100, 33);
	display_icons_in_icon_bar(icons, player, win);
}

void display_icons_in_icon_bar(Icons icons, Player player, window &win)
{
	if (player.sun_count < 50)
	{
		win.draw_png(SUNFLOWER_ICON_DIM_DIRECTORY, ICON_BAR_X1 + 3, SUNFLOWER_ICON_Y1, ICON_WIDTH, ICON_HEIGHT);
		win.draw_png(PEASHOOTER_100_ICON_DIM_DIRECTORY, ICON_BAR_X1 + 3, PEASHOOTER_100_ICON_Y1, ICON_WIDTH, ICON_HEIGHT);
		win.draw_png(WALNUT_ICON_DIM_DIRECTORY, ICON_BAR_X1 + 3, WALNUT_ICON_Y1, ICON_WIDTH, ICON_HEIGHT);
	}
	else if (player.sun_count >= 50)
	{
		win.draw_png(SUNFLOWER_ICON_BRIGHT_DIRECTORY, ICON_BAR_X1 + 3, SUNFLOWER_ICON_Y1, ICON_WIDTH, ICON_HEIGHT);
		win.draw_png(WALNUT_ICON_BRIGHT_DIRECTORY, ICON_BAR_X1 + 3, WALNUT_ICON_Y1, ICON_WIDTH, ICON_HEIGHT);
		if (player.sun_count >= 100)
			win.draw_png(PEASHOOTER_100_ICON_BRIGHT_DIRECTORY, ICON_BAR_X1 + 3, PEASHOOTER_100_ICON_Y1, ICON_WIDTH, ICON_HEIGHT);
		else
			win.draw_png(PEASHOOTER_100_ICON_DIM_DIRECTORY, ICON_BAR_X1 + 3, PEASHOOTER_100_ICON_Y1, ICON_WIDTH, ICON_HEIGHT);
	}
	return;
}

void display_game_elements(window &win, Elements &elements, Map &map)
{
	display_zombies(win, elements.zombies, map);
	display_peashooters(win, elements.peashooters, map);
	display_sunflowers(win, elements.sunflowers, map);
	display_walnuts(win, elements.walnuts, map);
	display_peas(win, elements.peas, map);
	display_suns(win, elements.suns, map);
}

void display_losing_message(window &win)
{
	win.draw_bg(BLACK_SCREEN_DIRECTORY);
	win.draw_bg(BACKGROUND_DIM_DIRECTORY);
	win.draw_png(LOSING_MESSAGE_DIRECTORY, WINDOW_WIDTH / 4, WINDOW_HEIGHT / 8 - 40, 500, 500);
}

void display_winning_message(window &win)
{
	win.draw_bg(BLACK_SCREEN_DIRECTORY);
	win.draw_bg(BACKGROUND_DIM_DIRECTORY);
	win.draw_png(WINNING_MESSAGE_DIRECTORY, WINDOW_WIDTH / 3, WINDOW_HEIGHT / 8 - 60, 300, 550);
}

void display_suns(window &win, vector<Sun> suns, Map &map)
{
	for (int i = 0; i < suns.size(); i++)
	{
		int col = suns[i].final_col;
		int x_location = map[0][col].x1;
		win.draw_png(SUN_DIRECTORY, x_location, suns[i].y_location, ELEMENT_WIDTH, ELEMENT_HEIGHT);
	}
}

void display_zombies(window &win, vector<Zombie> &zombies, Map &map)
{
	for (int i = 0; i < zombies.size(); i++)
	{
		int row = zombies[i].row;
		int y_location = map[row][0].y1;
		win.draw_png(zombies[i].directory, zombies[i].x_location, y_location, ELEMENT_WIDTH, ELEMENT_HEIGHT);
	}
}

void display_peashooters(window &win, vector<Peashooter> &peashooters, Map &map)
{
	for (int i = 0; i < peashooters.size(); i++)
	{
		int col = peashooters[i].col;
		int row = peashooters[i].row;
		win.draw_png(PEASHOOTER_DIRECTORY, map[row][col].x1 + 9, map[row][col].y1 + 9, ELEMENT_WIDTH, ELEMENT_HEIGHT);
	}
}

void display_peas(window &win, vector<Pea> &peas, Map &map)
{
	for (int i = 0; i < peas.size(); i++)
	{
		int row = peas[i].row;
		int y_location = map[row][0].y1 + 20;
		if (peas[i].x_location < (map[0][8].x2 - 35))
			win.draw_png(PEA_DIRECTORY, peas[i].x_location, y_location, PEA_WIDTH, PEA_HEIGHT);
	}
}

void display_sunflowers(window &win, vector<Sunflower> &sunflowers, Map &map)
{
	for (int i = 0; i < sunflowers.size(); i++)
	{
		int col = sunflowers[i].col;
		int row = sunflowers[i].row;
		win.draw_png(SUNFLOWER_DIRECTORY, map[row][col].x1 + 9, map[row][col].y1 + 9, ELEMENT_WIDTH, ELEMENT_HEIGHT);
	}
}

void display_walnuts(window &win, vector<Walnut> &walnuts, Map &map)
{
	for (int i = 0; i < walnuts.size(); i++)
	{
		int col = walnuts[i].col;
		int row = walnuts[i].row;
		win.draw_png(walnuts[i].directory, map[row][col].x1 + 9, map[row][col].y1 + 9, ELEMENT_WIDTH, ELEMENT_HEIGHT);
	}
}

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

void handle_user_click(Player &player, Icons &icons, Elements &elements, Map &map, int mouse_x, int mouse_y)
{
	bool sun_picked = false;
	if (player.is_first_click_made == false)
	{
		if (is_an_icon_chosen(mouse_x, mouse_y))
		{
			determine_icon_chosen(icons, mouse_y);
			player.is_first_click_made = true;
		}
		pick_sun_if_clicked_on(elements, map, mouse_x, mouse_y, sun_picked, player);
		if (!sun_picked && click_is_in_frontyard(map, mouse_x, mouse_y))
			remove_element_if_clicked_on(map, elements, mouse_x, mouse_y);
	}
	else if (player.is_first_click_made == true)
		if (click_is_in_frontyard(map, mouse_x, mouse_y))
		{
			create_new_plant(player, map, elements, icons, mouse_x, mouse_y);
			player.is_first_click_made = false;
		}
}

bool is_an_icon_chosen(int mouse_x, int mouse_y)
{
	if (mouse_x > ICON_BAR_X1 && mouse_x < ICON_BAR_X2 &&
		mouse_y > ICON_BAR_Y1 && mouse_y < ICON_BAR_Y2)
		return true;
	return false;
}

void determine_icon_chosen(Icons &icons, int mouse_y)
{
	icons.is_peashooter_chosen = false;
	icons.is_walnut_chosen = false;
	icons.is_sunflower_chosen = false;
	if (mouse_y > SUNFLOWER_ICON_Y1 && mouse_y < SUNFLOWER_ICON_Y1 + ICON_HEIGHT)
		icons.is_sunflower_chosen = true;
	else if (mouse_y > PEASHOOTER_100_ICON_Y1 &&
			 mouse_y < PEASHOOTER_100_ICON_Y1 + ICON_HEIGHT)
		icons.is_peashooter_chosen = true;
	else
		icons.is_walnut_chosen = true;
}

bool click_is_in_frontyard(Map &map, int mouse_x, int mouse_y)
{
	int right_bound = map[0][8].x2;
	int left_bound = map[0][0].x1;
	int upper_bound = map[0][0].y1;
	int lower_bound = map[4][0].y2;
	if (mouse_x > left_bound && mouse_x < right_bound &&
		mouse_y > upper_bound && mouse_y < lower_bound)
		return true;
	return false;
}

void determine_row_and_col_chosen_by_second_click(Map &map, int mouse_x, int mouse_y, int &row, int &col)
{
	for (int y = 0; y < VERT_BLOCK_COUNT; y++)
		for (int x = 0; x < HORIZ_BLOCK_COUNT; x++)
			if (mouse_x > map[y][x].x1 && mouse_x < map[y][x].x2 &&
				mouse_y > map[y][x].y1 && mouse_y < map[y][x].y2)
			{
				row = y;
				col = x;
				return;
			}
}

void create_new_plant(Player &player, Map &map, Elements &elements, Icons &icons, int mouse_x, int mouse_y)
{
	int row, col;
	determine_row_and_col_chosen_by_second_click(map, mouse_x, mouse_y, row, col);
	if (icons.is_sunflower_chosen && player.sun_count >= 50)
	{
		Sunflower temp;
		temp.row = row;
		temp.col = col;
		temp.bite = 0;
		elements.sunflowers.push_back(temp);
		player.sun_count -= SUNFLOWER_PRICE;
	}
	else if (icons.is_peashooter_chosen && player.sun_count >= 100)
	{
		Peashooter temp;
		temp.row = row;
		temp.col = col;
		temp.bite = 0;
		elements.peashooters.push_back(temp);
		player.sun_count -= PEASHOOTER_100_PRICE;
	}
	else if (icons.is_walnut_chosen && player.sun_count >= 50)
	{
		Walnut temp;
		temp.row = row;
		temp.col = col;
		temp.bite = 0;
		temp.directory = WALNUT_UNTOUCHED_DIRECTORY;
		elements.walnuts.push_back(temp);
		player.sun_count -= WALNUT_PRICE;
	}
}

void remove_element_if_clicked_on(Map &map, Elements &elements, int mouse_x, int mouse_y)
{
	for (int i = 0; i < elements.sunflowers.size(); i++)
	{
		if (is_click_made_in_element_block(elements.sunflowers[i].row, elements.sunflowers[i].col, mouse_x, mouse_y, map))
			elements.sunflowers.erase(elements.sunflowers.begin() + i);
	}
	for (int i = 0; i < elements.peashooters.size(); i++)
	{
		if (is_click_made_in_element_block(elements.peashooters[i].row, elements.peashooters[i].col, mouse_x, mouse_y, map))
			elements.peashooters.erase(elements.peashooters.begin() + i);
	}
	for (int i = 0; i < elements.walnuts.size(); i++)
	{
		if (is_click_made_in_element_block(elements.walnuts[i].row, elements.walnuts[i].col, mouse_x, mouse_y, map))
			elements.walnuts.erase(elements.walnuts.begin() + i);
	}
}

bool is_click_made_in_element_block(int row, int col, int mouse_x, int mouse_y, Map &map)
{
	if (mouse_x > map[row][col].x1 && mouse_x < map[row][col].x2 &&
		mouse_y > map[row][col].y1 && mouse_y < map[row][col].y2)
		return true;
	return false;
}

void pick_sun_if_clicked_on(Elements &elements, Map &map, int mouse_x, int mouse_y, bool &sun_picked, Player &player)
{
	for (int i = 0; i < elements.suns.size(); i++)
	{
		int row = elements.suns[i].final_row;
		int col = elements.suns[i].final_col;
		int right_bound = map[row][col].x2;
		int left_bound = map[row][col].x1;
		int upper_bound = elements.suns[i].y_location;
		int lower_bound = elements.suns[i].y_location + ELEMENT_HEIGHT;
		if (mouse_x > left_bound && mouse_x < right_bound &&
			mouse_y > upper_bound && mouse_y < lower_bound)
		{
			elements.suns.erase(elements.suns.begin() + i);
			sun_picked = true;
			player.sun_count += 50;
			return;
		}
		sun_picked = false;
	}
}

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

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

void read_savedata(Player &player, Level &level)
{
	string line;
	ifstream myfile("savedata.txt");
	if (myfile.is_open())
	{
		getline(myfile, line);
		player.name = line;
		getline(myfile, line);
		level.level_num = stoi(line);
		myfile.close();
	}
	else
		cout << "Unable to open file";
}

void read_level(Level &level)
{
	string wave_cnt, zombie_seq, wave_dur, temp;
	string file_name = "";
	file_name.push_back(level.level_num + '0');
	file_name += ".level.txt";
	ifstream myfile(file_name);
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

void save_wave_cnt_str_as_int(Level &level, string wave_cnt)
{
	string temp;
	int num_ind = wave_cnt.find(":") + 2;
	temp = wave_cnt.substr(num_ind, wave_cnt.size() - num_ind);
	level.wave_count = stoi(temp);
}

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

void init_game(window &win, Level &level, Player &player, Map &map)
{
	desplay_starting_screen(win);
	read_savedata(player, level);
	read_level(level);
	decide_zombie_cnt_for_each_sec(level);
	map = create_a_collection_of_blocks();
	player.sun_count = INIT_SUN_COUNT;
	player.is_first_click_made = false;
}

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

void desplay_starting_screen(window &win)
{
	bool game_started = false;
	bool quit = false;
	while (quit || !game_started)
	{
		win.draw_png(STARTING_SCREEN_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
		HANDLE(
			QUIT(quit = true);
			KEY_PRESS(q, quit = true);
			LCLICK({
				if (user_clicked_on_start(mouse_x, mouse_y))
					game_started = true;
			}););

		win.update_screen();
		DELAY(10);
	}
}

bool user_clicked_on_start(int mouse_x, int mouse_y)
{
	if (mouse_x > TAP_TO_START_X1 && mouse_y < TAP_TO_START_X2 &&
		mouse_y > TAP_TO_START_Y1 && mouse_y < TAP_TO_START_Y2)
		return true;
	return false;
}

bool has_player_lost(Elements &elements)
{
	for (int i = 0; i < elements.zombies.size(); i++)
	{
		if (elements.zombies[i].x_location < X_UPPER_LEFT + 5)
			return true;
	}
	return false;
}

bool has_player_won(Level &level, Elements &elements)
{
	if (level.waves_finished && elements.zombies.size() == 0)
		return true;
	return false;
}

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

int main(int argv, char **args)
{
	bool quit = false;
	long int clk = 0;
	Level level;
	Elements game_characters;
	Icons icons;
	Player player;
	Map map;

	window win(WINDOW_WIDTH, WINDOW_HEIGHT);
	init_game(win, level, player, map);

	while (!quit)
	{

		if (has_player_lost(game_characters))
			display_losing_message(win);
		else if (has_player_won(level, game_characters))
			display_winning_message(win);
		else
		{
			display_game_layout(win, player, icons);
			display_game_elements(win, game_characters, map);
			handle_movements(game_characters, map, clk);
			handle_changes(game_characters, map, level, clk);
		}

		HANDLE(
			QUIT(quit = true);
			KEY_PRESS(q, quit = true);
			LCLICK({
				handle_user_click(player, icons, game_characters, map, mouse_x, mouse_y);
			}););

		clk++;
		win.update_screen();
		DELAY(CLK_LENGTH);
	}
	return 0;
}
