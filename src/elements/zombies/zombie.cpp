#include "zombie.hpp"
#include "draw/rsdl.hpp"
#include "elements/Map/Map.hpp"
#include "elements/elements.hpp"
#include "music/music.hpp"
#include <algorithm>

extern int game_state;
extern Map cells;
extern Window win;
extern Elements game_characters;

/*Continuous locomotion (retail pacing): every tick the speed (retail
CS_VEL x HEAD_SPEED_SCALE, plain mean 0.30 px/cs = 30 px/s) accrues
into walk_carry and the integer x_location steps only the whole-pixel
part, so a plain walker glides 0/1 px per tick around its mean (fast
classes may occasionally take a 2-px tick) -- never a big lurch at
foot plants. The reanim walk loop is pace-pinned (reanim_zombie.cpp
head_pace_fps, HEAD_WALK_FPS = 8.33 fps = the retail legacy 12-cs pose)
and the sheet walk stride-locks on ground covered (ZOMBIE_WALK_STRIDE_PX),
both coupled to the ground speed like retail's UpdateAnimSpeed.*/

/*
repvz Plant::GetPlantRect horizontal span of tile (row,col)
(Plant.cpp:5221-5240; base plant box mWidth=80/mHeight=80 at Plant.cpp:129-130):
    default:          Rect(mX+10, mY, mWidth-20, ...) -> [x1+10, x2-10]
    TALLNUT (:282):   Rect(mX+10, mY, mWidth, ...)    -> [x1+10, x1+90]
    PUMPKIN (:380-384, mWidth=120): Rect(mX, mY, 100) -> [x1, x1+100]
    (the clone ships no Cob Cannon, so its 140-px rect (:595) has no case here)
FindPlantTarget (Zombie.cpp:6459-6476) accepts the first plant whose rect the
attack box overlaps by >=20, so the effective target band of a stacked tile
uses the union edges.
*/
void plant_rect_span(const int &row, const int &col, int &r_left, int &r_right)
{
    const Block &c = cells[row][col];
    int lacc = c.x1 + 10, racc = c.x2 - 10; // default rect; also the empty-tile fallback
    bool found = false;
    for (auto *p : game_characters.plants)
    {
        if (p->get_row() != row || p->get_col() != col)
            continue;
        int l, r;
        switch (p->get_type())
        {
        case TALLNUT_TYPE:   l = c.x1 + 10; r = c.x1 + 90;  break;
        case PUMPKIN_TYPE:   l = c.x1;      r = c.x1 + 100; break;
        default:             l = c.x1 + 10; r = c.x2 - 10;  break;
        }
        if (!found)
        {
            lacc = l, racc = r, found = true;
        }
        else
        {
            lacc = std::min(lacc, l);
            racc = std::max(racc, r);
        }
    }
    r_left = lacc;
    r_right = racc;
}

/*
Check if Zombie is in query tile or not to change its status.
repvz Zombie::FindPlantTarget (Zombie.cpp:6459-6476): the zombie's per-type
attack box (mZombieAttackRect, attack_x_span) must overlap the plant rect
(above) by at least 20 px before the bite locks on (the per-type mouth box
replaces the old flat x-1+95 probe point).
@param zombie: query zombie
@param row: query row
@param col: query column
@return true if zombie is not in query tile.
*/
bool has_zombie_reached_block(const Zombie &zombie, const int &row, const int &col)
{
    if (zombie.row != row)
        return false;
    int a_left, a_width;
    zombie.attack_x_span(a_left, a_width);
    if (a_width <= 0)
        return false; // bungee/boss-type: no tile to chew (repvz Rect(0,0,0,0))
    int p_left, p_right;
    plant_rect_span(row, col, p_left, p_right);
    const int left = zombie.x_location + a_left;
    const int right = left + a_width;
    return std::min(right, p_right) - std::max(left, p_left) >= 20;
}

/*
Check if zombie reached a planted tile.
If true, Update zombie status.
*/
bool has_zombie_reached_plant(Zombie &zombie)
{
    // D1: a headless zombie is dying and cannot eat (repvz !mHasHead makes
    // CheckIfPreyCaught bail out), so it never stops at a tile -- it shambles
    // straight over the plant while bleeding out.
    if (zombie.head_dropped)
        return false;
    // Scan columns right-to-left: the zombie walks left, so the rightmost
    // blocked tile its attack box overlaps is the one it bumps into first
    // (wide vehicle boxes can bridge two tiles at once, repvz (:406 133 px).
    for (int col = HORIZ_BLOCK_COUNT - 1; col >= 0; col--)
    {
        // A ladder placed by a ladder zombie neutralizes the plant's block:
        // every zombie (the ladder zombie included) climbs over the tile.
        if (cells[zombie.row][col].is_block_zombie && !cells[zombie.row][col].has_ladder &&
            has_zombie_reached_block(zombie, zombie.row, col))
        {
            // subclass override (pole vaulter jumps the first plant instead)
            if (zombie.on_reach_plant(col))
                return true;
            zombie.is_moving = false;
            zombie.change_zombie_eating_status();
            return true;
        }
    }
    return false;
}

/*
Check if zombie reach any plant or not.
Update zombies' moving status.
@return true if zombie reach any plant
*/
bool has_zombie_reached_any_plant(Zombie &zombie)
{
    if (has_zombie_reached_plant(zombie))
        return true;
    zombie.is_moving = true;
    zombie.change_zombie_eating_status();
    return false;
}

