#pragma once
#include <vector>
using namespace std;

/*repvz speeds (mVelX in Zombie.cpp) are px per centisecond tick: the
original runs 100 cs/s, this game 60 ticks/s, so px/game-tick = cs * 5/3.
Use CS_VEL(x) to port a "0.xx px/cs" figure from the source verbatim.*/
constexpr float CS_VEL(const float px_per_cs)
{
    return px_per_cs * (100.0f / 60.0f);
}

/*Retail pace standard: every movement constant is the original CS_VEL
figure verbatim (px per centisecond converted to this game's 60 tps),
so a plain walker rolls 0.23-0.37 px/cs like the original's
PickRandomSpeed default branch (PvZ-Portable Zombie.cpp:1213-1224) --
23-37 px/s. The earlier NMH "HEAD pacing" build halved every pace with
a 6/11 scale (zombies stepped 1 px per 4 ticks); that made levels much
more lenient than retail, so the scale is back at 1.0. All reanim rate
pins (reanim_zombie.cpp head_pace_fps) read this same constant, so
walk cycles stay foot-synced with the movement speed. Re-tune in one
line if a slower pacing is ever wanted again.*/
constexpr float HEAD_SPEED_SCALE = 1.0f;

/*Sheet-walk ground stride per frame-index (retail pace): the original plays
its walk off the reanim "_ground" track so one stride covers a fixed distance
(Zombie::UpdateAnimSpeed, Zombie.cpp:6685-6694), and its legacy sprite path
rolls mAnimTicksPerFrame 12/15 by speed (PickRandomSpeed, Zombie.cpp:1218-
1222). The 46-pose sheet walk cycle spans the same ground as the reanim cycle
it mirrors: 55.8 doc-px of _ground travel (Zombie.reanim anim_walk 31..170)
over 46 poses x ZOMBIE_FRAME 3 = 138 frame-indices -> 0.48 px per index, tuned
at the old 1.186 zoom. STALE-BUT-KEPT: the reanim path now renders at retail
scale 1.0 (ZOMBIE_SCALE deleted), where the same 55.8 doc-px of travel
implies 0.404 px per index -- 0.48 is kept so the sheet pacing stays at its
tuned pre-rework feel; a retail-true stride is a pure re-tune of this value.
`frame` advances on ground covered, so each zombie's step rate scales with
its rolled speed and the feet stay planted; at the 0.30 px/cs mean (0.5
px/tick) a full cycle takes ~2.2 s and holds 2 footfalls (~33 px apart), the
retail gait. The old 3.6 px per index (a 497 px cycle, ~7.5x too long for
the ground actually covered) glided the same ~9x desync the fixed-rate
reanim feed had.*/
constexpr float ZOMBIE_WALK_STRIDE_PX = 0.48f;

/*Chilled movement factor (original Zombie.cpp:70 CHILLED_SPEED_FACTOR =
0.4f): while cold, a zombie moves at 40% of its pace. The eat rate is
halved separately (BITE_CLK_COUNT doubling = the original's even-tick
chew gating).*/
constexpr float CHILLED_SPEED_FACTOR = 0.4f;
/*Retail walk pace factor for zombies moved by their _ground track: the anim
rate formula carries 47 * SECONDS_PER_UPDATE = 0.47 (Zombie.cpp:6694), so the
covered distance averages 0.47 * mVelX.*/
constexpr float GROUND_WALK_PACE = 0.47f;

/*Float range roll (repvz LawnCommon RandRangeFloat). Zombie speed variance
uses tiny bands, so integer rand(L, R) is not enough.*/
float rand_float(const float &lo, const float &hi);

/*Ticks (60 tps) of the one-shot purple swirl burst after the hypno-shroom
bite (repvz PARTICLE_MIND_CONTROL: 40 particle cs = 24 ticks).*/
constexpr int CHARM_FLASH_TICKS = 24;

