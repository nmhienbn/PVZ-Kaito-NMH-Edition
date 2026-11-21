#include "zombie_struct.hpp"
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Map/Map.hpp"
#include "elements/Level/Level.hpp"
#include "music/music.hpp"
#include "reanim/reanim_zombie.hpp"
#include <cmath>

extern int game_state;
extern Map cells;
extern Window win;
extern Level level;

/*Corresponding walking of zombie eat image*/
static map<int, int> degrade_of = {
    {ZOMBIE_WALK1_DIRECTORY, ARMLESS_ZOMBIE_WALK1_DIRECTORY},
    {ZOMBIE_WALK2_DIRECTORY, ARMLESS_ZOMBIE_WALK2_DIRECTORY},
    {ZOMBIE_EATING_DIRECTORY, ARMLESS_ZOMBIE_EATING_DIRECTORY},

    {CONE1_DIRECTORY, CONE2_DIRECTORY},
    {CONE2_DIRECTORY, CONE3_DIRECTORY},
    {CONE3_DIRECTORY, NULL_DIRECTORY},

    {BUCKET1_DIRECTORY, BUCKET2_DIRECTORY},
    {BUCKET2_DIRECTORY, BUCKET3_DIRECTORY},
    {BUCKET3_DIRECTORY, NULL_DIRECTORY},

    {FLAG_ZOMBIE_WALK1_1_DIRECTORY, FLAG_ZOMBIE_WALK1_2_DIRECTORY},
    {FLAG_ZOMBIE_WALK2_1_DIRECTORY, FLAG_ZOMBIE_WALK2_2_DIRECTORY},
    {FLAG_ZOMBIE_EATING_1_DIRECTORY, FLAG_ZOMBIE_EATING_2_DIRECTORY},
    {FLAG_ZOMBIE_WALK1_2_DIRECTORY, FLAG_ZOMBIE_WALK1_3_DIRECTORY},
    {FLAG_ZOMBIE_WALK2_2_DIRECTORY, FLAG_ZOMBIE_WALK2_3_DIRECTORY},
    {FLAG_ZOMBIE_EATING_2_DIRECTORY, FLAG_ZOMBIE_EATING_3_DIRECTORY},
    {FLAG_ZOMBIE_WALK1_3_DIRECTORY, ARMLESS_ZOMBIE_WALK1_DIRECTORY},
    {FLAG_ZOMBIE_WALK2_3_DIRECTORY, ARMLESS_ZOMBIE_WALK2_DIRECTORY},
    {FLAG_ZOMBIE_EATING_3_DIRECTORY, ARMLESS_ZOMBIE_EATING_DIRECTORY},

    {DOOR_ZOMBIE_WALK1_1_DIRECTORY, DOOR_ZOMBIE_WALK1_2_DIRECTORY},
    {DOOR_ZOMBIE_WALK2_1_DIRECTORY, DOOR_ZOMBIE_WALK2_2_DIRECTORY},
    {DOOR_ZOMBIE_EATING_1_DIRECTORY, DOOR_ZOMBIE_EATING_2_DIRECTORY},
    {DOOR_ZOMBIE_WALK1_2_DIRECTORY, DOOR_ZOMBIE_WALK1_3_DIRECTORY},
    {DOOR_ZOMBIE_WALK2_2_DIRECTORY, DOOR_ZOMBIE_WALK2_3_DIRECTORY},
    {DOOR_ZOMBIE_EATING_2_DIRECTORY, DOOR_ZOMBIE_EATING_3_DIRECTORY},
    {DOOR_ZOMBIE_WALK1_3_DIRECTORY, ZOMBIE_WALK1_DIRECTORY},
    {DOOR_ZOMBIE_WALK2_3_DIRECTORY, ZOMBIE_WALK2_DIRECTORY},
    {DOOR_ZOMBIE_EATING_3_DIRECTORY, ZOMBIE_EATING_DIRECTORY},

    // Duck tube (water-lane cone/bucket broken back to plain hp): the tube
    // stays on; degrade is a no-op on the state slot (arm pop still fires).
    {DUCKYTUBE_WALK_DIRECTORY, DUCKYTUBE_WALK_DIRECTORY},
    {DUCKYTUBE_EAT_DIRECTORY, DUCKYTUBE_EAT_DIRECTORY}

};

/*Zombie constructor.
 */
Zombie::Zombie()
{
}

/*Zombie constructor.
Random row and delay time appear.
Set init status, heath and image to render. (override)
Random first frame.
*/
Zombie::Zombie(int _type, int level_num)
{
    type = _type;
    // Random appear row among the level's playable lanes
    const vector<int> &rows = level.playable_rows;
    row = rows.empty() ? rand(0, 4) : rows[rand(0, (int)rows.size() - 1)];
    // Random delay time to appear
    x_location = WINDOW_WIDTH - ZOMBIE_EXACT_LOCATION + rand(10, 100);
    y_location = cells[row][0].y1 - 50;
    // Set init status
    is_moving = true;
    attacked_time = cold_time = 0;
    bite_time = BITE_CLK_COUNT;
    next_step_time = 0;
    // init zombie override
    init();
    // Random first frame.
    frame = rand(0, ZOMBIE_FRAME * all_img[directory_num].n_sheet - 1);
    reanim_time = (float)rand(0, 120);
}

Zombie::~Zombie()
{
}

/*Float range roll (repvz RandRangeFloat), built on the project's int rand.*/
float rand_float(const float &lo, const float &hi)
{
    return lo + (hi - lo) * (float)rand(0, 10000) / 10000.0f;
}

