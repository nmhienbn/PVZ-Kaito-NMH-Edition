#pragma once

// PvZ2 PAM rendering for in-game plants, mirroring reanim_zombie.cpp: one
// lazily-initialized module, caller falls back to the sprite sheets when
// ready() stays false (missing resources/pvz2anim assets).
//
// Segment selection is driven by the existing sprite-sheet state machine:
// the plant subclasses already switch directory_num / frame on attack, sun
// production, damage etc., so gameplay timing (pea fire tick, cherry explosion,
// potatomine arming) is fully preserved — only the visual is replaced. Each
// sheet directory maps to a pam segment; the sheet frame counter is mapped
// proportionally into the segment ("sync" mode) so events keyed to sheet
// frames stay aligned, except the potatomine's unarmed hold which runs on a
// free pam clock (`plant` clamps at the sheet's pinned last frame, then
// `plant_idle` loops).

#include "pam.hpp"
#include "pam_sdl.hpp"

#include <SDL.h>

namespace pamplant {

bool init(ISpriteRenderer& renderer);
bool ready();

// PAM frame rate of the plant's animation (0 when type has no PAM mapping).
float frameRate(int plantType);

// Draw plant `plantType` (enum PlantType in game_param.hpp) from its sheet
// state: sheetDir = Plants::directory_num, sframe = fractional sheet frame
// (float)frame / assets.frame.
// nSheet = all_img[sheetDir].n_sheet. Placement: horizontally centered on
// [cellX1, cellX2], bottom of the reference bbox sitting at footY (the
// sheet's bottom edge), scaled so the reference ("idle") bbox height matches
// gameHeight — i.e. the plant keeps the size it had as a sprite sheet.
// pamClock is per-instance free-running time (frames) for clock-driven loops.
// Returns false when this state has no PAM mapping (caller falls back).
// flash_fade: 0..1 strength of the damage flash; the game hook feeds it
// attacked_time/MAX_TIME_BLINK so the flash fades out over the blink budget,
// same ramp as reanim_plant.cpp. 1.0 = full strength.
bool drawPlant(int plantType, int sheetDir, float sframe, int nSheet,
               float pamClock, int cellX1, int cellX2, int footY,
               int gameHeight, bool flash, float flash_fade = 1.0f);

} // namespace pamplant