class ZombiePart
{
private:
    int row;
    int x_location;
    int img_num;
    int frame = 0;
    int frame_clk;
    bool is_cold;
    int width, height;
    // reanim skin for DEATH_PART_IMG / DROP_*_IMG parts (0 = plain zombie)
    int skin;
    // died while swimming (pool lane / snorkel underwater): the reanim death
    // then plays anim_waterdeath with the short linger (repvz mZombieFade=10)
    bool in_water = false;
    // died after its head was already torn off (D1): the reanim death segment
    // already hides the head group, and the usual tossed-head pop is skipped.
    bool headless = false;

public:
    ZombiePart(const int &_img_num, const int &_frame_clk,
               const int &_r, const int &_x,
               const int &_w, const int &_h, const bool &_cold = false,
               const int &_skin = 0, const bool &_in_water = false,
               const bool &_headless = false);
    // debug/harness introspection: the part's img slot (DEATH_PART_IMG /
    // DROP_*_IMG / CHARRED_PART_IMG / sheet directory) and its payload (reanim
    // skin for deaths/drops, CHARRED_* kind for charred corpses)
    int img() const { return img_num; }
    int payload() const { return skin; }
    bool in_pool() const { return in_water; }
    bool is_headless_corpse() const { return headless; }
    bool display(const int &_row);
    // reanim death corpse path (used when img_num == REANIM_DEATH_IMG)
    bool display_reanim_death(const int &_row);
    // reanim tossed-part path (arm/armor/flag pops; img_num = DROP_*_IMG)
    bool display_reanim_drop(const int &_row);
    // reanim charred-corpse path (img_num == CHARRED_PART_IMG; `skin` slot
    // carries the CHARRED_* kind)
    bool display_reanim_charred(const int &_row);
};
/*
@param row(int): The row of the zombie
@param x_location(int): The begin column of zombie
@param health(int): remaining health of zombie.
@param is_moving(bool): true if zombie is moving, otherwise false.
@param directory_num: order of string that is the path to image.
@param blinkdirectory_num: order of string that is the path to blink image.
@param n_sheet: number of sprites.
@param c_sheet: number of column in sprite sheet
@param directory(string): decide what image to be render.
*/
class Zombie
{
protected:
    /*Pool health on the 20:1 effective scale (1 point = 1 pea = 20 original
    damage); float because exact ports land on halves (270 -> 13.5).*/
    float health;
    /*Big Trouble Little Zombie (3-5): the original divides EVERY health pool
    of a mini zombie by 4 (repvz ZombieInitialize, Zombie.cpp:862-867), so all
    HP-derived thresholds (degrade tiers, armor drop, paper break, arm pops,
    carried-shield caps) follow this factor: effective threshold = base table
    value * health_scale. Damage itself stays absolute (1 pea = 1 point), so
    instant kills (90) and the mower trample need no adjustment.*/
    float health_scale = 1.0f;
    /*Bookkeeping for carried-shield zombies (door/newspaper/ladder): how
    much damage the shield tier has soaked from straight shots (take_hit /
    decrease_health). The pooled `health` alone cannot tell shield damage
    from body damage (health = shield_hp + body_hp is underdetermined), so
    lobbed shots (take_body_hit) read this to target the body exactly.
    Ignored by shield-less types.*/
    float shield_damage = 0.0f;
    int directory_num;
    int frame;
    int dir_width, dir_height;
    int nZom;
    int armor_dir = -1;
    virtual void render_zombie();
    // reanim render path; returns true when it handled the drawing
    bool render_reanim(const int &_minus_x);

public:
    int type;
    int wave_id = -1; // wave (0-based) this zombie was spawned for; -1: not from a wave
    int row, x_location, y_location;
    bool is_moving;
    int attacked_time, cold_time;
    int bite_time;
    /*Legacy step-clock countdown: movement is driven by step_speed/walk_carry
    now, so this stays 0 in practice. Kept because collision/boss code outside
    this directory still touches it (snow-pea snowball halves the pending step,
    boss spawn clears it).*/
    int next_step_time;
    /*Walking speed of this individual in px per game tick (60 tps), rolled at
    spawn with per-zombie variance (retail band x HEAD_SPEED_SCALE, so the band
    mean is CS_VEL(0.30) = the retail 30 px/s). Subclasses with
    state-dependent paces select between this and their own constants in
    speed_px_per_tick().*/
    float step_speed = CS_VEL(0.23f) * HEAD_SPEED_SCALE;
    /*Fractional-pixel carry: speed_px_per_tick() accumulates here every tick
    and whole pixels step the int x_location when the carry reaches 1 px
    (port of the original's continuous mPosX -= mVelX onto a pixel grid).*/
    float walk_carry = 0.0f;
    /*Walk-anim stride accumulator (ZOMBIE_WALK_STRIDE_PX): ground covered
    toward the next sheet frame-index -- the walk anim advances on DISTANCE,
    not time, so slow/rolled/chilled zombies step proportionally slower and the
    feet stay synced with x_location (retail _ground-track behavior).*/
    float walk_stride_acc = 0.0f;
    /*D2 diagnostics: the _ground delta fed to movement this tick (board px,
    0 when the state is not ground-driven) and the whole-pixel step actually
    applied. The autoplay walk check logs their running totals -- they must
    match, proving the feet track the applied dx exactly.*/
    float last_ground_dx = 0.0f;
    int last_step_px = 0;
    int degrade_steps = 0;       // armor/degrade tiers already fired (threshold catch-up)
    bool is_charmed = false; // hypnotized: walks right, fights other zombies
    /*D1 damage-state bookkeeping (repvz Zombie::UpdateDamageStates /
    DropHead / DropArm, Zombie.cpp:3511-3934): the arm and head each pop at
    most once, at 2/3 and 1/3 of the true BODY pool. A headless zombie keeps
    walking but bleeds out (mHasHead == false => isDying, Zombie.cpp:4661-
    4688) and can no longer eat. bleed_pending skips the bleed tick on which
    the head itself dropped (the "~0.7 s after the 10th pea" cadence).*/
    bool arm_dropped = false;
    bool head_dropped = false;
    bool bleed_pending = false;
    // ticks left of the one-shot purple swirl burst after the charm bite
    // (armed with CHARM_FLASH_TICKS by the trance hook)
    int charm_flash = 0;
    int freeze_time = 0;     // ice-shroom hold; when it ends the chill carries on
    int butter_time = 0;     // kernel-pult butter: glued in place, face pasted
    float reanim_time = 0.0f; // local time (reanim frames) in current segment
    float reanim_rate = 0.2f; // reanim frames per game tick (12 fps at 60 tps);
    // zombie flesh-rate pins live in reanim_zombie.cpp kPlayRateRules
    // (HEAD zombie_frame=3: 20 fps warm, 10 chilled)
    /*F2 layout-editor transient offsets (px): render_reanim/render_zombie add
    them to the drawn anchor, and the rsdl debug-scale registry scales the art
    about its bottom-center (feet). Render-only, default 0 -> gameplay
    untouched; the editor zeroes them on close (never persisted).*/
    int layout_dx = 0, layout_dy = 0;