/*Roll the individual walking speed: the retail band RandRangeFloat(0.23,
0.37) px/cs (PickRandomSpeed's default "else" branch every plain zombie
takes -- PvZ-Portable Zombie.cpp:1213-1224; repvz Zombie.cpp:1167-1229)
scaled by HEAD_SPEED_SCALE, so the band runs 23..37 px/s around the
retail mean 30 px/s. State-dependent classes (football/mad newspaper/...)
live in the subclass overrides.
Frame pacing is pinned in reanim_zombie.cpp's kPlayRateRules off the flat
0.2 feed; head_pace_fps reads HEAD_SPEED_SCALE, so the walk cycle stays
coupled to the movement speed (retail's UpdateAnimSpeed rate~mVelX link).*/
void Zombie::pick_speed()
{
    step_speed = rand_float(CS_VEL(0.23f), CS_VEL(0.37f)) * HEAD_SPEED_SCALE;
    reanim_rate = 0.2f;
}

/*Base walking pace in px per game tick (before chilling).*/
float Zombie::speed_px_per_tick() const
{
    float v = step_speed;
    // Big Trouble Little Zombie (3-5): mini zombies cover ground at the SAME
    // absolute pace as the plain roll -- the original never bumps mVelX for
    // them; it only halves the reanim scale and ties the walk anim to
    // rate = mVelX * (frames/distance) * 47 / mScaleZombie, then moves by
    // GetTrackVelocity("_ground") * mScaleZombie (repvz Zombie.cpp:6733 and
    // 4123), so the scale cancels out. What doubles is the ANIMATION rate
    // (1/0.5 = 2x footsteps), not the walk. Bite rate is unchanged too
    // (TICKS_BETWEEN_EATS has no little-trouble branch, Zombie.cpp:6907).
    // Whack-a-Zombie (2-5) risers roll their own pace at SPAWN instead
    // (WhackAZombieSpawning: mVelX = RandRangeFloat(0.5, curve(wave)) px/cs,
    // set once by the burst loop -- elements_actions.cpp whack_update).
    return v;
}

int Zombie::get_row() const
{
    return row;
}

/*repvz per-type body rects from ZombieInitialize (Zombie.cpp): the offsets
hang on the sprite box origin, so they port verbatim onto
x_location/y_location. State-changed rects (bobsled crash d2647, falling imp
d1424, flying altitude via GetDrawPos) come from subclass overrides.
Zombie boxes also grow with the 166 framing here the way the original's
mX/mWidth do, so a "wide hull" type's box spans the same artwork.*/
void Zombie::zrect_off(int &x, int &y, int &w, int &h) const
{
    // default body box (repvz Zombie ctor, Zombie.cpp:145)
    x = 36, y = 0, w = 42, h = 115;
    switch (type)
    {
    case FOOTBALL_TYPE:   x = 50,  y = 0;   w = 57,  h = 115; break; // Zombie.cpp:282
    case GARGANTUAR_TYPE: x = -17, y = -38; w = 125, h = 154; break; // :362
    case ZOMBONI_TYPE:
    case CATAPULT_TYPE:   x = 0,   y = -13; w = 153, h = 140; break; // :405 / :422
    case SNORKEL_TYPE:    x = 12,  y = 0;   w = 62,  h = 115; break; // :428
    case BUNGEE_TYPE:     x = -20, y = 22;  w = 110, h = 94;  break; // :275
    case DIGGER_TYPE:     x = 50,  y = 0;   w = 28,  h = 115; break; // :296
    case BALLOON_TYPE:    x = 36,  y = 30;  w = 42,  h = 115; break; // :630
    default: break; // bobsled w/sled (-50,0,275,115 :496) & falling imp
                    // (36,17,42,115 :1424) are state overrides; zomboss keeps
                    // its own handling (boss lane code reads raw boxes).
    }
}

int Zombie::zrect_left() const
{
    int x, y, w, h;
    zrect_off(x, y, w, h);
    return x_location + x;
}
int Zombie::zrect_right() const
{
    int x, y, w, h;
    zrect_off(x, y, w, h);
    return x_location + x + w;
}
int Zombie::zrect_top() const
{
    int x, y, w, h;
    zrect_off(x, y, w, h);
    return y_location + y;
}
int Zombie::zrect_bottom() const
{
    int x, y, w, h;
    zrect_off(x, y, w, h);
    return y_location + y + h;
}

/*repvz mZombieAttackRect per type (ZombieInitialize / later phase sets).
Width 0 mirrors the original's Rect(0,0,0,0) (bungee d276, boss d681): the
horde never stops these at a plant. Phase-swapped boxes (pole vault d1797,
dolphin d1857/d1927, in-vault d8406) come from subclass overrides.*/
void Zombie::attack_x_span(int &left, int &width) const
{
    // default melee box (repvz Zombie ctor, Zombie.cpp:146)
    left = 50, width = 20;
    switch (type)
    {
    // LoadPlainZombieReanim family (:1008); jackbox :447; newspaper :599;
    // yeti :225.
    case NORMAL_TYPE:
    case FLAG_TYPE:
    case CONE_TYPE:
    case BUCKET_TYPE:
    case DOOR_TYPE:
    case DUCKYTUBE_TYPE:
    case NEWSPAPER_TYPE:
    case JACKBOX_TYPE:
    case YETI_TYPE:       left = 20,  width = 50;  break;
    case LADDER_TYPE:     left = 10,  width = 50;  break; // :232
    case SNORKEL_TYPE:    left = -5,  width = 55;  break; // :429
    case POLEVAULT_TYPE:  left = -29, width = 70;  break; // :335 (pre-vault; subclass refines)
    case POGO_TYPE:       left = 10,  width = 30;  break; // :593
    case ZOMBONI_TYPE:
    case CATAPULT_TYPE:   left = 10,  width = 133; break; // :406 / :423
    case GARGANTUAR_TYPE: left = -30, width = 89;  break; // :363
    case BALLOON_TYPE:    left = 20,  width = 50;  break; // :631
    case BUNGEE_TYPE:
    case ZOMBOSS_TYPE:    left = 0,   width = 0;   break; // :276 / :681
    default: break;
    }
}

