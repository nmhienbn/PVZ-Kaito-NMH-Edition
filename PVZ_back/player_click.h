#pragma once
#include "rsdl.hpp"
#include "gameInfo.h"
#include "elements.h"

void handle_user_click(Player &player, Icons &icons, Elements &elements, Map &map, int mouse_x, int mouse_y);
void remove_element_if_clicked_on(Map &map, Elements &elements, int mouse_x, int mouse_y);
bool pick_sun_if_clicked_on(Elements &elements, Map &map, int mouse_x, int mouse_y, Player &player);
bool is_a_plant_seed_clicked_on(int mouse_x, int mouse_y);
void which_plant_is_chosen(Player &player, Icons &icons, int mouse_y, bool &is_a_plant_chosen);
bool click_is_in_frontyard(Map &map, int mouse_x, int mouse_y);
void determine_row_and_col_chosen_by_second_click(Map &map, int mouse_x, int mouse_y, int &row, int &col);
void create_new_plant(Player &player, Map &map, Elements &elements, Icons &icons, int mouse_x, int mouse_y);
bool is_click_made_in_element_block(int row, int col, int mouse_x, int mouse_y, Map &map);
