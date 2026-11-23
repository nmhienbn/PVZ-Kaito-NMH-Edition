#include "boss_zombie.hpp"
#include "../zombies/roof/roof_shared.hpp"
#include "boss_driver.hpp"
#include "boss_fight.hpp"
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "game_param.hpp"
#include "music/music.hpp"
#include "reanim/reanim_zombie.hpp"
#include <cstdio>

extern int game_state;
extern Level level;
extern Window win;

namespace
{
// boss_hp: 2000 pea-units = the original's 40000 HP (PvZ-Portable
// Zombie.cpp:692, adventure mode). The vulnerable-window gate plus the
// head-down lunge into mid-lawn decides how fast that pool drains, not this
// constant. Multiple of 2 and 4 on purpose: every damage source on this lane
// (melon 4, melon splash 2, butter 2, cabbage 1, ice-shroom chip 1) divides
// it exactly, so the kill still lands clean even if the out-of-folder
// "== 0" death sweeps ever miss an overshoot.
const int BOSS_HP = 2000;
// Vestigial: the ball is untouchable by damage (is_underground), so no
// attacker ever reduces this; only the counter sweeps remove a boulder.
// Kept high so the global "health <= 0" death sweeps can never reap it,
// even from the ice-shroom chip (1 per blast).
const int BOULDER_HP = 100000;
// ~120 px/s: crosses the lawn in ~6 s (orig fireball is barely faster; the
// old 3 px/tick crossed in under 4 s, before two pults could even cycle).
const int BOULDER_SPEED = 2;
} // namespace

BossZombie::~BossZombie()
{
    // Any removal of the boss body (bullet kill, or the jalapeno/mower
    // insta-delete holes living outside this folder) ends the fight; the
    // controller wipes the horde on the next tick instead of dereferencing
    // a dangling boss pointer. Idempotent with add_zombie_die.
    bossfight::on_boss_killed();
}

void BossZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = max_hp = BOSS_HP;
    directory_num = ZOMBOSS_WALK_DIRECTORY;
}

void BossZombie::set_phase(Phase p)
{
    phase = p;
    anim_t = 0.0f;
    float len = 0.0f;
    switch (p)
    {
    case PH_HEAD_ENTER:
        len = reanimz::skinSegmentLength(reanimz::SKIN_ZOMBOSS, "anim_head_enter");
        break;
    case PH_HEAD_LEAVE:
        len = reanimz::skinSegmentLength(reanimz::SKIN_ZOMBOSS, "anim_head_leave");
        break;
    case PH_STOMP:
        len = reanimz::skinSegmentLength(reanimz::SKIN_ZOMBOSS, "anim_stomp_1");
        break;
    case PH_BALL:
        len = reanimz::skinSegmentLength(reanimz::SKIN_ZOMBOSS, "anim_head_attack_1");
        break;
    case PH_RV:
        len = reanimz::skinSegmentLength(reanimz::SKIN_ZOMBOSS, "anim_stomp_2");
        break;
    case PH_FOOT_STOMP:
    {
        // anim_stomp_1..4 selected by the target row pair (repvz
        // BossStompAttack, Zombie.cpp:10002-10010).
        char seg_name[24];
        snprintf(seg_name, sizeof(seg_name), "anim_stomp_%d", stomp_row_pair + 1);
        len = reanimz::skinSegmentLength(reanimz::SKIN_ZOMBOSS, seg_name);
        break;
    }
    case PH_BUNGEE:
        len = reanimz::skinSegmentLength(reanimz::SKIN_ZOMBOSS, "anim_bungee_1_enter");
        break;
    default:
        break;
    }
    anim_rate = 0.2f;
    (void)len;
}

bool BossZombie::on_move_tick()
{
    return true; // the Zombot never walks; the fight controller drives it
}

bool BossZombie::on_reach_plant(const int &)
{
    return true;
}

bool BossZombie::can_eat_plant() const
{
    return false;
}

void BossZombie::determine_appearance(vector<ZombiePart> &)
{
    // The boss flashes when hit; no armor stages.
}

void BossZombie::add_zombie_die(vector<ZombiePart> &zombie_parts)
{
    play_sound_effect(BOSS_EXPLOSION_MUSIC_DIRECTORY);
    if (reanimz::ready())
        zombie_parts.push_back(ZombiePart(reanimz::DEATH_PART_IMG, 1, row, x_location,
                                          ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                          reanimz::SKIN_ZOMBOSS));
    bossfight::on_boss_killed();
}

