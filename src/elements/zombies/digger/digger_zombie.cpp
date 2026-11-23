#include "digger_zombie.hpp"
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Map/Map.hpp"
#include "music/music.hpp"
#include "reanim/reanim_zombie.hpp"

extern Map cells;

// Tunneling pace: PHASE_DIGGER_TUNNELING rolls 0.66-0.68 px/cs in the
// original (repvz PickRandomSpeed, Zombie.cpp:1201-1205; mid-band 0.67 here)
// -> 36.5 px/s under HEAD_SPEED_SCALE.
const float DIGGER_TUNNEL_PX = CS_VEL(0.67f) * HEAD_SPEED_SCALE;
// Turned-around walk-back: fixed 0.12 px/cs in the original
// (PHASE_DIGGER_WALKING, repvz Zombie.cpp:1173-1183) -> 6.5 px/s at HEAD pace.
const float DIGGER_WALKBACK_PX = CS_VEL(0.12f) * HEAD_SPEED_SCALE;
// The original rise arc runs 130 ticks, then the dizziness holds ~2.5 s.
const int DIGGER_RISE_TICKS = 110;
const int DIGGER_DIZZY_TICKS = 150;
// Mid-tunnel axe theft: the digger hangs underground ~2 s (original parks
// PHASE_DIGGER_TUNNELING_PAUSE_WITHOUT_AXE for 200 centisecs), then rises.
const int DIGGER_STALL_TICKS = 120;
// digger_zombie.ogg (2.1 s) spans the underground stretch start.
const int DIGGER_DIG_SND_CLK = 120;

DiggerZombie::~DiggerZombie()
{
}

void DiggerZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = *ZOMBIE_HEALTH_LIMIT[DIGGER_TYPE].rbegin();
    directory_num = DIGGER_WALK_DIRECTORY;
    // Original ZombieInit (repvz Zombie.cpp:290-317): the digger appears at
    // the right edge ALREADY tunneling (PHASE_DIGGER_TUNNELING at spawn --
    // no surface stroll in front of the lawn; it is bullet/mower-immune and
    // passes under plants from the first tick).
    state = TUNNELING;
    frame = 0;
    reanim_time = 0.0f;
    state_ticks = DIGGER_DIG_SND_CLK; // dig loop sound once it digs into view
}

void DiggerZombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - dir_height);
    directory_num = DIGGER_WALK_DIRECTORY;
    state = WALK_BACK;
}

const char *DiggerZombie::render_segment() const
{
    switch (state)
    {
    case TUNNELING:
    case TUNNEL_STALL: // frozen on the spot: still the dig loop art
        return "anim_dig";
    case RISING:
        // The data's anim_drill loop plays the whole rise.
        return "anim_drill";
    case DIZZY:
        return "anim_dizzy";
    default:
        return nullptr;
    }
}

bool DiggerZombie::can_lose_pickaxe() const
{
    return pickaxe && (state == WALK_IN || state == TUNNELING);
}

void DiggerZombie::lose_pickaxe()
{
    if (!pickaxe)
        return;
    pickaxe = false;
    if (state == TUNNELING)
    {
        // repvz TUNNELING_PAUSE_WITHOUT_AXE (200 cs): park underground, then
        // the TUNNEL_STALL branch rises in place (RISE_WITHOUT_AXE).
        state = TUNNEL_STALL;
        state_ticks = DIGGER_STALL_TICKS;
        reanim_rate = 0.0f; // SetAnimRate(0.0f): frozen mid-dig
        return;
    }
    if (state == WALK_IN)
        state = WALK_FORWARD; // no axe: never digs in, walks on left
}