float Zombie::get_health() const
{
    return health;
}

bool Zombie::wears_duckytube() const
{
    return directory_num == DUCKYTUBE_WALK_DIRECTORY ||
           directory_num == DUCKYTUBE_EAT_DIRECTORY;
}

/*Change zombie eating status
If is_moving is false, then the zombie must be eating.
Else the zombie must be moving.
*/
void Zombie::change_zombie_eating_status()
{
    if (is_moving == false)
    {
        auto dir = eat_of(directory_num);
        if (dir >= 0)
        {
            frame = 0;
            reanim_time = 0;
            directory_num = dir;
        }
    }
    else
    {
        if (walk_of(directory_num) >= 0)
        {
            frame = 0;
            reanim_time = 0;
            directory_num = walk_of(directory_num);
        }
    }
}

/*Determine zombies's appearance depend on their health:
Armor drop if necessary

Threshold semantics (repvz Zombie::TakeDamage uses subtractive helm/shield
health, so stages derive from remaining health, not from an exact-hit event):
a multi-damage shot (melon 4, fire pea 2, butter 2) can cross several tiers
in one tick, so fire one degrade step per tier boundary crossed, catching up
via degrade_steps. Steps fire in the same order a 1-dmg pea stream produces
them; the arm pops when health crosses 2/3 of the true body pool (the
original's 2/3-body mark, mBodyHealth < 2*mBodyMaxHealth/3,
Zombie.cpp:3892), never more than once.
*/
void Zombie::determine_appearance(vector<ZombiePart> &zombie_parts)
{
    // Tiers are the full-size table values; a big_trouble mini (every pool
    // /4, repvz Zombie.cpp:862-867) crosses each tier at tier*health_scale,
    // the same relative marks as the original's maxHealth-based 2/3 / 1/3
    // degrade checks (Zombie.cpp UpdateDamageStates).
    const set<float> &tiers = ZOMBIE_HEALTH_LIMIT[type];
    int idx = 0; // tier index walking the set top-down (skip the full-health tier)
    for (auto it = tiers.rbegin(); it != tiers.rend(); ++it, ++idx)
    {
        const float tier = *it;
        if (idx == 0)
            continue; // spawn health == largest tier: not a degrade step
        if (degrade_steps >= idx || health > tier * health_scale)
            continue;
        degrade_steps = idx;
        if (type == CONE_TYPE || type == BUCKET_TYPE)
        {
            auto d = degrade_of.find(armor_dir);
            if (d != degrade_of.end())
                armor_dir = d->second;
        }
        else
        {
            auto d = degrade_of.find(directory_num);
            if (d != degrade_of.end())
                directory_num = d->second;
            /*The arm pops crossing 2/3 of the TRUE body pool (repvz
            UpdateDamageStates: mBodyHealth < 2*mBodyMaxHealth/3,
            Zombie.cpp:3892) -- hardcoding tier <= 9 only matched the 270
            body. Armored carriers share the 270-normal body, the ladder
            holder has 500 body under its shield, everything else reaching
            this branch is body-only so the largest tier IS the pool (same
            table lose.cpp zombie_has_head uses). The boss keeps its parts.*/
            if (type != ZOMBOSS_TYPE)
            {
                float body_max;
                switch (type)
                {
                case DOOR_TYPE:
                case NEWSPAPER_TYPE:
                case FOOTBALL_TYPE:
                case BOBSLED_TYPE:
                    body_max = *ZOMBIE_HEALTH_LIMIT[NORMAL_TYPE].rbegin(); // 13.5 = 270 body
                    break;
                case LADDER_TYPE:
                    body_max = 25; // 500 body under the ladder shield
                    break;
                default:
                    body_max = *tiers.rbegin(); // body-only pool
                    break;
                }
                if (tier <= body_max * 2.0f / 3.0f + 1e-4f)
                {
                    if (reanimz::ready())
                        zombie_parts.push_back(ZombiePart(reanimz::DROP_ARM_IMG, 1, row, x_location,
                                                          ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0));
                    else
                        zombie_parts.push_back(ZombiePart(ZOMBIE_ARM_DIRECTORY, 1, row, x_location,
                                                          ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0));
                }
            }
        }
    }
    // Armor drop if necessary
    armor_drop(zombie_parts);
}

/*
Decrease zombie health and change their appearance if necessary.
@return true if zombie die (health <= 0, clamped at 0).
*/
bool Zombie::decrease_health(vector<ZombiePart> &zombie_parts)
{
    health -= 1;
    attacked_time = MAX_TIME_BLINK;
    float cap = shield_body_cap();
    if (cap >= 0)
    {
        float shield_total = *ZOMBIE_HEALTH_LIMIT[type].rbegin() * health_scale - cap;
        if (shield_damage < shield_total)
            shield_damage += 1; // soaked from the top: the shield takes it first
    }
    determine_appearance(zombie_parts);
    // clamp: an overkill hit must not leave the zombie walking at negative health
    if (health <= 0)
    {
        health = 0;
        return true;
    }
    return false;
}

