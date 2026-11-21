#pragma once

/*HUD audit numbers shared with demo/hud_layout_check.cpp (the check asserts
the drawn positions instead of re-deriving them, so a drift here breaks the
check and forces a conscious layout review).*/
extern const int SUN_BAR_WIDTH;  // sun bar drawn width (sun_bar.png height-scaled)
extern const int SUN_BAR_X, SUN_BAR_Y;
extern const int BELT_ART_X1;    // conveyor backdrop left edge (left frame)
extern const int BELT_ART_X2;    // right edge: flush with TURBO_ICON.x1, never under it
extern const int BELT_ART_H;     // backdrop drawn height
extern const int BELT_ART_TILE_W, BELT_ART_TILE_H;   // scrolling tile source size
extern const int BELT_ART_TILE_CLIP_H;               // rows sampled/drawn (see render_elements.cpp)

void display_game_elements();
void display_game_layout();
void display_game_paused_elements();

/*The board's grave stones for the intro credit pans (seed chooser + credit
roll): panned `pan` px left with the camera, no Grave Buster scaling. Pair
with tick_grave_grows() from the pan host so night stones rise during the
pan (repvz: the initial graves sprout in the level-intro cutscene).*/
void display_credit_graves(int pan);

void display_seed_packets_bar();
void display_chosen_plant();

void blink_row_and_col();   // reads the live mouse position
void blink_row_and_col_at(int mouse_x, int mouse_y);