#pragma once
#include "rsdl.hpp"
#include "gameInfo.h"
#include "elements.h"
#include "pea.h"
#include "peashooter.h"
#include "snowpea.h"
#include "sun.h"
#include "sunflower.h"
#include "zombie.h"
#include "cherrybomb.h"

void handle_changes(Icons &icons, Elements &elements, Map &cells, Level &level, int clk);
void handle_movements(Elements &elements, Map &cells, int clk);
void create_new_zombies(Elements &elements, Level &level);
void update_remaining_time(Icons &icons);