bool DiggerZombie::on_move_tick()
{
    switch (state)
    {
    case WALK_IN:
        // Surface entry (credit scene / defensive fallback): walks in, then
        // digs. Real spawns start TUNNELING in init() already.
        if (pickaxe && x_location < cells[row][6].x2)
        {
            state = TUNNELING;
            frame = 0;
            reanim_time = 0.0f;
            state_ticks = DIGGER_DIG_SND_CLK;
        }
        return false;
    case TUNNEL_STALL:
        if (--state_ticks <= 0)
        {
            // RISE_WITHOUT_AXE: surface here, then DIZZY is skipped and the
            // digger walks left like any zombie (WALK_FORWARD).
            state = RISING;
            state_ticks = DIGGER_RISE_TICKS;
            frame = 0;
            reanim_time = 0.0f;
            reanim_rate = 0.2f;
        }
        return true;
    case TUNNELING:
    {
        // One dig-loop sound as it digs in (spawn and legacy WALK_IN entry
        // both arm state_ticks; -1 = already played).
        if (state_ticks && --state_ticks == 0)
        {
            state_ticks = -1;
            play_sound_effect(DIGGER_MUSIC_DIRECTORY);
        }
        walk_carry += DIGGER_TUNNEL_PX; // tunnels on, chilled or not (status quo)
        int step = (int)walk_carry;
        walk_carry -= step;
        x_location -= step;
        if (x_location < cells[row][0].x1 + 35)
        {
            state = RISING;
            state_ticks = DIGGER_RISE_TICKS;
            frame = 0;
            reanim_time = 0.0f;
            reanim_rate = 0.2f;
            play_sound_effect(DIGGER_MUSIC_DIRECTORY);
        }
        return true;
    }
    case RISING:
        if (--state_ticks <= 0)
        {
            // With the axe in hand the digger shakes off the dizzy beat and
            // turns around; without it (RISE_WITHOUT_AXE) it just walks on.
            state = pickaxe ? DIZZY : WALK_FORWARD;
            state_ticks = pickaxe ? DIGGER_DIZZY_TICKS : 0;
            frame = 0;
            reanim_time = 0.0f;
        }
        return true;
    case DIZZY:
        if (--state_ticks <= 0)
        {
            state = WALK_BACK;
            is_moving = true;
            frame = 0;
            reanim_time = 0.0f;
        }
        return true;
    case WALK_FORWARD:
        // Axe-less walker: the base stepper handles everything.
        return false;
    case WALK_BACK:
    {
        // Walked off the right edge: leaves the lawn for good.
        if (x_location > WINDOW_WIDTH + 20)
        {
            walked_off = true;
            return true;
        }
        float v = DIGGER_WALKBACK_PX;
        if (cold_time)
            v *= CHILLED_SPEED_FACTOR;
        walk_carry += v;
        int back_step = (int)walk_carry;
        walk_carry -= back_step;
        if (is_moving)
            x_location += back_step;
        return true;
    }
    }
    return false;
}

/*Underground and mid-transition the digger ignores plant blocking entirely;
walking back (or on, axe-less) it stops and chews like any zombie
(nose-overlap rule).*/
bool DiggerZombie::on_reach_plant(const int &col)
{
    return state != WALK_BACK && state != WALK_FORWARD;
}

bool DiggerZombie::can_eat_plant() const
{
    return state == WALK_BACK || state == WALK_FORWARD;
}

bool DiggerZombie::is_underground() const
{
    return state == TUNNELING || state == TUNNEL_STALL || state == RISING;
}

/*Degrade is a plain flash (the digger skin wires no armless variant).*/
void DiggerZombie::determine_appearance(vector<ZombiePart> &zombie_parts)
{
}

void DiggerZombie::add_zombie_die(vector<ZombiePart> &zombie_parts)
{
    if (reanimz::ready())
    {
        zombie_parts.push_back(ZombiePart(reanimz::DEATH_PART_IMG, 1, row, x_location,
                                          ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                          reanimz::SKIN_DIGGER, false, head_dropped));
        return;
    }
    Zombie::add_zombie_die(zombie_parts);
}

int DiggerZombie::charred_kind() const
{
    return pickaxe ? reanimz::CHARRED_DIGGER : reanimz::CHARRED_DIGGER_NOAXE;
}

void DiggerZombie::render_zombie()
{
    const char *seg = render_segment();
    if (seg && reanimz::drawSkinSegment(reanimz::SKIN_DIGGER, seg, reanim_time,
                                        x_location, y_location,
                                        cold_time > 0, attacked_time > 0,
                                        0, 0, WORLD_SCALE))
        return;
    Zombie::render_zombie();
}
