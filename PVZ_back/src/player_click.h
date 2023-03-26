#pragma once
#include "rsdl.hpp"
#include "gameInfo.h"
#include "elements.h"
#include "music.h"

void handle_user_click(Player &player, Level &level, Icons &icons, Elements &elements, Map &cells, const int &mouse_x, const int &mouse_y);
void remove_element_if_clicked_on(Map &cells, Elements &elements, const int &mouse_x, const int &mouse_y);
bool pick_sun_if_clicked_on(Elements &elements, Map &cells, const int &mouse_x, const int &mouse_y, Player &player);
bool is_shovel_clicked(const int &mouse_x, const int &mouse_y);

/*
These functions are used for choose plant seed, and plant it on playground.
*/

bool is_a_plant_seed_clicked_on(Level &level, const int &mouse_x, const int &mouse_y);
void which_plant_is_chosen(Player &player, Icons &icons, int mouse_y, bool &is_a_plant_chosen);
bool click_is_in_frontyard(Map &cells, Level &level, const int &mouse_x, const int &mouse_y);
bool is_click_made_in_element_block(int row, int col, const int &mouse_x, const int &mouse_y, Map &cells);
void determine_row_and_col_chosen_by_second_click(Map &cells, const int &mouse_x, const int &mouse_y, int &row, int &col);
void remove_chosen_plant(Player &player, Icons &icons);
void create_new_plant(Player &player, Map &cells, Elements &elements, Icons &icons, const int &mouse_x, const int &mouse_y);

template <class VectorPlant>
void remove_plant(Map &cells, VectorPlant &plants, int p_ind)
{
    cells[plants[p_ind].row][plants[p_ind].col].is_planted = false;
    plants.erase(plants.begin() + p_ind);
}