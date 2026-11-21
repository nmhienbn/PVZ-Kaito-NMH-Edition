#pragma once

// One-shot reanim effects: short-lived transient animation spawned at a
// world position and erased once its doc plays through (zombie_parts style:
// aged and drawn per row during the element pass).
//
// Registered effects (docs under resources/reanim/, hand-authored over the
// 2Pak particle sprites the original's compiled particle defs name):
//   FX_SPLASH (splash.reanim, 2Pak XFL conversion): water contact -- ducky
//     tube zombies sinking, the squash dropping into a pool tile.
//   FX_ZOMBIE_GRAVE_RISE (zombie_grave_rise.reanim): port of the original
//     PARTICLE ZombieRise (repvz Zombie::RiseFromGrave, Zombie.cpp:8265-8345)
//     -- its compiled emitters are IMAGE_ZOMBIEARM + IMAGE_DIRTSMALL
//     (DirtSplash) + IMAGE_DIRTBIG (DirtClumps): a zombie arm thrusts out of
//     the fresh mound while dirt clods spray.
//   FX_WHACK_ZOMBIE_RISE (whack_zombie_rise.reanim): port of the original
//     PARTICLE WhackAZombieRise (same RiseFromGrave site, Zombie.cpp:8336) --
//     DirtSplash + DirtClumps only, no arm.
//   FX_GRAVE_SPROUT (grave_sprout.reanim): port of PARTICLE GraveStoneRise
//     (IMAGE_DIRTSMALL only) for a stone breaking out of the ground.
//   FX_GRAVEBUSTER_CHOMP / FX_GRAVEBUSTER_DIE (grave_buster_chomp/die.
//     reanim): ports of PARTICLE GraveBuster (IMAGE_ROCKSMALL pebble spray
//     while chewing) and GraveBusterDie (dirt burst when the stone is gone --
//     the retail emitter list is DoomBase/IMAGE_GRAVEBUSTER_DIRT only).
//   FX_BUTTER_SPLAT (butter_splat.reanim): kernel-pult butter landing burst
//     (2Pak particles/Cornpult_butter_splat): the pat grows 0.6 -> 1.2x and
//     fades over ~18 ticks on the victim's crown (the pat itself keeps
//     drawing in Zombie::display).
//   FX_MELON_IMPACT (melon_impact.reanim): melon-pult ground burst playing
//     the 9-frame MelonImpact strip once over ~20 ticks at the touchdown
//     point.
//   FX_PLANT_DIRT (plant_dirt.reanim): the 12-frame plant_dirt ring when a
//     seed goes into soil/grass (pool tiles keep FX_SPLASH),
//     ~20 ticks at the plant's base.
//   FX_DUST_PUFF (dust_puffs.reanim): the 3-frame Dust_puffs strip with a
//     scale/fade tail (~18 ticks); mower trail behind a running cleaner,
//     squash slam, sod-roll front edge.

class Zombie; // the butter splat anchors on the victim's head

namespace reanimfx {

enum FxId
{
    FX_SPLASH = 0,
    FX_ZOMBIE_GRAVE_RISE,
    FX_WHACK_ZOMBIE_RISE,
    FX_GRAVE_SPROUT,
    FX_GRAVEBUSTER_CHOMP,
    FX_GRAVEBUSTER_DIE,
    FX_BUTTER_SPLAT,
    FX_MELON_IMPACT,
    FX_PLANT_DIRT,
    FX_DUST_PUFF,
};

// `x, y`: screen position of the effect origin (splash: the water surface
// point the body hits). `row`: lawn lane, for the per-row draw order.
void spawn(FxId id, int x, int y, int row);

// Butter landing burst on the victim's crown: the same head anchor the
// persistent pat rides (reanimz::headAnchor), legacy sheet offset as the
// fallback. (armor_dir is Zombie-protected, so the anchor runs with -1 --
// exact for head-whole zombies, a segment guess for torn-arm states.)
void spawn_butter_splat(const Zombie *z);

// Trail behind a running mower: call every tick it rolls, a puff spawns
// every MOWER_DUST_EVERY calls (~12 ticks of cadence at 60 tps).
void mower_dust(int x, int y, int row);

// Planting burst at the tile's base: the dirt ring on soil/grass tiles,
// the splash FX on water tiles (pool rows).
void spawn_planted_dirt(int row, int col);

// Sod-roll intro pump: both intros (classic roll in credit.cpp, PvZ2 PAM
// roll in pam_game_hooks.cpp) run their own frame loop without the element
// pass, so this drops a puff at the roll's front edge every ~15 calls
// (call once per intro frame) AND draws/ages the live set for the playable
// rows (otherwise the puffs would queue up and all play on gameplay frame
// one).
void sod_dust(int front_x);

// Draw + age every live effect on `row`; finished ones are erased. Hooked
// into the per-row element pass (after zombie parts).
void display(int row);

// Drop all live effects (level reset / scene change).
void clear();

// Pending instances (headless harness asserts); the _of overload counts one
// effect family only.
int active_count();
int active_count_of(FxId id);

} // namespace reanimfx
