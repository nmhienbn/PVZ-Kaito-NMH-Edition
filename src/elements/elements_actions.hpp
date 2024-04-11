#pragma once
#include "draw/rsdl.hpp"
#include "elements/collision/BulletVsZombie.hpp"
#include "elements/collision/MowerVsZombie.hpp"
#include "elements/collision/ZombieVsPlant.hpp"
#include "elements/elements.hpp"

void handle_changes();
void handle_movements();
void create_new_zombies();
void update_new_wave_zombies();
void update_plant_seeds_remaining_time();