/*
Massive one-tick hit (e.g. explosion leftovers): unlike calling decrease_health
in a loop (which spams one tossed arm per call when health crosses the arm-drop
threshold), degrade the appearance only once at the end. Note that big
jumps can skip degrade thresholds, so callers wanting exact armor visuals loop
decrease_health instead (the whack hammer in player_click.cpp does).
@return true if zombie dies (health <= 0).
*/
bool Zombie::take_hit(const float &dmg, vector<ZombiePart> &zombie_parts)
{
    health -= dmg;
    if (health < 0)
        health = 0; // clamp: overkill must not leave an immortal negative-health zombie
    float cap = shield_body_cap();
    if (cap >= 0)
    {
        // Straight damage soaks the shield tier first (and a shield-breaking
        // hit rolls what's left into the body -- the pooled health drops by
        // dmg either way, shield_damage just records the split).
        float shield_total = *ZOMBIE_HEALTH_LIMIT[type].rbegin() * health_scale - cap;
        shield_damage = min(shield_damage + dmg, shield_total);
    }
    attacked_time = MAX_TIME_BLINK;
    determine_appearance(zombie_parts);
    return health <= 0;
}

bool Zombie::carries_shield() const
{
    return type == DOOR_TYPE || type == NEWSPAPER_TYPE || type == LADDER_TYPE;
}

float Zombie::shield_body_cap() const
{
    switch (type)
    {
    case DOOR_TYPE:      return *ZOMBIE_HEALTH_LIMIT[NORMAL_TYPE].rbegin() * health_scale; // door drops at the normal cap (ArmoredZombie::armor_drop)
    case NEWSPAPER_TYPE: return 13.5f * health_scale; // NEWSPAPER_PAPER_BREAK_HEALTH: paper tier sits above 13.5 (270-pt body)
    case LADDER_TYPE:    return 25 * health_scale; // LADDER_BREAK_HP (500-pt body)
    default:             return -1;
    }
}

/*
Lobbed shots fly over a carried shield: the original tags them
DAMAGE_BYPASSES_SHIELD (Projectile::GetDamageFlags, MOTION_LOBBED), so
Zombie::TakeDamage skips TakeShieldDamage and every point lands on the body
underneath; when the body dies the shield drops with the corpse (its tier is
never crossed, so the drop visuals stay attached).
The shield's remaining health comes from shield_damage (soaked shots fill it
first, magnet-shroom's full-strip take_hit sets it to the shield's size);
body health is what's left of the pool. The shield keeps its health here --
even the ping that the shield survived a lobbed hit matters when it re-enters
straight-shot range.
Taking damage here skips determine_appearance on purpose: the shield tiers
track shield_damage already, and a killing hit gets its art from
add_zombie_die on the same tick.
@return true if zombie dies (body health <= 0).
*/
bool Zombie::take_body_hit(const float &dmg, vector<ZombiePart> &zombie_parts)
{
    float cap = shield_body_cap();
    if (cap < 0) // no carried shield (helms soak like any straight shot)
        return take_hit(dmg, zombie_parts);
    float shield_total = *ZOMBIE_HEALTH_LIMIT[type].rbegin() * health_scale - cap;
    float shield_hp = shield_total - shield_damage;
    if (shield_hp < 0)
        shield_hp = 0;
    float body_hp = health - shield_hp - dmg;
    attacked_time = MAX_TIME_BLINK;
    if (body_hp <= 0)
    {
        health = 0; // body dead: shield and corpse fall together
        return true;
    }
    health = shield_hp + body_hp; // == health - dmg, booked into the body
    return false;
}

/*
Fume-shroom spore soak (repvz DAMAGE_HITS_SHIELD_AND_BODY,
Plant.cpp:4548-4555 + Zombie.cpp:8056-8062): a carried shield takes the
full damage AND the body takes the full damage -- the shield chips for its
crack paper/rage beats but soaks nothing, so shield carriers die from body
damage alone (13.5 hits for door/newspaper bodies, exactly like shields
were not there). Helms (cone/bucket/football) are not shields in the
original: the cloud soaks them like any straight shot.
Note the shield stages/rage fire via determine_appearance here (unlike
take_body_hit, which keeps lobbed shots' art pinned to the corpse).
@return true if zombie dies (body health <= 0).
*/
bool Zombie::take_shield_and_body_hit(const float &dmg, vector<ZombiePart> &zombie_parts)
{
    float cap = shield_body_cap();
    if (cap < 0) // helm or bare body: a straight soak
        return take_hit(dmg, zombie_parts);
    float shield_total = *ZOMBIE_HEALTH_LIMIT[type].rbegin() * health_scale - cap;
    float shield_hp = shield_total - shield_damage;
    if (shield_hp < 0)
        shield_hp = 0;
    float chip = min(dmg, shield_hp); // the shield takes the full hit too...
    float body_hp = health - shield_hp - dmg; // ...and the body takes the full hit
    shield_damage += chip;
    attacked_time = MAX_TIME_BLINK;
    if (body_hp <= 0)
    {
        health = 0; // body dead: shield and corpse fall together
        return true;
    }
    health = shield_hp - chip + body_hp;
    determine_appearance(zombie_parts); // shield-crack art, newspaper rage
    return false;
}
/*Charred corpse kind: one reanim doc per zombie group (repvz ApplyBurn
Zombie.cpp:8824-8862); common types share the generic body.*/
int Zombie::charred_kind() const
{
    return reanimz::charredKindForType(type);
}

