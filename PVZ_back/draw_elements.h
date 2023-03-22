#pragma once
#include "rsdl.hpp"
#include "gameInfo.h"
#include "elements.h"

void display_game_elements(window &win, Elements &elements, Map &cells);
void display_game_layout(window &win, Player player, Icons icons, Level &level);
void display_icons_in_icon_bar(Icons icons, Player player, window &win, Level &level);
void display_winning_message(window &win, Elements &elements, Map &cells, Level &level);
void display_losing_message(window &win, Elements &elements, Map &cells, Level &level);
void display_suns(window &win, vector<Sun> suns, Map &cells);
void display_zombies(window &win, vector<Zombie> &zombies, Map &cells, bool is_pause = false);
void display_peashooters(window &win, vector<Peashooter> &peashooters, Map &cells, bool is_pause = false);
void display_peas(window &win, vector<Pea> &peas, Map &cells);
void display_sunflowers(window &win, vector<Sunflower> &sunflowers, Map &cells, bool is_pause = false);
void display_walnuts(window &win, vector<Walnut> &walnuts, Map &cells, bool is_pause = false);
void display_chosen_plant(window &win, Player player, Icons icons);
void display_dead_zombies(window &win, vector<DeadZombie> &dead_zombies, Map &cells, bool is_pause = false);
void display_ready_set_plant(window &win, int image_num, Level &level);
void determine_walnut_appearance(Walnut &walnut);
void display_shovel(window &win, const Button &button = Shovel);
void display_menu_icon(window &win, const Button &button = MENU_ICON);
void display_menu(window &win, const Button &button = MENU);
void display_level_is_locked(window &win, const Button &button);
void display_game_pause(window &win, Elements &elements, Map &cells, Level &level);