#pragma once
#include "render_elements.h"
#include "music.h"

void display_game_pause();

void handle_menu_click(const int &mouse_x, const int &mouse_y);

void handle_menu_icon_click(const int &mouse_x, const int &mouse_y);

void unpause_game();