void push_burned_corpse(vector<ZombiePart> &zombie_parts, const Zombie *z)
{
    if (reanimz::ready())
    {
        // the ZombiePart `skin` slot carries the CHARRED_* kind
        zombie_parts.push_back(ZombiePart(reanimz::CHARRED_PART_IMG, 1, z->row, z->x_location,
                                          ZOMBIE_WIDTH, ZOMBIE_HEIGHT, false, z->charred_kind()));
        return;
    }
    zombie_parts.push_back(ZombiePart(ZOMBIE_BURNT_DIRECTORY, ZOMBIE_DIE_FRAME, z->row,
                                      z->x_location, ZOMBIE_WIDTH, ZOMBIE_HEIGHT));
}

/*
Add zombie die: body & head
*/
void Zombie::add_zombie_die(vector<ZombiePart> &zombie_parts)
{
    // Died while in the pool (repvz mInPool): plain-family zombies on a water
    // lane sink with anim_waterdeath and the short fade, tube still on.
    const bool drowned =
        level.row_has_water(row) &&
        (wears_duckytube() || type == NORMAL_TYPE || type == CONE_TYPE ||
         type == BUCKET_TYPE || type == FLAG_TYPE);
    if (reanimz::ready())
    {
        // one reanim death animation (head pop included) instead of 2 sheets
        zombie_parts.push_back(ZombiePart(reanimz::DEATH_PART_IMG, 1, row, x_location,
                                          ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                          0, drowned));
        if (type == FLAG_TYPE)
            zombie_parts.push_back(ZombiePart(reanimz::DROP_FLAG_IMG, 1, row, x_location,
                                              ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0));
        return;
    }
    zombie_parts.push_back(ZombiePart(ZOMBIE_DIE_DIRECTORY, ZOMBIE_DIE_FRAME,
                                      row, x_location,
                                      ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0));
    zombie_parts.push_back(ZombiePart(ZOMBIE_HEAD_DIRECTORY, HEAD_ZOMBIE_FRAME,
                                      row, x_location + 80,
                                      HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT, cold_time > 0));
}

/*
Crush kill (squash slam, repvz Plant::DoSquashDamage Plant.cpp:1406-1423):
the squash's hit carries DAMAGE_DOESNT_LEAVE_BODY (TakeDamage(1800, 18U)),
so a dying zombie runs PlayDeathAnim's early-out (Zombie.cpp:9039-9047) --
DieNoLoot with NO falling body, NO head pop, NO corpse: it simply vanishes
under the squash (no "flattened" sprite exists in the original assets).
Push nothing on purpose.
*/
void Zombie::add_zombie_crushed(vector<ZombiePart> &zombie_parts)
{
    (void)zombie_parts;
}

void Zombie::render_zombie()
{
    // The reanim hook (reanim_game_hooks.cpp) reads layout_dx/layout_dy and
    // the rsdl debug scale itself.
    if (render_reanim(0))
        return;
    // Roof slope: zombies walk down/up the sloped columns visually
    // (Board::GetPosYBasedOnRow); render-only, movement stays on the grid.
    const int ry = y_location + roof_slope_offset(x_location);
    // Bottom-left anchor: drop the frame's feet row onto the reanim path's
    // ground line (the door's taller sheets pad their feet differently).
    const int feet = dir_height > ZOMBIE_HEIGHT ? ZOMBIE_DOOR_FEET_Y : ZOMBIE_SHEET_FEET_Y;
    const int y_bottom = ry + ZOMBIE_SHEET_GROUND_Y + dir_height - feet;
    // F2 layout-editor adjustments (transient): offsets shift the anchor;
    // the debug scale grows the frame about its bottom-center (feet) point.
    // dbg == 1.0 and zero offsets in normal gameplay.
    const float dbg = rsdl_debug_scale(this);
    const int dw = (int)lroundf(dir_width * dbg * WORLD_SCALE);
    const int dh = (int)lroundf(dir_height * dbg * WORLD_SCALE);
    const int zx = x_location + layout_dx + (dir_width - dw) / 2;
    const int zyb = y_bottom + layout_dy;
    // zombie
    // current frame
    int sframe = frame / ZOMBIE_FRAME;
    win.draw_nth_frame_bl(directory_num, sframe, dir_width, dir_height,
                          zx, zyb, dw, dh);

    // zombie cold (also the ice-shroom's full freeze, which looks the same)
    if (cold_time || freeze_time)
    {
        win.set_texture_color(blink_of(directory_num), 0, 75, 255);
        win.set_texture_alpha(blink_of(directory_num), 120);
        win.draw_nth_frame_bl(blink_of(directory_num), sframe, dir_width, dir_height,
                              zx, zyb, dw, dh);
        win.set_texture_color(blink_of(directory_num), 255, 255, 255);
        win.set_texture_alpha(blink_of(directory_num), 70);
    }

    // Hypnotized: the whole body reads purple (repvz Zombie.cpp:5727-5732:
    // mColorOverride = ZOMBIE_MINDCONTROLLED_COLOR 230,15,245 + additive extra
    // draw; the sheet path approximates it like the cold tint above).
    if (is_charmed)
    {
        win.set_texture_color(blink_of(directory_num), 230, 15, 245);
        win.set_texture_alpha(blink_of(directory_num), 110);
        win.draw_nth_frame_bl(blink_of(directory_num), sframe, dir_width, dir_height,
                              zx, zyb, dw, dh);
        win.set_texture_color(blink_of(directory_num), 255, 255, 255);
        win.set_texture_alpha(blink_of(directory_num), 70);
    }

    // zombie attacked
    if (attacked_time)
    {
        win.draw_nth_frame_bl(blink_of(directory_num), sframe, dir_width, dir_height,
                              zx, zyb, dw, dh);
    }
}

