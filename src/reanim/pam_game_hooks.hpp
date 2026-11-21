#pragma once

// Game hook for the PvZ2 sod-roll lawn intro (implemented in
// pam_game_hooks.cpp). Kept separate like reanim_game_hooks.cpp: no hard
// dependency — returns false when the resources/pvz2anim assets are missing
// so the caller falls back to the classic sod-roll effect.

namespace pamsod {

// Init the PAM assets (SODROLL pam + atlas). False when they are unavailable
// (or PVZ_CLASSIC_SOD=1 forces the classic roll); callers then fall back.
bool sod_intro_ready();

// Draw one frame of the progressive sod-roll intro on the current (cleared)
// screen: previous-stage lawn as base, the new lawn revealed left-to-right
// up to the PAM roll's leading edge at progress prog (0..1), with the roll
// art on top. Returns false and draws nothing when the PAM assets are
// unavailable — caller should use credit.cpp's draw_sod_roll_frame instead.
bool display_sod_roll_intro_frame(float prog);

// Play the whole SODROLL intro once (looping display_sod_roll_intro_frame).
// Returns false when the PAM assets are unavailable (classic fallback).
bool display_sod_roll_intro();

} // namespace pamsod
