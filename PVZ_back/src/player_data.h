#pragma once
#include "button.h"
#include "render_elements.h"
#include "render_message.h"
#include "player_name.h"

void read_savedata(Player &player);
void update_unlocked_level(Player &player, Level &level);
void reset_unlocked_level(Player &player);
void update_player_name(Player &player, string new_name);