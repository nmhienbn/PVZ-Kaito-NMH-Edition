#pragma once
#include "zombie_type.hpp"

/*repvz Plant::GetPlantRect horizontal span of tile (row,col) (Plant.cpp:5221-5240):
the union right/left edges over the plants sitting on the tile. Default plants
occupy the tile inset 10 px both sides; Tall-nut keeps its full 80 px after
the +10 shift, the 120-px-wide pumpkin shell reaches 20 px past the tile's
right edge, Cob Cannon spans 140 px. Used by has_zombie_reached_block and the
zomboni drive-over probe.*/
void plant_rect_span(const int &row, const int &col, int &r_left, int &r_right);
bool has_zombie_reached_block(const Zombie &zombie, const int &row, const int &col);
bool has_zombie_reached_plant(Zombie &zombie);
bool has_zombie_reached_any_plant(Zombie &zombie);
bool can_zombie_move(Zombie &zombie);
void update_moving_status_for_zombies(vector<Zombie *> &zombies);
void move_zombies(vector<Zombie *> &zombies);
void display_zombies(vector<Zombie *> &zombies, const int &_row);
void display_zombie_parts(vector<ZombiePart> &zombie_parts, const int &_row);