/*
display zombie
*/
void Zombie::display(const int &_row)
{
    if (row == _row)
    {
        const int ry = y_location + roof_slope_offset(x_location);
        render_zombie();
        /*The overlays ride the body art: the sheet fallback is bottom-anchored
        on the ground line now, so its top sits `body_lift` px off the old box
        top (5 for the 144-frame sheets, -1 for the door's 197). On the reanim
        path the box top is unchanged -- no lift there.*/
        int body_lift = 0;
        if (!reanimz::ready())
        {
            const int feet = dir_height > ZOMBIE_HEIGHT ? ZOMBIE_DOOR_FEET_Y : ZOMBIE_SHEET_FEET_Y;
            body_lift = feet - ZOMBIE_SHEET_GROUND_Y;
        }
        // hypno trance: a purple aura hugs the body (repvz
        // ZOMBIE_MINDCONTROLLED_COLOR mColorOverride + additive extra draw,
        // Zombie.cpp:5727-5732; the zombie keeps its left-facing art and
        // moonwalks right, exactly like the original's IsWalkingBackwards).
        if (is_charmed)
        {
            int glow_phase = (frame / ZOMBIE_FRAME) & 31;
            int pulse = 90 + (glow_phase <= 16 ? glow_phase : 32 - glow_phase) * 4;
            win.set_texture_color(HYPNO_AURA_DIRECTORY, 230, 15, 245);
            win.set_texture_alpha(HYPNO_AURA_DIRECTORY, pulse);
            win.draw_png(HYPNO_AURA_DIRECTORY, x_location + layout_dx + 8, ry - 20 + layout_dy - body_lift, world_px(150), world_px(165));
            win.set_texture_color(HYPNO_AURA_DIRECTORY, 255, 255, 255);
            win.set_texture_alpha(HYPNO_AURA_DIRECTORY, all_img[HYPNO_AURA_DIRECTORY].alpha);
            // one-shot swirl burst right after the bite (repvz
            // PARTICLE_MIND_CONTROL emitter: the glyph recolored to the charm
            // purple, Zombie.cpp:3417-3418)
            if (charm_flash > 0)
            {
                win.set_texture_color(MIND_CONTROL_DIRECTORY, 230, 15, 245);
                win.draw_png_height_scaled(MIND_CONTROL_DIRECTORY, x_location + layout_dx + 25, ry - 15 + layout_dy - body_lift, world_px(45));
                win.set_texture_color(MIND_CONTROL_DIRECTORY, 255, 255, 255);
                // anim advances per logic tick, not per render (auto-FPS)
                for (int _lt = logic_ticked; _lt > 0 && charm_flash > 0 && check_status(game_state, IS_PAUSED) == false; _lt--)
                    charm_flash--;
            }
        }
        // kernel-pult butter pat pasted on the zombie's crown (repvz
        // Zombie::DrawButter, Zombie.cpp:6274-6321: the splat rides the
        // anim_head1 track -- pinned to the head, scaled with the skin and
        // the big-trouble half-size art via the same geometry the body uses).
        if (butter_time)
        {
            float head_cx, head_top, art_mul;
            // The body's own draw coords (reanim hook): layout offsets folded
            // in, slope taken at the drawn anchor.
            const int body_x = x_location + layout_dx;
            const int body_y = y_location + layout_dy + roof_slope_offset(body_x);
            if (reanimz::ready() &&
                reanimz::headAnchor(directory_num, armor_dir, reanim_time,
                                    cold_time > 0, body_x, body_y,
                                    head_cx, head_top, art_mul))
            {
                const int bw = max(1, (int)lroundf(36.0f * art_mul));
                const int bh = max(1, (int)lroundf(22.0f * art_mul));
                win.draw_png(BUTTER_BULLET_DIRECTORY,
                             (int)lroundf(head_cx - 2.0f * art_mul) - bw / 2,
                             (int)lroundf(head_top - 0.5f * bh), bw, bh);
            }
            else // sheet fallback: keep the legacy hand-tuned offset
                win.draw_png(BUTTER_BULLET_DIRECTORY, x_location + layout_dx + 40, ry + 8 + layout_dy - body_lift, world_px(36), world_px(22));
        }
        // Hit-flash decay runs even while frozen/buttered: repvz decrements
        // mJustGotShotCounter on every Zombie::Update (Zombie.cpp:4366-4367).
        // Gating it under the stun held the full-body flash for the whole
        // butter hold (the "blinking stunned zombie" bug).
        for (int _lt = logic_ticked; _lt > 0 && check_status(game_state, IS_PAUSED) == false && attacked_time > 0; _lt--)
            attacked_time--;
        // zombie next frame (a fully frozen zombie does not even animate);
        // anims advance ONCE PER LOGIC TICK (auto-FPS: logic_ticked = the
        // tick count of this rendered frame -- turbo runs 2, keeping the
        // doubled movement and anim rates in lockstep).
        for (int _lt = logic_ticked; _lt > 0 && check_status(game_state, IS_PAUSED) == false && !freeze_time && !butter_time; _lt--)
        {
            if (cold_time)
                if (--cold_time == 0)
                {
                    bite_time /= 2;
                    next_step_time /= 2;
                }
            // Walk anim advances on GROUND COVERED, not time: one sheet pose
            // per ZOMBIE_WALK_STRIDE_PX px of travel (retail _ground-track
            // stride lock; see zombie_struct.hpp). The rolled and the chilled
            // pace both scale the step rate through speed_px_per_tick(), so
            // feet stay planted -- the old fixed every-tick advance scurried
            // ~2.4x too fast for the ground actually covered.
            const float walk_v = speed_px_per_tick() * (cold_time ? CHILLED_SPEED_FACTOR : 1.0f);
            bool walk_fed = false;
            if (is_moving && walk_v > 0.0f)
            {
                walk_stride_acc += walk_v;
                while (walk_stride_acc >= ZOMBIE_WALK_STRIDE_PX)
                {
                    walk_stride_acc -= ZOMBIE_WALK_STRIDE_PX;
                    frame++;
                }
                // reanim walk: the same stride lock (retail UpdateAnimSpeed)
                // -- feed units per ground px from the doc's _ground track.
                // < 0 = not a distance-driven walk state (eat, one-shots,
                // snorkel/pogo/bobsled/catapult/dolphin): pinned rate below.
                const float feed_per_px =
                    reanimz::walkFeedPerPx(directory_num, armor_dir, cold_time > 0);
                if (feed_per_px >= 0.0f)
                {
                    reanim_time += walk_v * feed_per_px;
                    walk_fed = true;
                }
            }
            else if (cold_time % 2 == 0)
            {
                // Non-walk states (eating, ...) keep the per-tick pose; the
                // discrete sheets halve it while chilled.
                frame++;
            }
            // reanim runs at 12 fps, the game at 60 -> reanim_rate (0.2 by
            // default); the distance-driven walk states advanced on ground
            // covered above instead. Subclasses can speed a segment up
            // (pole vault).
            if (!walk_fed)
                reanim_time += reanim_rate;
            {
                float seg_len = reanimz::segmentLength(directory_num, cold_time > 0);
                if (seg_len > 0 && reanim_time >= seg_len)
                    reanim_time -= seg_len;
            }
            if (frame >= ZOMBIE_FRAME * all_img[directory_num].n_sheet)
                frame = 0;
        }
    }
}

