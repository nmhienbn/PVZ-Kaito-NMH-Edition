#pragma once
#include "elements.h"
#include "rsdl.hpp"

void determine_walnut_appearance(Walnut &walnut);
void display_walnuts(window &win, vector<Walnut> &walnuts, Map &cells, bool is_pause = false);