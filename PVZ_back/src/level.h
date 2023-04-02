#pragma once
#include "render_elements.h"
#include "render_message.h"
#include "elements_actions.h"
#include "player_click.h"

#include "game_pause.h"
#include "restart_menu.h"
#include "leave_menu.h"

#include "player_data.h"
#include "music.h"

void start_level();

bool display_win();
void display_lose();
void display_R_S_P();

void display_all_in_game();

void unlock_plant(const int &_plant_type);