/*
Grave-rise emergence render (see zombie_struct.hpp). The zombie is NOT in
game_characters yet -- this only draws the pending riser of a
level.grave_rises record, sliding it up out of the dirt with the retail
altitude + ground clip.
*/
void Zombie::display_grave_rise(const int &rise_clk)
{
    if (rise_clk <= 0 || rise_clk > GRAVE_RISE_EMERGE_TICKS)
        return;
    // repvz mAltitude: TodAnimateCurve(50, 0, mPhaseCounter, -200, 0) with the
    // phase counter in centiseconds; 50cs == GRAVE_RISE_EMERGE_TICKS ticks, so
    // `buried` runs 200px -> 0 as rise_clk runs 30 -> 0.
    const float buried = 200.0f * WORLD_SCALE * (float)rise_clk / (float)GRAVE_RISE_EMERGE_TICKS;
    const int dx = x_location + layout_dx;
    const int slope = roof_slope_offset(dx);
    // Feet line of the standing pose (reanim ART_BOTTOM_Y == sheet ground).
    const int ground_y = y_location + layout_dy + slope + ZOMBIE_SHEET_GROUND_Y;
    // DrawReanim's clip: the band bottom sits at ground + min(phase, 40) doc
    // px (the -7 under the imageOffsetY+113/ART_BOTTOM 136 mapping), so the
    // emerging head is the only thing above ground early on.
    const int overhang = min((int)(rise_clk * (5.0f / 3.0f) * WORLD_SCALE), world_px(40));
    const int clip_bottom = ground_y - world_px(7) + overhang;
    if (clip_bottom <= 0)
        return;

    if (!reanimz::ready())
        reanimz::init(win.sprite_renderer());
    if (reanimz::ready())
    {
        // Risers never pass through the live-zombie render hook, so set the
        // same scale it would: mode factor (big-trouble half size) times the
        // world knob. Matches the stale-inherit behavior in normal levels.
        reanimz::setZombieScaleMul(
            (level.is_special(LevelSpecial::BigTrouble) ? 0.5f : 1.0f) *
            WORLD_SCALE);
        SDL_Rect clip = {0, 0, WINDOW_WIDTH, clip_bottom};
        win.sprite_renderer().setClipRect(&clip);
        reanimz::drawZombie(directory_num, armor_dir, reanim_time, dx,
                            (int)lround(y_location + layout_dy + slope + buried),
                            false, false);
        win.sprite_renderer().setClipRect(nullptr);
    }
    else
    {
        // Sprite-sheet dev fallback (no reanim docs): same slide + a source
        // crop standing in for the clip -- only the part of the frame above
        // the ground line is drawn (sheet frames draw 1:1, so source rows ==
        // destination rows).
        const int feet =
            dir_height > ZOMBIE_HEIGHT ? ZOMBIE_DOOR_FEET_Y : ZOMBIE_SHEET_FEET_Y;
        const int zyb =
            y_location + slope + ZOMBIE_SHEET_GROUND_Y + dir_height - feet +
            (int)lround(buried) + layout_dy;
        const int vis = clip_bottom - (zyb - dir_height);
        const int cols = all_img[directory_num].c_sheet;
        if (vis > 0 && cols > 0)
        {
            const int sframe = frame / ZOMBIE_FRAME;
            if (vis >= dir_height)
                win.draw_nth_frame_bl(directory_num, sframe, dir_width,
                                      dir_height, dx, zyb, dir_width,
                                      dir_height);
            else
                win.draw_png(directory_num, dir_width * (sframe % cols),
                             dir_height * (sframe / cols), dir_width, vis, dx,
                             zyb - dir_height, dir_width, vis);
        }
    }

    // Same aging as Zombie::display, per logic tick (auto-FPS) -- no
    // cold/butter/attack on a riser that is still underground.
    if (check_status(game_state, IS_PAUSED))
        return;
    for (int _lt = logic_ticked; _lt > 0; _lt--)
    {
        frame++;
        reanim_time += reanim_rate;
        {
            float seg_len = reanimz::segmentLength(directory_num, false);
            if (seg_len > 0 && reanim_time >= seg_len)
                reanim_time -= seg_len;
        }
        if (frame >= ZOMBIE_FRAME * all_img[directory_num].n_sheet)
            frame = 0;
    }
}

