#pragma once
#include "button.h"
#include "peashooter.h"
#include "sunflower.h"
#include "walnut.h"
#include "snowpea.h"
#include "cherrybomb.h"
#include "zombie.h"
#include "pea.h"
#include "sun.h"

void render_choose_level_no_mouse(window &win, Player &player);
void display_game_elements(window &win, Elements &elements, Map &cells);
void display_game_layout(window &win, Player &player, Icons &icons, Level &level);
void display_game_paused_elements(window &win, Elements &elements, Map &cells, Level &level);

void display_icons_in_icon_bar(Icons &icons, Player &player, window &win, Level &level);
void display_chosen_plant(window &win, Player &player, Icons &icons);

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Display Game Buttons
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

void display_button(window &win, const Button &button, const int &button_directory);
void display_level_is_locked(window &win, const Button &button);
void blink_row_and_col(window &win, Map &cells, Level &level);