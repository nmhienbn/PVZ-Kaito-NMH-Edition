#pragma once
#include "elements/button/button.hpp"

/*Player name + rename live on the selector/mode screen's wood sign
(selector_scene.cpp), like the retail GameSelector -- no plaque here.*/
const Button RESET_LEVEL_BUTTON(155, 155 + 252, 520, 520 + 72);
const Button QUIT_BUTTON(750, 750 + 252, 535, 535 + 72);
/*Back to the selector/mode screen (adventure / minigame / ... tombstones).*/
const Button BACK_TO_SELECTOR_BUTTON(15, 15 + 145, 15, 15 + 42);

void display_choose_level(const bool &is_mouse_needed);

// The Replanted backdrop pass alone (the level-select map drawn at the
// retail panel's canvas scale, bottom-anchored; see display_choose_level.cpp);
// exposed so headless checks can diff a bg-only frame against the full
// scene to prove the world signs are actually on screen.
void display_background();