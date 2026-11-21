#pragma once

// PopCap reanim (per-part skeletal) rendering for in-game plants, mirroring
// reanim_zombie.cpp / pam_plants.cpp: one lazily-initialized module; the
// caller keeps the sprite-sheet path when ready() stays false (missing
// resources/reanim files).
//
// Segment selection is driven by the existing sprite-sheet state machine:
// the plant subclasses already switch directory_num / frame on attack, sun
// production, damage etc., so gameplay timing (pea fire tick, cherry
// explosion, potatomine arming) is fully preserved — only the visual is
// replaced. Each sheet directory maps to a reanim frame range and the sheet
// frame counter is mapped proportionally into it ("sync" mode), so events
// keyed to sheet frames stay aligned; one-shot ranges clamp at their end.
// Pure idle states are the exception: they run a clock-driven lifecycle
// (reanimClock) that loops the idle segment and, on a per-plant hashed
// schedule, inserts a blink/fidget clip (native Wallnut blink segments,
// synthetic eye-swap clips for PeaShooter/Snowpea/Sunflower), matching the
// original game's "idle loop + occasional blink" feel.

#include "reanim.hpp"
#include "reanim_sdl.hpp"

#include <SDL.h>

#include <string>
#include <unordered_set>
#include <vector>

namespace reanimplant {

bool init(ISpriteRenderer& renderer);
bool ready();

// Reanim frame rate of the plant's animation (0 when type has no mapping).
float frameRate(int plantType);

// Pinned on-screen height of the plant's idle art in retail px (the gSkins
// artH column, scale_audit.md S8.2); achieved on screen is this x WORLD_SCALE.
// Static table -- safe before init(). 0 on an out-of-range type.
float plant_idle_height_px(int plantType);

// Draw plant `plantType` (enum PlantType in game_param.hpp) from its sheet
// state: sheetDir = Plants::directory_num, sframe = fractional sheet frame
// (float)frame / assets.frame, nSheet = all_img[sheetDir].n_sheet. Placement: horizontally centered on
// [cellX1, cellX2], bottom of the reference bbox sitting at footY (the
// sheet's bottom edge), scaled so the reference ("idle") bbox height matches
// gameHeight — i.e. the plant keeps the size it had as a sprite sheet.
// reanimClock is per-instance free-running time (frames) that drives the
// idle/fidget lifecycle. Returns false when this state has no reanim mapping
// (caller falls back to PAM / sprite sheets).
// extraHide/extraShow: debug-tooling overrides (same semantics as
// reanimz::drawZombie — names compared lowercase): extraHide hides tracks on
// top of the built-in rules (ornament_*), extraShow reveals a track instead
// (wins over extraHide and the built-in rules, and renders it even on f=-1
// frames). partsDrawn: out count of drawn parts.
// Looping states are played with the loop-seam duplicate frame trimmed from
// the play range (when frame `last` duplicates `first`, the seam stays
// reachable purely as the wrap interpolation target).
// `sampledFrame` (optional) receives the absolute reanim frame drawn this
// call (fractional = interpolated; -1 for the SPLITPEA compose); debug and
// test harnesses use it to check loop-boundary continuity.
// scaleMul (F2 layout editor): extra multiplier on the calibrated scale,
// growing the art about its pinned bottom-center (refCX/refBottom stay on
// (cell center, footY)); 1.0 = unchanged.
bool drawPlant(int plantType, int sheetDir, float sframe, int nSheet,
               float reanimClock, int cellX1, int cellX2, int footY,
               int gameHeight, bool flash,
               const std::unordered_set<std::string>* extraHide = nullptr,
               const std::unordered_set<std::string>* extraShow = nullptr,
               int* partsDrawn = nullptr, float* sampledFrame = nullptr,
               float flash_fade = 1.0f, float scaleMul = 1.0f);

// Track names (lowercase) of the plant's reanim doc; empty when the plant has
// no loaded skin. For debug tooling.
std::vector<std::string> trackNames(int plantType);

// Per-track visibility snapshot drawPlant would produce at this sheet state
// (same state->segment mapping; clock-driven idle states are sampled inside
// their main idle loop). Empty when the state has no reanim mapping.
struct TrackStatus {
    std::string name; // lowercase
    bool hasImage;    // track carries an image somewhere in the timeline
    bool codeHidden;  // hidden by reanimplant's own rules (ornament_*)
    bool dataHidden;  // frame flag f == -1 at the sampled frame
    bool forceShown;  // always false for plants (mirrors reanimz::TrackStatus)
};
std::vector<TrackStatus> trackStatuses(int plantType, int sheetDir,
                                       float sframe, int nSheet);

// --- debug tooling (anim viewer): direct label playback ----------------------
struct SegmentInfo {
    std::string name; // "anim_*" label
    int first, last;  // absolute frame range in the plant's doc
};
// Animation labels of the plant's reanim doc, in timeline order; empty when
// the plant has no loaded skin.
std::vector<SegmentInfo> segments(int plantType);

// Looping play length of one labelled segment in reanim frames (loop-seam
// duplicate frame trimmed, matching drawPlantSegment's play window).
// 0 when the plant/segment is unavailable. For debug tooling.
float segmentLength(int plantType, const char* segment);

// Draw one labelled segment directly (no sheet-state mapping, no idle/fidget
// lifecycle — the raw label, looping; frame is a local reanim frame inside
// the segment, fps 12). Uses the same calibration and visibility defaults
// (isHidden rules) as drawPlant; extraHide/extraShow override them like on
// the drawPlant debug path. SPLITPEA draws the raw range, not the in-game
// head/leaf compose. Returns false when the plant/segment is unavailable.
bool drawPlantSegment(int plantType, const char* segment, float frame,
                      int cellX1, int cellX2, int footY, bool flash,
                      const std::unordered_set<std::string>* extraHide = nullptr,
                      const std::unordered_set<std::string>* extraShow = nullptr,
                      int* partsDrawn = nullptr);

// Per-track visibility snapshot for the raw label playback of
// drawPlantSegment at local frame `frame`.
std::vector<TrackStatus> segmentTrackStatuses(int plantType, const char* segment,
                                              float frame);

} // namespace reanimplant
