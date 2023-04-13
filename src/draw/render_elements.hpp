#pragma once
#include "elements/button/button.hpp"
#include "elements/elements.hpp"
#include "elements/progress_bar/progress_bar.hpp"
#include "elements/mower/mower.hpp"

void display_choose_level(const bool &is_mouse_needed);
void display_game_elements();
void display_game_layout();
void display_game_paused_elements();

void display_icons_in_icon_bar();
void display_chosen_plant();

void blink_row_and_col();