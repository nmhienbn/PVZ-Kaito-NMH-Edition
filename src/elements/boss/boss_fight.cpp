#include "boss_fight.hpp"
#include "../elements.hpp"
#include "../zombies/bungee/bungee_zombie.hpp"
#include "../zombies/roof/roof_shared.hpp" // crush_plants_at
#include "../zombies/zombie_type.hpp"
#include "boss_zombie.hpp"
#include "draw/rsdl.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/plants/plants.hpp"
#include "game_param.hpp"
#include "music/music.hpp"
#include "reanim/reanim_zombie.hpp"
#include "support.hpp"
#include <utility>
#include <vector>

extern Level level;
extern Elements game_characters;
extern Map cells;
extern Window win;

namespace bossfight
{
namespace
{
// Pattern timers (game ticks, 60/s), matching the original Zombot loop
// (PvZ-Portable Zombie.cpp UpdateBoss, original runs at 100 tps; centiticks
// x 0.6 = ticks):
//   summon zombies: 450-550 -> 350-450 -> 150-250 cs (4.5-5.5 -> 3.5-4.5 ->
//     1.5-2.5 s), re-armed per damage stage; exactly ONE zombie per summon,
//     dropped mid-roof; dropped type gated by fight age (normal -> cone ->
//     pail -> full mix at 125 s, BossSpawnContact).
//   head-down (vulnerable) attack: every 4000-5000 cs, head stays down
//     ~800 cs; damage only lands in this window.
//   fire/ice ball: no separate timer -- spat automatically mid-window.
//   RV throw: no separate timer -- the bungee clock's expiry rolls it at
//     1/4 (adventure): RV instead of the bungee raid (Zombie.cpp:10300-10311).
const int SPAWN_CLK_MIN = 270;   // mode 0 reinforcement cadence (4.5-5.5 s,
                                 // RandRangeInt(450,550)cs, Zombie.cpp:9799)
const int SPAWN_CLK_MAX = 330;
const int SPAWN_CLK_MID_MIN = 210; // mode 1 (3.5-4.5 s, :9803)
const int SPAWN_CLK_MID_MAX = 270;
const int SPAWN_CLK_LATE_MIN = 90; // mode 2 (1.5-2.5 s, :9807)
const int SPAWN_CLK_LATE_MAX = 150;
const int HEAD_CLK = 2400;       // 40-50 s between head-down attacks
                                 // (RandRangeInt(4000,5000)cs, Zombie.cpp:9989)
const int HEAD_ENTER_TICKS = 120; // hydraulics lowering (still invulnerable)
const int HEAD_DOWN_TICKS = 480;  // vulnerable window ~8 s (orig ~800 @100tps)
const int HEAD_LEAVE_TICKS = 120; // rising back (invulnerable)
const int BALL_AT_DOWN_LEFT = 300; // ball spat 3 s into the window (orig ~5 s)
const int RV_WARN_TICKS = 80;
const int BOSS_ACTION_TICKS = 90; // stomp/RV one-shot phase length
// Foot stomp (repvz BossStompAttack:5500-6500cs re-arm -> 55-65 s here).
// The counter only starts ticking once the first RV lands at damage stage 2
// (orig arms 4000cs at BossRVLanding); a stomp that finds no plant in
// columns 5-8 fizzles but still burns its cooldown (orig re-arms before the
// row check).
const int STOMP_CLK_MIN = 3300;
const int STOMP_CLK_MAX = 3900;
const int STOMP_ARM_TICKS = 2400;   // orig 4000cs after the first RV landing
const int STOMP_CRUSH_AT = 45;      // mid-phase thud (orig anim event ~0.5)
// Bungee raid (repvz BossBungeeAttack: 4000-5000cs -> 40-50 s; unlocked by
// the stage-1 transition -- hp < 4/5 max -- which fires one immediately).
const int BUNGEE_CLK_MIN = 2400;
const int BUNGEE_CLK_MAX = 3000;
const int BUNGEE_ARM_LATE = 3900;   // re-arm once stage 2 starts, orig 6500cs
const int BUNGEES_DROP_AT = 54;     // contact at anim time 0.4 of the enter
const int NUM_BOSS_BUNGEES = 3;     // orig NUM_BOSS_BUNGEES
// Body anchor. Standing, the Zombot sits past the right grid edge; for the
// head-down attack it lunges mid-lawn (orig: the head comes down around
// columns 6-7). In the clone this ALSO fixes damage delivery: lobbed shots
// pick the nearest zombie ahead, so at the standing anchor the massive body
// sat behind every dropped zombie and soaked nothing -- the head could
// almost never be targeted at all. At HEAD_X_DOWN the head's torso point
// (x+60 = 615) sits in front of fresh drops (x+60 = 660), so the boss lane's
// pults connect whenever the lane ahead of the drop point is clear.
const int BOSS_STAND_X = 890;
const int HEAD_X_DOWN = 555;

struct Fight
{
    int level_num = -1;    // lazy reset key
    bool spawned = false;
    bool boss_killed = false;
    bool wiped = false;
    int age = 0;           // fight ticks since the Zombot appeared
    int spawn_clk = 300;   // first drop at 5 s (orig mSummonCounter = 500
                           // at spawn, Zombie.cpp:696)
    int head_clk = HEAD_CLK;
    int phase_ticks = 0;   // remaining ticks of the current one-shot phase
    int ball_left = -1;    // >0: ticks of this window until the ball spits
    BossZombie *boss = nullptr;
    // RV throw state
    int rv_warn = -1;      // -1 idle; >=0 warning ticks left before impact
    int rv_row = 0, rv_col = 0;
    // New attacks (F4)
    int stage = 0;              // boss damage stage seen (orig mBossMode)
    int stomp_clk = -1;         // -1: locked until the stage-2 RV (orig: the
                                // stomp counter is armed at BossRVLanding)
    int stomp_pair = -1;        // row-pair index of the running foot stomp
    bool stomp_crushed = false; // thud already applied this stomp
    int bungee_clk = -1;        // -1: locked until damage stage 1
    bool bungees_dropped = false;
    int bungee_col0 = 0;        // first of the 3 adjacent target columns (debug)
    // Jalapeno burn notification: jalapeno.cpp pushes each burned row here
    // when its fuse fires (bossfight::on_jalapeno_burn); update() consumes
    // the queue against the ICE boulders below.
    vector<int> jalapeno_burn_rows;
};
Fight f;

// Reinforcement roster by fight age (orig BossSpawnContact, Zombie.cpp:9830-
// 9859: normal only for the first 35 s, cone until 80 s, pail until 125 s,
// then the full roof mix). The late list is gBossZombieList verbatim
// (Zombie.cpp:116-129); the top row never gets the gargantuar (:9848-9852).
const int DROP_EARLY[] = {NORMAL_TYPE};
const int DROP_MID[] = {CONE_TYPE};
const int DROP_MAIN[] = {BUCKET_TYPE};
const int DROP_LATE[] = {CONE_TYPE, BUCKET_TYPE, FOOTBALL_TYPE, POLEVAULT_TYPE,
                         JACKBOX_TYPE, LADDER_TYPE, ZOMBONI_TYPE, CATAPULT_TYPE,
                         POGO_TYPE, NEWSPAPER_TYPE, DOOR_TYPE, GARGANTUAR_TYPE};

float boss_hp_frac()
{
    return f.boss ? (float)f.boss->get_health() / f.boss->max_health() : 1.0f;
}

// Boss damage stages (repvz Zombie::GetBodyDamageIndex, Zombie.cpp:5836-5851:
// for the boss -- unlike every other zombie -- the thresholds are 4/5 and
// 1/2 of max health).
int damage_stage()
{
    float h = boss_hp_frac();
    return h < 0.5f ? 2 : h < 0.8f ? 1 : 0;
}

bool is_idle()
{
    return f.boss && f.boss->get_phase() == BossZombie::PH_IDLE;
}

void drop_zombies()
{
    const int *roster = DROP_EARLY;
    int roster_n = 1;
    // Age gates (orig BossSpawnContact, Zombie.cpp:9833-9855: < 35 s normal,
    // < 80 s cone, < 125 s pail, then the full mix -- 3500/8000/12500 cs).
    if (f.age >= 7500) // 125 s
    {
        roster = DROP_LATE;
        roster_n = (int)(sizeof(DROP_LATE) / sizeof(int));
    }
    else if (f.age >= 4800) // 80 s
    {
        roster = DROP_MAIN;
        roster_n = (int)(sizeof(DROP_MAIN) / sizeof(int));
    }
    else if (f.age >= 2100) // 35 s
    {
        roster = DROP_MID;
        roster_n = (int)(sizeof(DROP_MID) / sizeof(int));
    }

    // Orig: exactly ONE zombie per summon (BossSpawnContact AddZombieInRow),
    // dropped mid-roof (mPosX = 600). The top row never draws the gargantuar
    // (Zombie.cpp:9848-9852): re-roll such a pick off the roster instead.
    for (int tries = 0; ; tries++)
    {
        int type = roster[rand(0, roster_n - 1)];
        Zombie *z = init_zombie(type, level.level_num);
        if (type == GARGANTUAR_TYPE && tries < 8 &&
            z->row == level.playable_rows.front())
        {
            delete z; // top-row gargantuar: pick again
            continue;
        }
        z->wave_id = -1;
        // Orig: the Zombot drops them mid-roof (BossSpawnContact sets
        // mPosX = 600). Dropping past the boss at the right edge instead
        // made them walk ~20 s behind the huge invulnerable body that
        // soaks every lobbed shot -- undefendable.
        z->x_location = 600 + rand(0, 40);
        z->y_location = cells[z->row][0].y1 - 50;
        game_characters.zombies.push_back(z);
        break;
    }
    f.boss->set_phase(BossZombie::PH_STOMP);
    f.phase_ticks = BOSS_ACTION_TICKS;
    play_sound_effect(ZOMBIE_FALLING_MUSIC_DIRECTORY);
}

// Fire/ice boulder down a random lane. In the original this is always the
// head-attack spit, mid vulnerable window (BossHeadSpitContact); it is NOT a
// free-standing attack, so no separate phase/timer here.
void roll_boulder()
{
    BoulderZombie *b = new BoulderZombie(rand(0, 1) == 0); // fire or ice
    b->init();
    b->type = ZOMBOSS_TYPE; // generic slot; nothing type-special applies
    b->wave_id = -1;
    // Never down the Zombot's own lane: the ball would roll out of the
    // lowered head (visually wrong), and burning it there would run into the
    // jalapeno whole-row burn that lives outside this folder and ignores the
    // boss's invulnerable state (instant kill).
    int row_pick = rand(0, (int)level.playable_rows.size() - 2);
    b->row = level.playable_rows[row_pick >= (int)level.playable_rows.size() / 2
                                      ? row_pick + 1
                                      : row_pick];
    b->x_location = WINDOW_WIDTH - 160;
    b->y_location = cells[b->row][0].y1 - 50;
    b->is_moving = true;
    b->attacked_time = b->cold_time = 0;
    b->bite_time = BITE_CLK_COUNT;
    b->next_step_time = 0;
    game_characters.zombies.push_back(b);
    play_sound_effect(AWOOGA_MUSIC_DIRECTORY);
    play_sound_effect(BOSS_BOULDER_MUSIC_DIRECTORY);
}

void throw_rv()
{
    // Target a 2(rows)x3(cols) patch of the lawn (orig: target col 0-2, so
    // the RV lands on the left half where the pults are parked).
    f.rv_col = rand(0, 2);
    f.rv_row = level.playable_rows[rand(0, (int)level.playable_rows.size() - 2)];
    f.rv_warn = RV_WARN_TICKS;
    f.boss->set_phase(BossZombie::PH_RV);
    f.phase_ticks = BOSS_ACTION_TICKS;
    play_sound_effect(RVTHROW_MUSIC_DIRECTORY);
}

void rv_impact()
{
    for (int i = 0; i < (int)game_characters.plants.size();)
    {
        Plants *p = game_characters.plants[i];
        if (p->get_row() >= f.rv_row && p->get_row() <= f.rv_row + 1 &&
            p->get_col() >= f.rv_col && p->get_col() <= f.rv_col + 2)
        {
            delete_plant(game_characters.plants, i); // decrements i itself
            i++;
        }
        else
        {
            i++;
        }
    }
    play_sound_effect(BOSS_EXPLOSION_MUSIC_DIRECTORY); // thud + debris
    // Orig BossRVLanding (Zombie.cpp:9766-9791): every RV impact re-arms the
    // pattern clocks -- summon 500cs, head 5000cs, stomp 4000cs from damage
    // stage 1, bungee 6500cs from stage 2.
    f.spawn_clk = 300;
    f.head_clk = 3000;
    if (damage_stage() >= 1)
        f.stomp_clk = STOMP_ARM_TICKS;
    if (damage_stage() >= 2)
        f.bungee_clk = BUNGEE_ARM_LATE;
}

/*=== Foot stomp (repvz BossStompAttack/BossCanStompRow/BossStompContact,
Zombie.cpp:9980-10043) ========================================================
Every 55-65 s the Zombot slams its foot on a 2-row band that holds at least
one plant in a column >= 5 (the right half, where the attacking pults park),
crushing EVERY plant of those two rows in columns 5-8. The cooldown re-arms
even when the attack fizzles for lack of targets; the counter itself only
starts once damage stage 2 has armed it (see update()).*/

// Any plant in rows [first,second] at a column >= 5 (orig BossCanStompRow)?
static bool stompable_pair(int pair_idx)
{
    int r0 = level.playable_rows[pair_idx];
    int r1 = level.playable_rows[pair_idx + 1];
    for (auto *p : game_characters.plants)
        if ((p->get_row() == r0 || p->get_row() == r1) && p->get_col() >= 5)
            return true;
    return false;
}

void stomp_attack()
{
    f.stomp_clk = STOMP_CLK_MIN + rand(0, STOMP_CLK_MAX - STOMP_CLK_MIN); // 55-65 s
    vector<int> pairs;
    for (int i = 0; i + 1 < (int)level.playable_rows.size(); i++)
        if (stompable_pair(i))
            pairs.push_back(i);
    if (pairs.empty())
        return; // nothing in the stomp zone: fizzle, cooldown already spent
    f.stomp_pair = pairs[rand(0, (int)pairs.size() - 1)];
    f.stomp_crushed = false;
    f.boss->stomp_row_pair = f.stomp_pair;
    f.boss->set_phase(BossZombie::PH_FOOT_STOMP);
    f.phase_ticks = BOSS_ACTION_TICKS;
    play_sound_effect(HYDRAULIC_SHORT_MUSIC_DIRECTORY);
}

// The thud: crush every plant of the two target rows in columns 5-8
// (orig BossStompContact iterates plants and Squish()es those in range).
void stomp_contact()
{
    int r0 = level.playable_rows[f.stomp_pair];
    int r1 = level.playable_rows[f.stomp_pair + 1];
    for (int r = r0; r <= r1; r++)
        for (int c = 5; c < HORIZ_BLOCK_COUNT; c++)
            crush_plants_at(r, c);
    play_sound_effect(GARGANTUAR_THUMP_MUSIC_DIRECTORY);
}

/*=== Bungee raid (repvz BossBungeeAttack/BossBungeeSpawn,
Zombie.cpp:10046-10070) =======================================================
The Zombot sends NUM_BOSS_BUNGEES=3 bungees diving for three ADJACENT columns
starting at col rand(0,2) (orig mTargetCol+i); each rope yanks the plant it
lands on (the clone bungee's forced-steal mode handles the steal and the
empty-tile bounce-off).*/

void bungee_attack()
{
    f.bungee_clk = BUNGEE_CLK_MIN + rand(0, BUNGEE_CLK_MAX - BUNGEE_CLK_MIN); // 40-50 s
    f.boss->set_phase(BossZombie::PH_BUNGEE);
    f.phase_ticks = BOSS_ACTION_TICKS;
    f.bungees_dropped = false;
    f.bungee_col0 = rand(0, 2); // orig mTargetCol = RandRangeInt(0, 2)
    play_sound_effect(HYDRAULIC_SHORT_MUSIC_DIRECTORY);
    play_sound_effect(BUNGEE_SCREAM_SND_DIRECTORY);
}

void bungee_drop()
{
    for (int i = 0; i < NUM_BOSS_BUNGEES; i++)
    {
        int col = f.bungee_col0 + i;
        // Orig PickBungeeZombieTarget(col): cells WITH a plant outrank empty
        // ones 10000:1 inside the fixed column; an all-empty column still gets
        // a bungee (it hovers and leaves empty-handed).
        vector<int> planted_rows;
        for (int r : level.playable_rows)
            if (cells[r][col].is_planted)
                planted_rows.push_back(r);
        int row = planted_rows.empty()
                      ? level.playable_rows[rand(0, (int)level.playable_rows.size() - 1)]
                      : planted_rows[rand(0, (int)planted_rows.size() - 1)];
        BungeeZombie *b = static_cast<BungeeZombie *>(init_zombie(BUNGEE_TYPE, level.level_num));
        b->force_target(row, col, -1); // forced steal of that tile
        b->wave_id = -1;
        game_characters.zombies.push_back(b);
    }
}

} // namespace

// RV landing-zone warning patch -- render-path hook (see boss_fight.hpp):
// display_game_layout calls this topmost; update() only advances rv_warn.
void draw_rv_warning()
{
    if (f.rv_warn < 0)
        return;
    // Translucent target patch (approximation of the RV shadow).
    int x = cells[f.rv_row][f.rv_col].x1;
    int y = cells[f.rv_row][f.rv_col].y1;
    int w = cells[f.rv_row][f.rv_col + 2].x2 - x;
    int h = cells[f.rv_row + 1][f.rv_col].y2 - y;
    win.fill_rect(x, y, w, h, RGB(60, 60, 60));
    win.draw_rect(x, y, w, h, RGB(220, 40, 40));
}

bool active()
{
    return level.is_special(LevelSpecial::Boss);
}

void on_boss_killed()
{
    f.boss_killed = true;
}

void update()
{
    if (!active())
        return;
    if (f.level_num != level.level_num)
    {
        f = Fight();
        f.level_num = level.level_num;
    }

    // Lazy boss spawn: the Zombot enters at the right edge; the regular wave
    // system is shut off for this level (win condition = boss death).
    if (!f.spawned)
    {
        BossZombie *boss = static_cast<BossZombie *>(init_zombie(ZOMBOSS_TYPE, level.level_num));
        boss->row = level.playable_rows[(int)level.playable_rows.size() / 2];
        // Anchored by the art's feet into the 166x144 box (calibrated against
        // the rendered bbox: scale 0.6 puts the Zombot just right of the
        // grid, head under the ceiling, feet on the tiles).
        boss->x_location = BOSS_STAND_X;
        boss->y_location = 434;
        boss->wave_id = -1;
        game_characters.zombies.push_back(boss);
        f.boss = boss;
        f.spawned = true;
        level.waves_finished = true; // no regular waves: the boss is the level
        play_sound_effect(HYDRAULIC_MUSIC_DIRECTORY);
        return;
    }

    // Victory: the boss died -> stomp out the rest of the horde, then the
    // regular win path (waves finished + lawn cleared) takes over.
    if (f.boss_killed)
    {
        if (!f.wiped)
        {
            f.wiped = true;
            for (int i = 0; i < (int)game_characters.zombies.size();)
            {
                Zombie *z = game_characters.zombies[i];
                z->add_zombie_die(game_characters.zombie_parts);
                delete z;
                game_characters.zombies.erase(game_characters.zombies.begin() + i);
            }
        }
        return;
    }
    if (!f.boss)
        return;

    f.age++;
    // Attack counters (orig UpdateBoss ticks them every frame the boss is not
    // ice-trapped; the clone has no board-level ice clock, so plain ticks).
    if (f.stomp_clk > 0)
        f.stomp_clk--;
    if (f.bungee_clk > 0)
        f.bungee_clk--;

    // The clone's roof cleaner (5-10 keeps mowers, like the original) deletes
    // ANY zombie it rolls over. The Zombot's standing body registers as
    // underground, so it is immune -- but its lowered head is not. Rather
    // than let one mower trip silently skip the whole fight, the Zombot
    // refuses to duck its head into a rolling cleaner on its lane.
    bool mower_on_boss_row = false;
    for (auto *m : game_characters.mowers)
        if (m && m->status == MOWER_ACTIVE && m->row == f.boss->row)
            mower_on_boss_row = true;
    if (mower_on_boss_row &&
        (f.boss->get_phase() == BossZombie::PH_HEAD_ENTER ||
         f.boss->get_phase() == BossZombie::PH_HEAD_DOWN))
    {
        // Snap the head back up and out of the harm's way; the attack clock
        // re-arms on its usual cycle.
        f.boss->set_vulnerable(false);
        f.boss->set_phase(BossZombie::PH_HEAD_LEAVE);
        f.phase_ticks = HEAD_LEAVE_TICKS;
        f.ball_left = -1;
    }

    /*Boulder sweeps (the ball itself takes no damage; is_underground keeps
    every bullet / burn / explosion path off it):
      * Ice-shroom vs FIRE boulder (orig IceZombies destroys the fireball
        instantly, anywhere on the lawn -- Plant.cpp:4282-4286). The clone's
        ice-shroom only freezes + chips 1 HP, so a frozen FIRE boulder
        shatters here.
      * Ball vs roof cleaner (orig UpdateBossFireball squishes the mower it
        rolls onto -- Zombie.cpp:10291-10299). The clone has no
        MOWER_SQUISHED state, so the cleaner is simply destroyed (this is the
        ball<->mower hook the mower folder left for the boss folder).
    */
    for (int i = 0; i < (int)game_characters.zombies.size();)
    {
        auto *b = dynamic_cast<BoulderZombie *>(game_characters.zombies[i]);
        if (!b)
        {
            i++;
            continue;
        }
        if (!b->is_ice() && b->freeze_time > 0)
        {
            b->add_zombie_die(game_characters.zombie_parts);
            delete b;
            game_characters.zombies.erase(game_characters.zombies.begin() + i);
            play_sound_effect(BOSS_EXPLOSION_MUSIC_DIRECTORY);
            continue;
        }
        for (int m = 0; m < (int)game_characters.mowers.size(); m++)
        {
            Mower *mw = game_characters.mowers[m];
            if (mw && mw->row == b->row && mw->x > b->x_location - 20 &&
                mw->x <= b->x_location + 70)
            {
                delete mw;
                game_characters.mowers[m] = nullptr;
                play_sound_effect(GARGANTUAR_THUMP_MUSIC_DIRECTORY);
            }
        }
        i++;
    }

    // --- phase machine: one action at a time, like the original -----------
    switch (f.boss->get_phase())
    {
    case BossZombie::PH_HEAD_ENTER:
        // Second half of the lowering anim already counts as hittable in the
        // original (EffectedByDamage: enter blocked only while anim < 0.5).
        if (f.phase_ticks <= HEAD_ENTER_TICKS / 2)
            f.boss->set_vulnerable(true);
        // Lunge in sync with the hydraulics.
        f.boss->x_location = BOSS_STAND_X +
                             (int)((HEAD_X_DOWN - BOSS_STAND_X) *
                                   (float)(HEAD_ENTER_TICKS - f.phase_ticks) /
                                   HEAD_ENTER_TICKS);
        if (--f.phase_ticks <= 0)
        {
            f.boss->set_phase(BossZombie::PH_HEAD_DOWN);
            f.phase_ticks = HEAD_DOWN_TICKS;
            f.ball_left = HEAD_DOWN_TICKS - BALL_AT_DOWN_LEFT;
        }
        break;
    case BossZombie::PH_HEAD_DOWN:
        f.boss->x_location = HEAD_X_DOWN;
        if (--f.ball_left == 0)
            roll_boulder();
        if (--f.phase_ticks <= 0)
        {
            f.boss->set_phase(BossZombie::PH_HEAD_LEAVE);
            f.phase_ticks = HEAD_LEAVE_TICKS;
        }
        break;
    case BossZombie::PH_HEAD_LEAVE:
        // Orig: leave anim blocks damage once past its halfway point.
        if (f.phase_ticks <= HEAD_LEAVE_TICKS / 2)
            f.boss->set_vulnerable(false);
        // Rise back to the standing anchor.
        f.boss->x_location = HEAD_X_DOWN +
                             (int)((BOSS_STAND_X - HEAD_X_DOWN) *
                                   (float)(HEAD_LEAVE_TICKS - f.phase_ticks) /
                                   HEAD_LEAVE_TICKS);
        if (--f.phase_ticks <= 0)
        {
            f.boss->set_vulnerable(false);
            f.boss->x_location = BOSS_STAND_X;
            f.boss->set_phase(BossZombie::PH_IDLE);
        }
        break;
    case BossZombie::PH_IDLE:
        break; // pattern timers below pick the next action
    case BossZombie::PH_FOOT_STOMP:
        // Thud at mid-phase (orig contact event ~0.5, Zombie.cpp:10468-10483).
        if (!f.stomp_crushed && f.phase_ticks <= BOSS_ACTION_TICKS - STOMP_CRUSH_AT)
        {
            stomp_contact();
            f.stomp_crushed = true;
        }
        if (--f.phase_ticks <= 0)
            f.boss->set_phase(BossZombie::PH_IDLE);
        break;
    case BossZombie::PH_BUNGEE:
        // The three ropes drop at anim time 0.4 of the enter (orig
        // BossBungeeSpawn, Zombie.cpp:10485-10491).
        if (!f.bungees_dropped && f.phase_ticks <= BUNGEES_DROP_AT)
        {
            bungee_drop();
            f.bungees_dropped = true;
        }
        if (--f.phase_ticks <= 0)
            f.boss->set_phase(BossZombie::PH_IDLE);
        break;
    default:   // PH_STOMP / PH_BALL / PH_RV one-shot phases
        if (--f.phase_ticks <= 0)
            f.boss->set_phase(BossZombie::PH_IDLE);
        break;
    }

    // RV warning -> impact (runs independently of the phase; the warning
    // patch itself is drawn by the render path -- draw_rv_warning, called
    // from display_game_layout -- since the auto-FPS loop redraws the whole
    // scene after the ticks and would wipe anything drawn in here).
    if (f.rv_warn >= 0)
    {
        if (--f.rv_warn < 0)
            rv_impact();
    }

    // Pattern timers. Each waits for the boss to go idle before firing, and
    // retries shortly otherwise (the original also runs one attack at a
    // time and never interrupts the head-down window). Original priority in
    // PHASE_BOSS_IDLE (Zombie.cpp:10403-10455): damage-stage transition >
    // stomp > bungee/RV > head attack > summon.
    // Damage-stage transitions (Zombie.cpp:10414-10426): crossing hp < 4/5
    // sends the bungees AT ONCE; crossing hp < 1/2 makes the first RV throw,
    // which arms the stomp and late bungee clocks (BossRVLanding :9897-9906).
    // A crossing mid-action is banked until the boss next goes idle.
    if (is_idle())
    {
        int st = damage_stage();
        if (st != f.stage)
        {
            if (st == 1)
                bungee_attack();
            else if (st == 2)
            {
                throw_rv();
                f.stomp_clk = STOMP_ARM_TICKS;     // orig 4000cs at landing
                f.bungee_clk = BUNGEE_ARM_LATE;    // orig 6500cs
            }
            f.stage = st;
        }
    }
    if (f.stomp_clk == 0 && is_idle())
        stomp_attack(); // re-arms the clock even when nothing is stompable
    if (f.bungee_clk == 0 && is_idle())
    {
        // Orig (Zombie.cpp:10300-10311): when the bungee clock expires,
        // 1/4 (adventure) rolls an RV throw instead of the raid; the roll
        // re-arms this clock (RandRangeInt(4000,5000)cs), and the RV impact
        // re-arms it again from damage stage 2 (rv_impact ->
        // BossRVLanding 6500cs).
        if (rand(0, 3) == 0)
        {
            f.bungee_clk = BUNGEE_CLK_MIN + rand(0, BUNGEE_CLK_MAX - BUNGEE_CLK_MIN);
            throw_rv();
        }
        else
            bungee_attack();
    }
    if (--f.spawn_clk <= 0)
    {
        if (is_idle())
        {
            // Orig BossSpawnAttack (Zombie.cpp:9797-9808) re-arms per
            // damage stage: RandRangeInt(450,550) / (350,450) / (150,250) cs
            // = 4.5-5.5 / 3.5-4.5 / 1.5-2.5 s. The stage gates are the
            // boss's 4/5 and 1/2 hp marks (damage_stage()).
            int st = damage_stage();
            int lo = st == 0 ? SPAWN_CLK_MIN : st == 1 ? SPAWN_CLK_MID_MIN : SPAWN_CLK_LATE_MIN;
            int hi = st == 0 ? SPAWN_CLK_MAX : st == 1 ? SPAWN_CLK_MID_MAX : SPAWN_CLK_LATE_MAX;
            f.spawn_clk = lo + rand(0, hi - lo);
            drop_zombies();
        }
        else
            f.spawn_clk = 45;
    }
    if (--f.head_clk <= 0)
    {
        if (is_idle() && !mower_on_boss_row)
        {
            f.head_clk = HEAD_CLK + rand(0, 600); // orig 4000-5000 cs
            f.boss->set_phase(BossZombie::PH_HEAD_ENTER);
            f.phase_ticks = HEAD_ENTER_TICKS;
            play_sound_effect(HYDRAULIC_SHORT_MUSIC_DIRECTORY);
        }
        else
            f.head_clk = 45;
    }

    /*Jalapeno burn vs ICE boulder (orig Plant::BurnRow destroys the ICE ball
    when the ball rolls on the burned row -- Plant.cpp:4313-4318; the FIRE ball
    is untouched by fire, its counter is the ice-shroom above). burn_row is
    pure damage here and the ball takes no damage, so the counter is applied
    by the controller, fed by jalapeno.cpp's direct notification
    (bossfight::on_jalapeno_burn, queued the tick the fuse fires).*/
    for (int r : f.jalapeno_burn_rows)
    {
        // Row r just burned: every ICE boulder on that row is destroyed.
        for (int i = 0; i < (int)game_characters.zombies.size();)
        {
            auto *b = dynamic_cast<BoulderZombie *>(game_characters.zombies[i]);
            if (b && b->is_ice() && b->row == r)
            {
                b->add_zombie_die(game_characters.zombie_parts);
                delete b;
                game_characters.zombies.erase(game_characters.zombies.begin() + i);
                play_sound_effect(BOSS_EXPLOSION_MUSIC_DIRECTORY);
            }
            else
                i++;
        }
    }
    f.jalapeno_burn_rows.clear();
}

/*Jalapeno hook: the pepper signals each row it burns when its fuse fires.
The rows queue for the next controller tick (fight code owns the boulders;
on non-boss levels there is nothing to counter, so the burn is dropped).*/
void on_jalapeno_burn(int row)
{
    if (!active() || f.boss_killed)
        return;
    f.jalapeno_burn_rows.push_back(row);
}

bool boss_health(int &hp, int &max_hp)
{
    // Reports the body whenever a BossZombie is on the zombies list -- even
    // at 0 HP (dead but not yet reaped reads as "fight over" = full meter;
    // "no body at all" is what signals pre-spawn or post-wipe).
    for (auto *z : game_characters.zombies)
        if (auto *b = dynamic_cast<BossZombie *>(z))
        {
            hp = b->get_health();
            max_hp = b->max_health();
            return true;
        }
    return false;
}

int debug_boss_phase()
{
    return f.boss ? (int)f.boss->get_phase() : -1;
}
int debug_stomp_clk()
{
    return f.stomp_clk;
}
int debug_bungee_clk()
{
    return f.bungee_clk;
}
void debug_set_attack_clk(int stomp, int bungee)
{
    f.stomp_clk = stomp;
    f.bungee_clk = bungee;
}

} // namespace bossfight
