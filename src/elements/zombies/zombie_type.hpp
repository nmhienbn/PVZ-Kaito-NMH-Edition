#pragma once
#include "bucket/bucket_zombie.hpp"
#include "cone/cone_zombie.hpp"
#include "door/door_zombie.hpp"
#include "flag/flag_zombie.hpp"
#include "normal/normal_zombie.hpp"
#include "zombie_struct.hpp"

Zombie *init_zombie(int _type, int level_num);
Zombie *init_zombie(int _type, int level_num, const int &x_bias);