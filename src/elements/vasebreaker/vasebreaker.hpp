#pragma once

/*
Level 4-5 Vasebreaker ("scary_potter" preset): 15 ceramic vases stand on
cols 6-8 x rows 0..4 (repvz Challenge.cpp:3919-3933, ScaryPotterPopulate
adventure stage 0); clicking a vase breaks it and reveals its content --
an instant plant (the original hands you the seed packet on the cursor; the
clone auto-plants instead, see break_vase) or a zombie (spawns on the tile).
Deck: 5 peashooter + 5 squash + 4 normal + 1 buckethead. No waves, no sky
sun; the win check is the regular waves_finished + no-zombies gate, with
waves_finished set once every vase is broken.
*/
namespace vasebreaker {

// Called once per game tick (handle_changes): lazily populates the grid on
// the vasebreaker level and flips waves_finished when the board is clear.
void update();
// Click handling: break the vase under the cursor. Returns true when the
// click was consumed.
bool break_vase_if_clicked(const int &mouse_x, const int &mouse_y);
// Draw the unbroken vases of one lawn row (called from display_game_elements).
void display_row(const int &row);
// Debug/verification: vases currently standing.
int standing_vases();

} // namespace vasebreaker
