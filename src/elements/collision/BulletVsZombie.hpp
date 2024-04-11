#pragma once
#include "../bullets/bullet.hpp"
#include "../zombies/zombie_struct.hpp"

bool has_bullet_reached_zombie(const Bullet &bullet, const Zombie &zombie);
void handle_bullet_encounter_zombie(vector<Bullet *> &bullets, vector<Zombie *> &zombies,
                                    vector<ZombiePart> &zombie_parts);
void snow_pea_effect(Bullet &bullet, Zombie &zombie);
bool apply_bullet_hit_zombie(Bullet &bullet, Zombie &zombie, vector<ZombiePart> &zombie_parts);