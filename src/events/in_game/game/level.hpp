#pragma once

void start_level();
// Ambient zombie groans, density-scaled (repvz Zombie::UpdatePlaying,
// Zombie.cpp:4581-4608). Called once per logic tick from the gameplay
// frame driver in start_level; also driven directly by the headless
// groan check.
void update_ambient_groans();