    Zombie();
    Zombie(int _type, int level_num);
    virtual ~Zombie();
    /*Init a zombie*/
    virtual void init(){};
    /*Make zombie for credit*/
    virtual void make_credit(){};
    /*Make zombie's armor drop*/
    virtual void armor_drop(vector<ZombiePart> &zombie_parts){};

    /*Per-tick movement-phase hook (runs inside move_zombies before the
    regular step logic). Returns true when it fully handled this tick's
    movement (pole vault flight, newspaper gasp pause).*/
    virtual bool on_move_tick() { return false; }
    /*Hook fired when the zombie reaches a plant tile; return true to
    suppress the default "stop and eat" handling (pole vault over it).*/
    virtual bool on_reach_plant(const int &col) { return false; }
    /*Bite gate: vaulting zombies fly over the plant, gasping newspaper
    zombies hold still -- neither chews mid-transition.*/
    virtual bool can_eat_plant() const { return true; }
    /*Roll this zombie's own walking speed (retail RandRangeFloat(0.23, 0.32)
    px/cs band x HEAD_SPEED_SCALE -> 0.21..0.29 px/tick around the 15 px/s HEAD
    mean). Called once per spawn by init_zombie (after init(), so subclass
    state is set). Fast classes override.*/
    virtual void pick_speed();
    /*px per game tick for the current state, BEFORE the chill halving applied
    in move_zombies. Subclasses override for phase paces (football run, mad
    newspaper, bobsled on-ice glide, ...); the whack special ramps the base
    feet-on-ground pace here (big_trouble minis walk the plain roll -- only
    their anim rate doubles, per repvz UpdateAnimSpeed's 1/mScaleZombie).*/
    virtual float speed_px_per_tick() const;

