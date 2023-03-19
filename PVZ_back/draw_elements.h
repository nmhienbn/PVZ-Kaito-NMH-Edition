#pragma once
#include "rsdl.hpp"
#include "gameInfo.h"
#include "elements.h"

void display_game_elements(window &win, Elements &elements, Map &map);
void display_game_layout(window &win, Player player, Icons icons);
void display_icons_in_icon_bar(Icons icons, Player player, window &win);
void display_winning_message(window &win);
void display_losing_message(window &win, Elements &elements, Map &map);
void display_suns(window &win, vector<Sun> suns, Map &map);
void display_zombies(window &win, vector<Zombie> &zombies, Map &map);
void display_peashooters(window &win, vector<Peashooter> &peashooters, Map &map);
void display_peas(window &win, vector<Pea> &peas, Map &map);
void display_sunflowers(window &win, vector<Sunflower> &sunflowers, Map &map);
void display_walnuts(window &win, vector<Walnut> &walnuts, Map &map);
void display_chosen_plant(window &win, Player player, Icons icons);
void display_dead_zombies(window &win, vector<DeadZombie> &dead_zombies, Map &map);
void display_ready_set_plant(window &win, int image_num);
void determine_walnut_appearance(Walnut &walnut);
void display_shovel(window &win);
void display_level_is_locked(window &win, const Button &button);