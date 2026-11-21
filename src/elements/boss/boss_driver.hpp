#pragma once

// Zomboss dome driver (Zombie_Boss_driver.reanim): the zombie sitting in the
// Zombot's head dome. The driver doc is authored in head-local coordinates,
// so it is drawn through reanim's AttachReanimation equivalent
// (DrawOptions::attachFrom) anchored on the Zomboss doc's Boss_head track.

namespace bossdriver {

// Raglan init on first call (loads Zombie_boss.reanim for the head anchor +
// the driver doc). False when assets are missing -- caller just skips it.
bool ready();

// Enable/disable the driver draw (headless visual-diff harness).
void set_enabled(bool on);
bool enabled();

// Draw the driver for the boss body currently rendered by
// BossZombie::render_zombie: same x/y/scale anchor as
// reanimz::drawSkinSegment(SKIN_ZOMBOSS, seg, anim_t, x, y, ..., scale_mul).
// Returns the number of part quads drawn (0 when disabled/unavailable).
int draw(const char *boss_seg, float boss_anim_t, bool boss_loop,
         int x_location, int y_location, float scale_mul,
         bool cold, bool attacked);

} // namespace bossdriver
