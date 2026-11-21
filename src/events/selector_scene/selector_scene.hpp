#pragma once

#include "draw/rsdl.hpp"

// Selector/mode screen ("bia mo chon mode"), a la original PvZ1: tombstone
// store/almanac and the wood-plank mode buttons (ADVENTURE reachable; the
// rest announce "not available yet"). Per-frame handler mirroring
// handle_choosing_level_screen: draws one frame and processes one event
// batch. Clears IS_SELECTOR_SCREEN when ADVENTURE is picked so the main loop
// continues into the existing level list.
void handle_selector_screen();

// The draw half of one selector frame without the present (announcer text
// included). Hook for the offscreen frame-dump harness, which must read the
// framebuffer back before update_screen() swaps it away.
void render_selector_frame();
