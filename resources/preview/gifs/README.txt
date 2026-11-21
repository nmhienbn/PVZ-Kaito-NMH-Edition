Preview GIFs — rendered through the game's own code paths (headless SDL build
of src/reanim/*: zombies via reanimz::drawZombie / drawDeathPart, plants via
reanimplant::drawPlant). No separate viewer math involved.

TIMING
------
- Game loop: 60 ticks/s (resources/dat/parameters.json FPS=60).
- Reanim data: 12 fps; the game advances reanim time 0.2 frame/tick
  (zombies: Zombie::reanim_rate, src/elements/zombies/zombie_struct.hpp:65;
   plants: reanim_clock += frameRate/FPS, src/reanim/reanim_plant_hooks.cpp:39).
- => 1 reanim frame per 5 ticks = 83.33 ms. GIF frame durations alternate
  80/80/90 ms (centisecond granularity), averaging exactly 83.33 ms/frame;
  "_83ms" in the file names. One capture per 5 game ticks.

ZOMBIE (plain, ZOMBIE_TYPE normal) — asset: resources/reanim/Zombie.reanim
---------------------------------------------------------------------------
FIXED (2026-10-07): the in-game asset is now the retail PvZ1 data again ---
the decompiled fla XML (504 frames, all parts at the original 0.8 scale),
replacing the 2Pak mod's re-authored 543-frame doc which had distorted
per-part proportions (coat ~+47% wider than the head vs retail, hunched
gait) and a merged walk. Part PNGs were already the pvz1 originals.

Effective in-game ranges (from f-keyframe labels):

  GIF                    state            reanim frames (0-based)
  zombie_walk1_83ms.gif  ZOMBIE_WALK1_DIR  44..90   (47f, 3.9s loop)
  zombie_walk2_83ms.gif  ZOMBIE_WALK2_DIR  91..137  (47f, 3.9s loop)
  zombie_eat_83ms.gif    ZOMBIE_EATING_DIR 138..177 (40f, 3.3s loop; tongue shown)
  zombie_die_83ms.gif    corpse state      178..216 (aging 0..59 of 39f + head pop, 5.0s)

Notes:
- walk1 and walk2 are the original two distinct PvZ1 walk takes (arm/head-bob
  variants; mean frame |diff| ~3.4, loop seams exactly 0).
- die: rendered via reanimz::drawDeathPart (body collapse + tossed-head pop
  physics; pop anchors re-measured on this doc's frame 0). In-game the corpse
  plays the 77-frame take at the original's PlayDeathAnim rate (24-30 fps
  seeded per corpse), then fades over the last 0.1 s of a 1 s hold
  (reanimz::deathLingerTicks ~= fall ticks + 60; pool deaths sink faster).
- Zombie scale/placement: fixed ZOMBIE_SCALE = 1.01 (reanim_zombie.cpp:127,
  our 101px day tile / pvz1's 100px tile), box 166x144, feet at box bottom.
  Walk-union bbox ~88-93x123-130 px, center stays within +-3 px. Loop seams:
  walk1/walk2/eat last->first exactly 0 (clean loops).

PLANTS — assets: PeaShooter.reanim / Snowpea.reanim (resources/reanim)
----------------------------------------------------------------------
  peashooter_idle_83ms.gif    anim_full_idle 43..59 via clock-driven idle
                              cycle (17f idle + 4f blink slot = 21 frames);
                              blink fires in cycle 1 -> GIF = cycles 0-1 (42f, 3.5s)
  peashooter_attack_83ms.gif  anim_shooting 25..42, sheet-synced: sheet 60f @
                              2 ticks/frame = 120 ticks (2.0 s); captured every
                              5 ticks -> 24 frames (reanim interpolation on)
  repeater_idle/attack        same dirs as peashooter (PEASHOOTER_*); 42f / 24f
  snowpea_idle_83ms.gif       anim_full_idle 45..61; blink in cycle 3 -> 42f
  snowpea_attack_83ms.gif     anim_shooting 25..44, sheet 30f @ 4 ticks/frame
                              = 120 ticks (2.0 s) -> 24 frames

Verified behavior while rendering:
- isHidden() hides tracks "leaf" and "peashooter_eyebrow" for PEASHOOTER_TYPE
  and REPEATER_TYPE (reanim_plant.cpp:660-664). Confirmed via
  trackStatuses(): both tracks codeHidden=1 (data flags them visible
  dataHidden=0, so the C++ hide rule is what removes them — a raw sheet/XML
  render would wrongly show the baked-in repeater face).
- Repeater is NOT identical to peashooter: drawRepeaterOverlays adds
  repeater_eyebrows + repeater_leaf (mean frame diff 0.41; repeater bbox is
  ~25 px taller from the leaf tuft).
- Snowpea's doc has no leaf/eyebrow tracks (nothing to hide).
- No empty/black frames in any capture; nothing clipped at canvas edges.
- Plant on-screen size comes from per-skin calibration (fitted scale at load:
  PeaShooter 1.162, Repeater 1.162, Snowpea 1.184), art bottom parked at
  footY with the sheet-measured padBottom — same placement math as in-game.
- Idle fidget cadence is per-plant-cell hashed (pFidget 0.3/cycle), so a
  given GIF shows one deterministic blink schedule; in-game neighboring
  plants blink on different cycles by design.

Regeneration:
  tool source: /tmp/pvz_preview/anim_preview.cpp (built with the same flags as
  `make -f Makefile.linux reanim_gameview`, links src/reanim/*.cpp);
  assembly: /tmp/pvz_preview/make_gifs.py (PIL, duration=83ms, disposal=2).
