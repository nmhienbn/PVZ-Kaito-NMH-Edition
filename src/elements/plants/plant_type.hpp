#pragma once
#include "./PotatoMine/PotatoMine.hpp"
#include "./cherrybomb/cherrybomb.hpp"
#include "./peashooter/peashooter.hpp"
#include "./snowpea/snowpea.hpp"
#include "./sunflower/sunflower.hpp"
#include "./wallnut/wallnut.hpp"
#include "plants.hpp"

Plants *init_plant(const int &type, const int &row, const int &col);