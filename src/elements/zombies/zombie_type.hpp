#pragma once
#include "zombie_struct.hpp"
#include "normal/normal_zombie.hpp"
#include "flag/flag_zombie.hpp"
#include "cone/cone_zombie.hpp"
#include "bucket/bucket_zombie.hpp"
#include "door/door_zombie.hpp"

Zombie *init(int _type, int level_num);