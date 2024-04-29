#pragma once
#include "elements/button/button.hpp"

const Button RENAME_BUTTON(10, 237, 265, 405);
const Button RESET_LEVEL_BUTTON(155, 155 + 252, 520, 520 + 72);
const Button QUIT_BUTTON(750, 750 + 252, 535, 535 + 72);

void display_choose_level(const bool &is_mouse_needed);