/*
display credited zombie
*/
void Zombie::display_credited(const int &_minus_x)
{
    const int REAL_FRAME = ZOMBIE_FRAME * 2;
    if (render_reanim(_minus_x))
    {
        // credit zombies animate at half speed (REAL_FRAME is 2x ZOMBIE_FRAME)
        reanim_time += 0.1f;
        float seg_len = reanimz::segmentLength(directory_num);
        if (seg_len > 0 && reanim_time >= seg_len)
            reanim_time -= seg_len;
        if (++frame >= REAL_FRAME * all_img[directory_num].n_sheet)
        {
            frame = 0;
        }
        return;
    }
    // current frame (feet anchored on the same ground line as the reanim path)
    int sframe = frame / REAL_FRAME;
    const int feet = dir_height > ZOMBIE_HEIGHT ? ZOMBIE_DOOR_FEET_Y : ZOMBIE_SHEET_FEET_Y;
    const int dw = world_px(dir_width);
    const int dh = world_px(dir_height);
    win.draw_nth_frame_bl(directory_num, sframe, dir_width, dir_height,
                          x_location - _minus_x + (dir_width - dw) / 2,
                          y_location + ZOMBIE_SHEET_GROUND_Y + dir_height - feet,
                          dw, dh);

    // zombie next frame
    if (++frame >= REAL_FRAME * all_img[directory_num].n_sheet)
    {
        frame = 0;
    }
}

/*
Zombie part constructor
*/
ZombiePart::ZombiePart(const int &_img_num, const int &_frame_clk,
                       const int &_r, const int &_x,
                       const int &_w, const int &_h, const bool &_cold,
                       const int &_skin, const bool &_in_water)
{
    img_num = _img_num;
    frame_clk = _frame_clk;
    row = _r;
    x_location = _x;
    width = _w;
    height = _h;
    is_cold = _cold;
    skin = _skin;
    in_water = _in_water;
}

/*@return 'true' if display finished*/
bool ZombiePart::display_reanim_charred(const int &_row)
{
    if (row != _row)
        return false;
    // Corpses land on the same sloped ground line as the walking zombie.
    int y_location = cells[row][0].y1 - 50 + roof_slope_offset(x_location);
    reanimz::drawCharredPart(skin, frame, x_location, y_location);
    // anim advances per logic tick, not per render (auto-FPS)
    for (int _lt = logic_ticked; _lt > 0 && check_status(game_state, IS_PAUSED) == false; _lt--)
        frame++;
    return frame >= reanimz::charredLingerTicks(skin);
}

/*@return 'true' if display finished*/
bool ZombiePart::display(const int &_row)
{
    if (img_num == reanimz::DEATH_PART_IMG)
        return display_reanim_death(_row);
    if (img_num <= reanimz::DROP_ARM_IMG && img_num >= reanimz::DROP_FLAG_IMG)
        return display_reanim_drop(_row);
    if (img_num == reanimz::CHARRED_PART_IMG)
        return display_reanim_charred(_row);
    if (row == _row)
    {
        // Corpses land on the same sloped ground line as the walking zombie.
        int y_location = cells[row][0].y1 - 50 + roof_slope_offset(x_location);
        // Same bottom-left anchor as the body: the part sheets come from the
        // 144-box art set, feet row ZOMBIE_SHEET_FEET_Y (the 156-tall head
        // sheet keeps the same 5 px lift).
        const int y_bottom = y_location + ZOMBIE_SHEET_GROUND_Y + height - ZOMBIE_SHEET_FEET_Y;
        // current frame
        int sframe = frame / frame_clk;
        const int dw = world_px(width);
        const int dh = world_px(height);
        const int dx = x_location + (width - dw) / 2;
        win.draw_nth_frame_bl(img_num, sframe, width, height,
                              dx, y_bottom, dw, dh);
        // cold
        if (is_cold)
        {
            win.set_texture_color(blink_of(img_num), 0, 75, 255);
            win.set_texture_alpha(blink_of(img_num), 120);
            win.draw_nth_frame_bl(blink_of(img_num), sframe, width, height,
                                  dx, y_bottom, dw, dh);
            win.set_texture_color(blink_of(img_num), 255, 255, 255);
            win.set_texture_alpha(blink_of(img_num), 70);
        }
        // next frame: anim advances per logic tick, not per render (auto-FPS);
        // un-incremented frames just report the same state again.
        if (check_status(game_state, IS_PAUSED) == false)
        {
            for (int _lt = logic_ticked; _lt > 0; _lt--)
                ++frame;
            if (frame >= frame_clk * all_img[img_num].n_sheet)
            {
                return true;
            }
        }
    }
    return false;
}
