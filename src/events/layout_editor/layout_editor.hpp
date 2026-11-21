/*
F2 layout editor (designer tooling).

Toggle with F2 in ANY scene: install() arms an SDL event filter that opens
the editor as a blocking nested scene straight from the event pump, so every
scene that polls SDL events once per frame (selector/mode screen, level list,
award cutscenes, seed chooser, credit pan, sod roll, READY-SET-PLANT and
gameplay) can be entered without touching that scene's code. While open,
gameplay is frozen (IS_PAUSED holds per-object animation counters) and audio
is paused; both are restored on F2/ESC.

What the editor draws underneath its overlay: game.cpp registers a live
redraw callback per scene via set_scene_redraw() (level list, survey view of
the lawn during the intro, full lawn during gameplay). Scenes with no live
redraw (selector screen, seed chooser, cutscenes) show the frozen
last-presented frame instead -- pixel-accurate, just still.

What can be selected, by current scene (game_state flags):
  + gameplay (IS_GAME_STARTED): plants, zombies, mowers, seed packets, suns
    (bullets too, read-only measure);
  + intro/survey (IS_LEVEL_CHOSEN, game not started): mowers + seed packets;
  + seed chooser (IS_CHOOSING_SEEDS): seed packets only;
  + menus/level list/selector: nothing (mouse coords + dump only).
Plants are never enumerated outside running gameplay: pre-planted flower pots
of a roof level load with the level, and the frozen chooser/menu frames pan
them off-screen -- hit-testing them there selected invisible objects.

Edits are transient and render-only: drag/arrows R-adjust per-object
layout_dx/layout_dy fields (plants/zombies/mowers/suns) or the SeedPacket
layout registry, +/- and the wheel scale through the rsdl debug-scale
registry (all five kinds draw scaled for real, growing about the art's
bottom-center contact point; packets scale about the slot center), G toggles
the lawn grid, F9 dumps userdata/layout_dump.txt (with dx/dy columns and an
offscreen flag). Everything reverts when the editor closes -- the dump is
the hand-off for hand-normalizing constants.
*/
#pragma once
#include <functional>

namespace layout_editor
{
// Arm the F2 -> open-editor event filter. Call once at startup.
void install();

// Register the live redraw for the scene currently on screen; called by
// game.cpp's main loop every frame. nullptr = no live redraw available, the
// editor replays the frozen last frame instead.
void set_scene_redraw(std::function<void()> redraw);
} // namespace layout_editor
