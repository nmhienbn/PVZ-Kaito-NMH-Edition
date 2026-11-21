#pragma once
#include "player/player_data.hpp"
#include "music/music.hpp"

void init_game();

void display_loading_screen();
void display_starting_screen();
// One starting-screen frame with the start label in the given state
// (display_starting_screen derives `tap_hovered` from the mouse).
void draw_starting_screen_frame(const bool &tap_hovered);