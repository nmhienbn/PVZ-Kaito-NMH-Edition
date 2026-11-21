#pragma once

namespace bossfight
{
// true while the current level is the Zomboss fight (the "boss" special).
bool active();
// Per-tick driver, called from handle_changes (elements_actions.cpp):
// lazily spawns the boss, runs the attack pattern loop, resolves the RV
// warning/impact, wipes the horde when the boss dies.
void update();
// Called when the boss's fight is over: from add_zombie_die when its HP runs
// out, and from BossZombie's destructor so out-of-band deletion (jalapeno
// burn, lawn mower) ends the fight instead of dangling. Idempotent.
void on_boss_killed();

// Jalapeno hook: jalapeno.cpp reports each row it burns the tick its fuse
// fires; the controller consumes the queue to destroy ICE boulders on those
// rows (orig Plant::BurnRow, Plant.cpp:4313-4318). No-op outside the fight.
void on_jalapeno_burn(int row);

// RV landing-zone warning patch (translucent target rect while an RV is in
// the air). RENDER-PATH hook: display_game_layout draws it topmost, exactly
// where update() used to draw it after the layout in the old tick-then-draw
// loop -- the auto-FPS redesign clears and redraws the whole scene after
// the logic ticks, so anything drawn inside update() would be wiped.
void draw_rv_warning();

// Health of the live Zombot body (the 5-10 progress meter reads this --
// repvz UpdateProgressMeter, Board.cpp:5646-5656). False when no boss body
// stands on the lawn (not spawned yet, or dead).
bool boss_health(int &hp, int &max_hp);

// Headless-harness hooks (demo/boss_check.cpp): read/force the attack
// pattern clocks (-1 = locked until its damage-stage gate).
int debug_boss_phase(); // BossZombie::Phase as int; -1 when no boss body
int debug_stomp_clk();
int debug_bungee_clk();
void debug_set_attack_clk(int stomp, int bungee);
} // namespace bossfight