    /*Airborne (balloon before it pops): flies over plants and ground
    projectiles pass underneath. Cactus spikes / Blover wind call
    pop_balloon to force the landing.*/
    virtual bool is_flying() const { return false; }
    /*Balloon only: pop the balloon (cactus spike / blover). After the pop
    the zombie falls and walks like a plain one (over the pool it drowns).*/
    virtual void pop_balloon(vector<ZombiePart> &zombie_parts) {}
    /*Digger tunnel phase (and rising): underground, immune to bullets and
    mowers, passes under plants.*/
    virtual bool is_underground() const { return false; }
    /*Mid-vault, off the ground (repvz PHASE_POLEVAULTER_IN_VAULT /
    PHASE_DOLPHIN_IN_JUMP -- Plant::FindSquashTarget Plant.cpp:1442-1448 and
    EffectedByDamage's DAMAGAGES_OFF_GROUND gate, Zombie.cpp:8224-8234 refuse
    them): squash neither locks nor slams these.*/
    virtual bool is_vaulting() const { return false; }
    /*Mounted and sliding (repvz PHASE_DOLPHIN_RIDING /
    IsBobsledTeamWithSled -- same FindSquashTarget exclusions): squash
    refuses to lock a riding target.*/
    virtual bool is_riding_vehicle() const { return false; }
    /*repvz Zombie::mZombieRect (per-type ZombieInitialize cases; read by
    GetZombieRect, Zombie.cpp:8381-8398): body rect offsets from the sprite
    box's top-left corner. Clone anchor: x_location/y_location are the own
    sprite origins, same frame layout, so the offsets port verbatim (garg's
    mWidth=180 box shifts nothing -- the original's mX is the same origin).
    Virtual: bobsled (in/out of the sled), balloon (altitude) and imp
    (falling) change their rect with state (repvz d2647, d1424 + the
    GetDrawPos mBodyY altitude shift).*/
    virtual void zrect_off(int &x, int &y, int &w, int &h) const;
    int zrect_left() const;
    int zrect_right() const;
    int zrect_top() const;
    int zrect_bottom() const;
    /*repvz Zombie::mZombieAttackRect (per-type ZombieInitialize; consumed by
    FindPlantTarget -> GetZombieAttackRect, Zombie.cpp:6460-6475): the forward
    "mouth" box (left offset from x_location, width) that decides which tile
    the zombie stops to chew. Width 0 (bungee/boss) = never chews tiles.
    Virtual: pole vault and dolphin rider swap the box per phase
    (repvz d1797, d1857, d1927, and the in-vault branch d8404-8407).*/
    virtual void attack_x_span(int &left, int &width) const;
    /*Bungee zombies that finished their grab/drop fly back off-screen and
    ask to be erased (they never die by bullets at that point).*/
    virtual bool despawn_ready() const { return false; }
    /*Snorkel swimming under water: straight shots (peas, fume clouds) pass
    over it; it surfaces to eat a plant.*/
    virtual bool is_submerged() const { return false; }
    /*Zomboni flattens the plant it runs over instead of stopping to chew
    (the actual plant kill happens in ZombieVsPlant).*/
    virtual bool crushes_plants() const { return false; }

