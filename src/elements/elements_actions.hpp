#pragma once

void handle_changes();
void handle_movements();
void create_new_zombies();
void update_new_wave_zombies();
void update_plant_seeds_remaining_time();
// Per-tick lifecycles split out of handle_changes (also used by the headless
// behavior checks): ice-trail melt countdown + doom-shroom crater healing.
void update_ice_trails();
void update_doom_craters();
// Sprouting-stone clocks only (split from update_graves so the intro pans --
// seed chooser and credit roll -- can grow the initial stones before gameplay
// starts, like the original's level-intro cutscene).
void tick_grave_grows();
// Night gravestone tick: sprouting stones finish growing in; pending
// grave-risers break the surface and start walking.
void update_graves();
// Grave Buster chew progress on a stone tile: 0 = stone full size, ->1 gone
// (landing ticks report 0; the chew maps linearly onto 0..1). The grave
// renderer scales the stone by (1 - fraction) while a buster is on it.
float grave_eaten_fraction(int row, int col);
// Wave materialization: pushes current_wave's zombies onto the lawn
// (bobsled teams without live ice in their lane are dropped here).
void add_zombies_from_level_data();
// Final-wave ambush tick (pool rise / bungee drop / grave rise); call every
// frame once the game is running -- it only acts when level.final_ambush_clk
// is armed by the final wave spawn.
void update_final_wave_ambush();
