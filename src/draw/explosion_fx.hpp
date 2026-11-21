#pragma once

// One-shot comic blast caption ("POWIE!"-style) layered on top of the plant
// explosions. PvZ1 renders it as a particle, so it lives outside the
// reanim/sheet art: the caller runs it for ~1s from the blast start, anchored
// by caption center (cx, cy). The caption pops in (60% -> 110% of the base
// width over 3/4 of its life), then fades out over the last third. It pauses
// and despawns with the game because it is driven by the caller's tick
// counter. Returns false once the caption has finished.

#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "game_param.hpp"

#include <algorithm>

extern Window win;

inline bool draw_boom_caption(int cx, int cy, int ticks_elapsed)
{
    const int total = FPS; // ~1 s at the game's tick rate
    if (ticks_elapsed < 0 || ticks_elapsed >= total)
        return false;
    const int base_w = world_px(150); // on-screen width at 100%
    const int pop_end = total * 3 / 4;
    float pop = std::min(1.0f, (float)ticks_elapsed / (float)pop_end);
    int w = (int)(base_w * (0.6f + 0.5f * pop));
    const int fade = total / 3; // fade-out over the last third
    if (ticks_elapsed >= total - fade)
        win.set_texture_alpha(BOOM_DIRECTORY, 255 * (total - ticks_elapsed) / fade);
    win.draw_png(BOOM_DIRECTORY, cx - w / 2, cy - w / 2, w, w);
    if (ticks_elapsed >= total - fade)
        win.set_texture_alpha(BOOM_DIRECTORY, all_img[BOOM_DIRECTORY].alpha);
    return true;
}
