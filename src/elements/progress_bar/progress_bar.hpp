#pragma once

// advance = false: draw-only pass for the 4-10 storm redraw (see
// storm.cpp redraw_ui_on_top) -- the chase/raise state must NOT advance
// twice in one frame; every other caller keeps the default (true).
void display_progress_bar(const bool advance = true);

// Headless-check hooks (demo/progress_bar_check.cpp): the smoothed fill width
// in original mProgressMeterWidth units (0 == meter hidden, 150 == full) and
// the flag-meter layout read off the current level's wave plan.
int progress_bar_fill_units();
int progress_bar_num_flags();
// Screen x of the pole for the flag carried by the given 0-based wave index
// (repvz Board.cpp:6840); -1 when the level is waveless/waves unloaded.
int progress_bar_flag_x(int flag_wave_index);
// Raise-beat state of the flag meter (headless checks): how many flag waves
// have started their raise animation, and ticks into the newest one's beat.
int progress_bar_raised_flags();
int progress_bar_flag_raise_clk();
