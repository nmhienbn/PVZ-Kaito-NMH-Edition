#pragma once
#include "draw/render_elements.hpp"
#include "music/music.hpp"

void display_unlock_plant(const int &_plant_type, int f);
void unlock_plant(const int &_plant_type);
void unlock_shovel();
void unlock_zombie_note();
void unlock_zombie_note2();
void unlock_almanac();
void unlock_carkeys();
void unlock_taco();
void unlock_wateringcan();

// Dev/headless-check compose: a full settled award frame (backdrop, texts,
// Continue skin) drawn WITHOUT update_screen so it can be read back to PNG.
// Matches what the pumps present at frame f (0=plant intro start).
// award ids: 100=plant (_plant_type), 101=zombie note1, 102=note2,
// 103=shovel, 104=almanac, 105=car keys, 106=taco, 107=watering can.
void debug_award_frame(int award_id, const int &_plant_type, int f);