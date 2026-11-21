#pragma once

// Jalapeno lane fire (repvz Board::DoFwoosh / Board::UpdateFwoosh,
// Board.cpp:9743-9805): when the pepper goes off, twelve staggered flame
// stations burn across its row -- the looping "anim_flame" take of
// resources/reanim/fire.reanim for the first half of the fwoosh countdown,
// then the one-shot "anim_done" burn-out rolling station by station. Kept
// next to the plant; the transient-FX layer (one_shot_fx) has no staggered
// per-station lifecycle, so this registry lives here.
namespace jalapeno_fx {

// Spawn the twelve flame stations of `row` (called by Jalapeno::burn_row).
void spawn_lane_fire(int row);
// Draw + age `row`'s flames during render_elements' row pass.
void display_row(int row);
// Level reset (Elements::reset): drop every live flame.
void clear();
// Headless test hook: live stations on `row`.
int active_count(int row);

} // namespace jalapeno_fx