    int get_row() const;
    float get_health() const;
    /*D1 body-pool split: `health` is the combined pool (body + helm/shield).
    body_max_health() is the per-type 270/500/... original-HP body converted
    to clone units (x health_scale); body_health() subtracts whatever helm/
    shield tier is still soaking damage (min(health, body_max) for helms,
    health - remaining carried shield for door/newspaper/ladder). Arm/head
    thresholds and the headless bleed read these, exactly like the original's
    mBodyHealth/mBodyMaxHealth (Zombie.cpp:3908-3934, 4661-4688).*/
    float body_max_health() const;
    float body_health() const;
    /*repvz Update's isDying test (Zombie.cpp:4663-4670): a zombie whose head
    is gone, or a zomboni/catapult whose body is under 200 original HP
    (10 clone units). Those bleed out each tick until dead.*/
    bool is_dying_body() const;
    /*repvz Zombie::CanLoseBodyParts (Zombie.cpp:3490-3502).*/
    virtual bool can_lose_body_parts() const;
    /*Per-tick arm/head pop pass; called after determine_appearance in every
    damage path. Pushes the head/arm FX once (reanimz DROP_*_IMG).*/
    void update_damage_states(vector<ZombiePart> &zombie_parts);
    /*Headless dying: original HP lost per ORIGINAL tick (1, yeti 10 but
    overridden to 3 for mBodyMaxHealth >= 500, Zombie.cpp:4672-4687) mapped
    to clone HP per 60 Hz logic tick (x100/60 original ticks, /20 HP unit).*/
    float headless_bleed_per_tick() const;
    /*Simple health subtraction used by the bleed (no armor/appearance work:
    the bleeding body is already bare and headless).*/
    void bleed_health(float dmg);
    /*Per logic tick: hit-flash/chill clocks (run even frozen/buttered).
    Render frames no longer age zombie state (D2).*/
    void update_logic_timers();
    /*Per logic tick: advance the sheet pose + reanim clock. `walk_v_raw` is
    this tick's unchilled forward pace (0 when not walking); `cold` applies
    the retail chill (fallback movement x0.4, ground-driven anim rate x0.5
    per RefreshAnimRate, Zombie.cpp:6637-6644). When the state has a usable
    _ground track, returns the board-px ground delta caused by the advance
    and sets ground_driven (the caller moves BY it, yielding retail's
    0.47*velX); otherwise the caller moves by the chilled raw pace.*/
    float update_logic_animation(float walk_v_raw, bool cold, bool &ground_driven);
    /*Narrow write access to the pooled health for spawn-time team assembly:
    bobsled mates spawn with the bare body+sled tier instead of the leader's
    full helm stack (level_data.cpp materialize_wave_plans; original only the
    lead wears the sled armor, Zombie.cpp:553-560). Everything else keeps
    health pool-managed through decrease_health.*/
    void spawn_set_health(float hp) { health = hp; }
    /*Current sprite/reanim state slot (read-only peek at directory_num).*/
    int cur_dir() const { return directory_num; }
    /*HP-threshold scale of this individual zombie (0.25 for big_trouble
    minis, 1.0 otherwise): multiply absolute HP constants by this before
    comparing against the zombie's health pools.*/
    float hp_scale() const { return health_scale; }
    /*Ground-shadow draw scale: the original scales the shadow blob by
    mScaleZombie (repvz Zombie::DrawShadow, Zombie.cpp:9129-9135 +
    TodDrawImageCenterScaledF at Zombie.cpp:9304-9308; Little-Trouble levels
    set mScaleZombie = 0.5, Zombie.cpp:790-792). The clone's mini flag is the
    per-zombie health_scale (zombie_type.cpp), and the 0.5 matches the
    renderer's BIG_TROUBLE_SCALE_MUL (reanim/reanim_game_hooks.cpp).*/
    float shadow_scale() const { return health_scale < 1.0f ? 0.5f : 1.0f; }
    // true while the duck tube is on the state slot (water-lane wrap of a
    // normal/cone/bucket zombie); the armor tier lives in armor_dir, so cone
    // and bucket keep their helmets over the tube.
    bool wears_duckytube() const;
    void change_zombie_eating_status();
    virtual void determine_appearance(vector<ZombiePart> &zombie_parts);
    bool decrease_health(vector<ZombiePart> &zombie_parts);
    /*Carried shield (screen door / newspaper / ladder): straight shots clank
    off the shield tier first; lobbed shots arc over it and land on the body
    (original ShieldType != SHIELDTYPE_NONE vs Projectile DAMAGE_BYPASSES_SHIELD).
    Cone/bucket/football gear is a helm, not a shield: every shot soaks it.*/
    bool carries_shield() const;
    /*Highest body-health threshold of a carried-shield type (shield tiers sit
    above it): door reuses the normal-zombie cap, newspaper 13.5, ladder 25.
    -1 for shield-less types. Scaled by health_scale so a big_trouble mini
    keeps its paper/door up to its quarter-size body.*/
    float shield_body_cap() const;
    /*Multi-damage hit (whack-a-zombie hammer): appearance
    degrades once, returns true when the zombie dies.*/
    bool take_hit(const float &dmg, vector<ZombiePart> &zombie_parts);
    /*Body-only hit for lobbed shots: the carried shield keeps its health, the
    damage lands on the body. If the body dies, the shield falls with the
    corpse. Non-shield zombies (helms included) take a plain take_hit.*/
    bool take_body_hit(const float &dmg, vector<ZombiePart> &zombie_parts);
    /*Fume-shroom soak (repvz DAMAGE_HITS_SHIELD_AND_BODY, Plant.cpp:4548 +
    Zombie.cpp:8056-8062): the carried shield takes the full hit AND the body
    takes the full hit -- the shield chips away but soaks nothing, so door/
    newspaper/ladder zombies die on body health alone (dragger counters).
    Helms soak the hit as usual (plain take_hit).*/
    bool take_shield_and_body_hit(const float &dmg, vector<ZombiePart> &zombie_parts);
    /*Charred corpse kind for burn kills (cherry bomb / jalapeno /
    doom-shroom / potato mine / explode-o-nut); reanimz::CHARRED_* value.
    The digger overrides this for its axe-less art (repvz ApplyBurn's
    PHASE_DIGGER_WALKING_WITHOUT_AXE branch, Zombie.cpp:8865-8869).*/
    virtual int charred_kind() const;
    virtual void add_zombie_die(vector<ZombiePart> &zombie_parts);
    /*Crush kill (squash slam): the original kills with DOESNT_LEAVE_BODY, so
    the zombie vanishes on the spot -- no corpse/head part is pushed. Not
    virtual: the visual is type-independent (repvz Zombie.cpp:9039-9047).*/
    void add_zombie_crushed(vector<ZombiePart> &zombie_parts);
    void display(const int &_row);
    /*Grave-rise emergence render (repvz UpdateZombieRiseFromGrave +
    DrawReanim's clip, Zombie.cpp:3093-3103 / 5689-5694): draws the body
    `rise_clk`-driven px deeper into the ground (200->0 over
    GRAVE_RISE_EMERGE_TICKS ticks) and clips everything below the ground
    line (+ the original's min(phase,40)px overhang). The zombie is a pending
    level.grave_rises riser here, not yet in game_characters.zombies, so no
    overlays/ghosts apply -- and it does not move, exactly like the original's
    UpdateZombie early-out during PHASE_RISING_FROM_GRAVE (Zombie.cpp:4637).
    Called from render_elements.cpp's row pass, under the grave dirt FX.
    Sheet-art fallback crops the drawn frame to the same clip line.
    Ages reanim_time like Zombie::display (renderer-tick cadence).*/
    void display_grave_rise(const int &rise_clk);
    virtual void display_credited(const int &_minus_x);

