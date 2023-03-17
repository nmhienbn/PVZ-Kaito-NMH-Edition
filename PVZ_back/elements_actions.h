#pragma once
#include "rsdl.hpp"
#include "gameInfo.h"
#include "elements.h"

void handle_changes(Icons &icons, Elements &elements, Map &map, Level &level, int clk);
void handle_movements(Elements &elements, Map &map, int clk);

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Zombie actions
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
bool has_zombie_reached_element(Zombie zombie, int row, int col, Map &map);
bool has_zombie_reached_any_elements(Elements &elements, Zombie &zombie, Map &map);
void update_moving_status_for_zombies(Elements &elements, Map &map);
void create_new_zombies(Elements &elements, Level &level);
void move_zombies(vector<Zombie> &zombies, Elements &elements, Map &map);
bool can_zombie_move(Zombie &zombie, Elements &elements, Map &map);

//------------------------------Zombie bite--------------------------------------
void apply_zombie_bite_on_peashooter(Elements &elements, int z_ind, int p_ind, Map &map);
void handle_zombie_peashooter_encounter(Elements &elements, Map &map);

void apply_zombie_bite_on_sunflower(Elements &elements, int z_ind, int s_ind, Map &map);
void handle_zombie_sunflower_encounter(Elements &elements, Map &map);

void apply_zombie_bite_on_walnut(Elements &elements, int z_ind, int w_ind, Map &map);
void handle_zombie_walnut_encounter(Elements &elements, Map &map);

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Sun actions
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
void gen_random_sun_from_sky(Elements &elements);
void remove_suns(Elements &elements);
void move_suns(vector<Sun> &suns, Map &map);

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Bullets actions
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
void handle_pea_zombie_encounter(Elements &elements, Map &map);
bool apply_pea_hitting_zombie(Elements &elements, int p_ind, int z_ind);
bool has_pea_reached_zombie(Pea pea, Zombie zombie);
void move_peas(vector<Pea> &peas, Elements &elements, Map &map);
bool can_pea_move(Pea &pea, Elements &elements, Map &map);

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Peashooter actions
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
void fire_peas(Elements &elements, Map &map);
bool are_there_zombies_in_peashooter_row(Peashooter peashooter, Elements &elements);

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Sunflower actions
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
void gen_sun_from_all_sunflowers(Elements &elements, Map &map);
void gen_sun_from_a_sunflower(Elements &elements, Sunflower sunflower, Map &map);

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Wall-nut actions
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Icon actions
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
void update_remaining_time(Icons &icons);