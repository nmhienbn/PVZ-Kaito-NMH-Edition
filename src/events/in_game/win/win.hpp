#pragma once
#include "draw/render_elements.hpp"
#include "music/music.hpp"
#include "player/player_data.hpp"

bool display_win();
bool has_player_won();
// One win-effect frame over the frozen lawn (f = effect clock); drawn by
// display_win and re-rendered alone by the headless win/award check.
void draw_win_frame(int f);
