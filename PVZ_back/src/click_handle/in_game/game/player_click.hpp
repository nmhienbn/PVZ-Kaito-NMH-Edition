#pragma once
#include "draw/rsdl.hpp"
#include <elements\button\button.hpp>
#include "elements/elements.hpp"
#include "music/music.hpp"

void handle_user_click(const int &mouse_x, const int &mouse_y);
void remove_element_if_clicked_on(const int &mouse_x, const int &mouse_y);
bool pick_sun_if_clicked_on(const int &mouse_x, const int &mouse_y);
bool is_shovel_clicked(const int &mouse_x, const int &mouse_y);

/*
These functions are used for choose plant seed, and plant it on playground.
*/

bool is_a_plant_seed_clicked_on(const int &mouse_x, const int &mouse_y);
void which_plant_is_chosen(int mouse_y, bool &is_a_plant_chosen);
bool click_is_in_frontyard(const int &mouse_x, const int &mouse_y);
bool is_click_made_in_element_block(const int &row, const int &col, const int &mouse_x, const int &mouse_y);
void determine_row_and_col_chosen_by_second_click(const int &mouse_x, const int &mouse_y, int &row, int &col);
void remove_chosen_plant();
void create_new_plant(const int &mouse_x, const int &mouse_y);

extern Map cells;
template <class VectorPlant>
void remove_plant(VectorPlant &plants, int p_ind)
{
    cells[plants[p_ind].row][plants[p_ind].col].is_planted = false;
    plants.erase(plants.begin() + p_ind);
}