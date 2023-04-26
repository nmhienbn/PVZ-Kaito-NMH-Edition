#pragma once
#include "../volume/volume.hpp"
#include "draw/render_elements.hpp"
#include "music/music.hpp"

void display_game_pause();

void handle_menu_click(const int &mouse_x, const int &mouse_y);

void handle_pause_icon_click(const int &mouse_x, const int &mouse_y);

void unpause_game();