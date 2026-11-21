#pragma once

#include <cmath>

/*The ONE world render scale. Every world object (plants, zombies, bullets,
mowers, suns, particles, ...) draws at its measured retail-pixel size times
this single factor. Per-asset calibration factors elsewhere only undo the
2Pak export's uneven sizing (scale_audit.md R column) -- they are measured
data, not knobs. To grow or shrink the whole game world, change THIS one
number and nothing else. 1.0 = retail pixel size (the scale particles/FX
already use, one_shot_fx.cpp FX_DEFS).*/
inline constexpr float WORLD_SCALE = 1.0f;

/*Seed-packet icon sizing: a packet's plant portrait is scaled so its visible
height reads (plant world px height x WORLD_SCALE x THIS), clamped to the
packet's art box -- the icon inherits the plant's world size (tallnut tall,
puff-shroom small) instead of every icon being stretched to the same box.
Plant world heights are the gSkins artH column (retail px, scale_audit.md
S8.2). 1.0 keeps mid/tall plants at their previous box-clamped ~54 px look
while the small arts shrink proportionally.*/
inline constexpr float SEED_ICON_SCALE = 1.0f;

/*retail px -> screen px. Exact pass-through at WORLD_SCALE == 1.0 (integer
inputs round-trip unchanged), so existing draws stay byte-identical.*/
inline int world_px(int retail_px) { return (int)lroundf(retail_px * WORLD_SCALE); }
