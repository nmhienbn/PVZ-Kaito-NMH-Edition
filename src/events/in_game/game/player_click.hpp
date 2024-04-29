#pragma once
#include "elements/plants/plant_type.hpp"
#include <vector>
using namespace std;

void handle_user_click(const int &mouse_x, const int &mouse_y);
void remove_plant_if_clicked_on(vector<Plants *> &plants, const int &mouse_x, const int &mouse_y);
bool pick_sun_if_clicked_on(const int &mouse_x, const int &mouse_y);

/*
These functions are used for choose plant seed, and plant it on playground.
*/

bool is_a_plant_seed_clicked_on(const int &mouse_x, const int &mouse_y);
void which_plant_is_chosen(int mouse_y);
void change_chosen_status(const int &i);
bool click_is_in_frontyard(const int &mouse_x, const int &mouse_y);
bool is_click_made_in_element_block(const int &row, const int &col, const int &mouse_x, const int &mouse_y);
void determine_row_and_col_chosen_by_second_click(const int &mouse_x, const int &mouse_y, int &row, int &col);
void remove_chosen_plant();
void create_new_plant(const int &mouse_x, const int &mouse_y);