void BossZombie::render_zombie()
{
    const char *seg = "anim_idle";
    bool loop = true;
    switch (phase)
    {
    case PH_HEAD_ENTER:
        seg = "anim_head_enter";
        loop = false;
        break;
    case PH_HEAD_DOWN:
        seg = "anim_head_idle";
        break;
    case PH_HEAD_LEAVE:
        seg = "anim_head_leave";
        loop = false;
        break;
    case PH_STOMP:
        seg = "anim_stomp_1";
        loop = false;
        break;
    case PH_BALL:
        seg = "anim_head_attack_1";
        loop = false;
        break;
    case PH_RV:
        seg = "anim_stomp_2";
        loop = false;
        break;
    case PH_FOOT_STOMP:
    {
        static char stomp_seg[24];
        snprintf(stomp_seg, sizeof(stomp_seg), "anim_stomp_%d", stomp_row_pair + 1);
        seg = stomp_seg;
        loop = false;
        break;
    }
    case PH_BUNGEE:
        seg = "anim_bungee_1_enter";
        loop = false;
        break;
    default:
        break;
    }
    // Zomboss doc compensation: drawSkinSegment's retail pivot anchors the
    // doc's (30, 120) point, but the boss art sits ~(672.75, 325.47) doc
    // units off the box origin at scale 1 -- the compensation pair restores
    // the measured idle placement, and bossdriver::draw passes the same pair
    // so the dome driver stays glued to the body.
    int boss_dx, boss_dy;
    reanimz::bossDocCompensation(0.6f * WORLD_SCALE, boss_dx, boss_dy);
    if (!reanimz::drawSkinSegment(reanimz::SKIN_ZOMBOSS, seg, anim_t,
                                  x_location, y_location,
                                  cold_time > 0, attacked_time > 0,
                                  boss_dx, boss_dy, 0.6f * WORLD_SCALE, loop))
        Zombie::render_zombie();
    else
    {
        // Dr. Zomboss visible inside the dome: the driver doc rides the
        // Boss_head track of the zombot doc (same frame the body just drew).
        bossdriver::draw(seg, anim_t, loop, x_location, y_location, 0.6f * WORLD_SCALE,
                         cold_time > 0, attacked_time > 0);
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && !check_status(game_state, IS_PAUSED); _lt--)
            anim_t += anim_rate;
    }

    // HP bar: top-right corner, shrinks with remaining health.
    const int bar_w = 300, bar_h = 14, bar_x = WINDOW_WIDTH - bar_w - 20, bar_y = 8;
    win.draw_rect(bar_x, bar_y, bar_w, bar_h, RGB(20, 20, 20));
    int fill = (int)((long)bar_w * health / max_hp);
    if (fill > 0)
        win.fill_rect(bar_x, bar_y, fill, bar_h, RGB(200, 40, 40));
}

//*-*-*-*-*-*- Boulder -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

BoulderZombie::BoulderZombie(bool is_ice)
{
    ice = is_ice;
}

BoulderZombie::~BoulderZombie()
{
}

void BoulderZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = BOULDER_HP;
    directory_num = ZOMBOSS_WALK_DIRECTORY; // sprite-sheet fallback slot
    is_moving = true;
}

bool BoulderZombie::on_move_tick()
{
    x_location -= BOULDER_SPEED;
    // Flatten every plant tile the boulder rolls over.
    int col = get_block_col(x_location + 80);
    if (col >= 0 && col < HORIZ_BLOCK_COUNT && cells[row][col].is_planted)
        crush_plants_at(row, col);
    // Roll off the lawn past the (squashed) roof cleaner at ~200; still far
    // short of the house-loss line (x + ZOMBIE_EXACT_LOCATION < 180, i.e.
    // x < 110) so the ball never ENDS the run by itself.
    if (x_location < X_UPPER_LEFT - 100)
        gone = true;
    return true;
}

bool BoulderZombie::on_reach_plant(const int &)
{
    return true; // the crush happens in on_move_tick; never stops to bite
}

bool BoulderZombie::can_eat_plant() const
{
    return false;
}

bool BoulderZombie::despawn_ready() const
{
    return gone;
}

void BoulderZombie::determine_appearance(vector<ZombiePart> &)
{
    // Flash on hit, no degrade stages.
}

void BoulderZombie::add_zombie_die(vector<ZombiePart> &)
{
    // Shot apart: no corpse (approximation).
}

void BoulderZombie::render_zombie()
{
    int skin = ice ? reanimz::SKIN_BOSS_ICEBALL : reanimz::SKIN_BOSS_FIREBALL;
    // Retail-size ball at the retail pivot (scale_audit 8.4): the fire doc's
    // total bottom (256.6, incl. the superglow halo) keeps the ball's floor
    // at y+225.6 -- the same pixel the fitted 0.84 draw landed on. The doc
    // pivot (30, 120) sits at the ball's center, so (x+10, y-43) keeps the
    // ball's screen center on the boulder slot.
    if (!reanimz::drawSkinSegment(skin, "anim_role", anim_t,
                                  x_location + 10, y_location - 43,
                                  false, attacked_time > 0,
                                  0, 0, 1.0f * WORLD_SCALE, true))
        return; // no sprite-sheet fallback worth drawing
    // anim advances per logic tick, not per render (auto-FPS)
    for (int _lt = logic_ticked; _lt > 0 && !check_status(game_state, IS_PAUSED); _lt--)
        anim_t += 0.25f;
}
