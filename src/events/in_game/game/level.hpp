#pragma once
#include "draw/render_elements.hpp"
#include "draw/render_message.hpp"
#include "elements/elements_actions.hpp"
#include "player_click.hpp"

#include "../pause_menu/game_pause.hpp"
#include "../restart_menu/restart_menu.hpp"
#include "../main_menu/leave_menu.hpp"
#include "../win/win.hpp"
#include "../lose/lose.hpp"
#include "../turbo/turbo.hpp"
#include "../credit/credit.hpp"

#include "player/player_data.hpp"
#include "music/music.hpp"

void start_level();
void display_all_in_game();