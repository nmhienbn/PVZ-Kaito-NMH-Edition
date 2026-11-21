#pragma once
#include "dave_script.hpp"
/*
Crazy Dave cutscenes (PvZ1 GOTY scripts, see dave_script.hpp).

Scene flow:
    + Hooked at the top of display_credit() (before the credits pan):
      play_intro() runs the blocking dialog over the frozen lawn.
    + Level 1-5 additionally starts the dig phase: Dave's 3 pre-placed
      peashooters must be shoveled before the bowling belt opens — driven
      per-tick by update_in_game() from start_level(), with the shovel advice
      banners on level.advice's manual channel.
    + First playthrough only: the scene key is level_num == unlocked_level
      (replaying an older level skips the intro entirely).
*/
namespace dave
{

// Blocking intro dialog; no-op unless this is the level's first playthrough.
// Call right after loading/choosing the level (display_credit does).
void play_intro();

// Per-tick hook from start_level(): runs the 1-5 dig phase (gates the first
// wave and the bowling belt until Dave's peashooters are dug up, then plays
// the "BOWLING!" dialog and hands the game over to the belt).
void update_in_game();

// level_num == player.unlocked_level: the level is being played for the
// first time.
bool is_first_play();

} // namespace dave