/*
Check if zombie can move or not:
    + If zombie go out background: false (because you have lose)
    But we don't need to do this because the has_player_lose will check it and end the game.
    + If zombie.is_moving is false: false (of course)
    + If zombie reached any element: has_zombie_reached_any_plant will decide it
*/
bool can_zombie_move(Zombie &zombie)
{
    if (!zombie.is_moving)
        return false;
    if (has_zombie_reached_any_plant(zombie))
        return false;
    return true;
}

/*
Update all zombies' moving status.
*/
void update_moving_status_for_zombies(vector<Zombie *> &zombies)
{
    for (auto &zombie : zombies)
    {
        if (zombie->is_charmed)
            continue; // charmed ignore plants; the brawl loop sets is_moving
        has_zombie_reached_any_plant(*zombie);
    }
}

/*
Move the zombie: For all zombie:
    If zombie can move, its location -= their speed.
    Note that a zombie can be slow if it's cold.
*/
void move_zombies(vector<Zombie *> &zombies)
{
    for (auto &zombie : zombies)
    {
        zombie->last_ground_dx = 0.0f;
        zombie->last_step_px = 0;
        // Damage-flash / charm-swirl clocks tick once per logic tick, even
        // while frozen or buttered (repvz mJustGotShotCounter decrements in
        // Zombie::Update; D2 moved this out of the render pass).
        zombie->update_logic_timers();
        // Ice-shroom hold: the zombie stands dead still for freeze_time ticks,
        // then keeps the ice-shroom chill afterwards (repvz HitIceTrap:
        // ApplyChill(true) = 2000cs, longer than the snow-pea 1000cs chill).
        // A fully frozen zombie does not animate either.
        if (zombie->freeze_time)
        {
            zombie->freeze_time--;
            if (zombie->freeze_time == 0)
                zombie->cold_time = ICE_SHROOM_COLD_TIME;
            continue;
        }
        // Kernel-pult butter: glued in place, no lingering chill afterwards.
        if (zombie->butter_time)
        {
            zombie->butter_time--;
            continue;
        }
        /*Walking speed this tick: the per-zombie roll in px/tick (raw mVelX,
        unchilled). Ground-driven walkers cover 0.47*raw per tick (retail
        UpdateAnimSpeed); phases without a _ground track move by raw veering
        x0.4 when chilled (CHILLED_SPEED_FACTOR). Subclass paces come via
        speed_px_per_tick(); the whack sprint ramp scales there.*/
        const bool cold = zombie->cold_time > 0;
        const float v_raw = zombie->speed_px_per_tick();
        const float v = cold ? v_raw * CHILLED_SPEED_FACTOR : v_raw;
        // subclass movement override (pole vault flight, newspaper gasp):
        // true means the tick's movement is fully handled; the animation
        // still advances below.
        const bool handled = zombie->on_move_tick();
        /*D2: the walk animation advances here, in LOGIC, and the doc's
        _ground track reports the ground actually covered (retail
        UpdateZombieWalking, Zombie.cpp:4126-4133: dx = ground.x(new anim
        time) - ground.x(old anim time), x mScaleZombie). The reanim rate is
        the retail formula, so the average pace is 0.47 * velX. States with
        no distance-driven walk segment (eating, one-shots, vault, swim,
        zamboni, bobsled, dolphin jump, digger tunnel) fall back to raw velX
        and a fixed anim rate. Nothing is computed in the draw pass, so the
        movement is identical with or without rendered frames.*/
        const bool walking = zombie->is_moving && v_raw > 0.0f;
        bool ground_driven = false;
        const float ground_dx =
            zombie->update_logic_animation(walking ? v_raw : 0.0f, cold, ground_driven);
        if (handled)
            continue;
        if (!walking)
            continue;
        /*Carry the fractional distance; whole pixels step x_location below.
        On ground-driven states the delta IS the distance covered (the seam
        tick holds at 0 and the stride is repaid across the loop, so the
        average is exactly 0.47 * velX). The carry drains even while blocked,
        so a zombie released from a bite resumes without any backlog lurch.*/
        zombie->walk_carry += ground_driven ? ground_dx : v;
        int step = (int)zombie->walk_carry;
        zombie->walk_carry -= step;
        zombie->last_ground_dx = ground_driven ? ground_dx : 0.0f;
        zombie->last_step_px = step;
        // Hypnotized zombies march right and ignore plants/grave rules; the
        // zombie-vs-zombie fight in update loop marks them stopped.
        if (zombie->is_charmed)
        {
            if (zombie->is_moving)
                zombie->x_location += step;
            continue;
        }
        if (step > 0 && can_zombie_move(*zombie))
            zombie->x_location -= step;
    }
}

/*
Display zombie of exactly status.
*/
void display_zombies(vector<Zombie *> &zombies, const int &_row)
{
    for (auto &zombie : zombies)
    {
        zombie->display(_row);
    }
}

void display_zombie_parts(vector<ZombiePart> &zombie_parts, const int &_row)
{
    for (int i = 0; i < (int)zombie_parts.size();)
    {
        if (zombie_parts[i].display(_row))
        {
            zombie_parts.erase(zombie_parts.begin() + i);
        }
        else
        {
            i++;
        }
    }
}