#pragma once

/*
Level 4-10 "stormy night": the fog-pool board plays in pitch black, mirrored
from repvz (Challenge::UpdateStormyNight / DrawStormNight). Outside lightning
the whole lawn sits under a fully opaque black sheet (alpha 255); flashes come
in three repvz styles - bright strike with a dimmer second pulse, one long
slow fade, one short fade - whose darkness alpha ramps from ~0 back to 255
with a small flicker. Thunder cracks as each flash starts; rain loops the
whole level (rain.ogg). Black flash gaps last 1.5-6 s.
The UI (seed packets / belt, sun counter, shovel, pause icon, progress bar)
and floating suns are re-drawn above the darkness like the original's
DrawUIBottom/DrawTopRightUI, and Plantern does NOT pierce the storm darkness
(only lightning does - repvz DrawStormNight has no plant light pass; level
4-10's belt doesn't even offer Plantern).
The white additive glow repvz lays over the flash is dropped: the clone has
no white/blendable sheet texture, and the alpha ramp alone carries the beat.
Nothing here touches the fog overlay (that is the fog agent's render layer;
the storm level itself ships fog_column -1 / none).
*/
namespace storm {

// One tick of the storm clock (dark/flash schedule + rain loop). Called from
// handle_changes each unpaused game tick.
void update();
// Darkness overlay drawn over the finished frame (end of display_game_layout),
// then the top UI and suns are re-drawn above it (repvz draw order).
void draw_overlay();
// Current darkness alpha (255 pitch black, ~0 fully lit); 0 on non-storm
// levels. Pure - safe for headless tests.
int overlay_alpha();

} // namespace storm