    friend Zombie *init_zombie(int _type, int level_num);
    friend Zombie *init_zombie(int _type, int level_num, const int &x_bias);
    friend Zombie *init_zombie(int _type, int level_num, const int &x_bias, int wave_idx);
    // Spawn-site tube wrap (zombie_type.cpp): a normal/cone/bucket placed on a
    // water lane by grave rise / pool ambush / straight init swaps its walk
    // state to the duckytube slot.
    friend void wrap_in_duckytube(Zombie *z);
};

/*Sheet-sprite ground anchor (the fallback path when no reanim set is loaded):
reanim zombies anchor at the retail pivot (reanim_zombie.cpp: doc point
(30,120) lands at (x+45, y+132)), the plain walk's opaque feet landing at
~y+134.8; the 166x144 sheet frames carry their opaque feet at +141 and
the door's 196x197 frames at ~+135, so drawing bottom-anchored at
ground + (frame_height - feet_row) drops every sheet onto that same ground
line (the plain set lifts 5 px, the door sheets stay put).*/
constexpr int ZOMBIE_SHEET_GROUND_Y = 136;
constexpr int ZOMBIE_SHEET_FEET_Y = 141;
constexpr int ZOMBIE_DOOR_FEET_Y = 135;

extern const int ZOMBIE_FRAME;
extern const int ZOMBIE_EXACT_LOCATION;
extern const int ZOMBIE_WIDTH;
extern const int ZOMBIE_HEIGHT;

extern const int ZOMBIE_EATING_FRAME;
extern const int ZOMBIE_DIE_FRAME;
extern const int ZOMBIE_BURNT_FRAME;

/*Explosion/burn corpse for a just-killed zombie (cherry bomb, jalapeno,
doom-shroom, potato mine, explode-o-nut). Reanim charred art per zombie type
(repvz Zombie::ApplyBurn, Zombie.cpp:8756-8905) when the reanim set is loaded;
falls back to the legacy Zombie_burnt sheet otherwise.*/
void push_burned_corpse(vector<ZombiePart> &zombie_parts, const Zombie *z);

extern const int HEAD_ZOMBIE_FRAME;
extern const int HEAD_ZOMBIE_WIDTH;
extern const int HEAD_ZOMBIE_HEIGHT;