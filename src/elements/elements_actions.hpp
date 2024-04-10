#pragma once
#include "draw/rsdl.hpp"
#include "elements/collision/BulletVsZombie.hpp"
#include "elements/elements.hpp"
#include "elements/mower/mower.hpp"

void handle_changes();
void handle_movements();
void create_new_zombies();
void update_new_wave_zombies();
void update_plant_seeds_remaining_time();
