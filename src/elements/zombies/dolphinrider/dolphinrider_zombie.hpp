#pragma once
#include "../zombie_struct.hpp"

/*
Dolphin rider (pool water lanes): sprints the right margin with the dolphin
under its arm at the original's 0.89-0.91 px/cs roll (repvz
UpdateZombieDolphinRider's PHASE_DOLPHIN_WALKING, Zombie.cpp:1833-1839, with
ZombieInitialize's PickRandomSpeed at :344-348; "anim_walkdolphin" of the 2Pak
doc), leaps into the water at the pool edge (the original's mX in (700, 720]
window, board offset +220 px here) playing "anim_jumpinpool" once at 16 fps
with the splash beat at the 0.56 timed event, lurches 70 px in and rides the
dolphin fast ("anim_ride", same 89-91 px/s band), then vaults the first plant
in its way: "anim_dolphinjump" once at 10 fps while the body grinds 0.5 px/cs
until the 0.49 splash event, hangs for the rest of the take, and snaps 94 px
past the plant on landing (Zombie.cpp:1884-1933) -- the dolphin is gone
afterwards and it wades on at near-plain pace (the original's
PHASE_DOLPHIN_WALKING_IN_POOL re-roll pins 0.3 px/cs, Zombie.cpp:1169-1172;
the plain roll here covers it). Eats plants like any zombie once swimming. A
Tall-nut blocks the jump: bonk, the dolphin is gone, the rider wades on in
front of it (repvz Zombie.cpp:1902-1916).
*/
class DolphinRiderZombie : public Zombie
{
public:
    ~DolphinRiderZombie();
    void init() override;
    void make_credit() override;
    bool on_move_tick() override;
    bool on_reach_plant(const int &col) override;
    bool can_eat_plant() const override;
    void pick_speed() override;
    /*repvz PHASE_DOLPHIN_IN_JUMP (airborne) / PHASE_DOLPHIN_RIDING (mounted).*/
    bool is_vaulting() const override { return state == JUMPING; }
    bool is_riding_vehicle() const override { return state == RIDING; }
    /*repvz phase attack boxes (Zombie.cpp:1857 ride, :8406 jump, :1927 wade).*/
    void attack_x_span(int &left, int &width) const override;
    float speed_px_per_tick() const override;
    void determine_appearance(vector<ZombiePart> &zombie_parts) override;
    void add_zombie_die(vector<ZombiePart> &zombie_parts) override;
    using Zombie::Zombie;

    // Test hooks: still on the way in (before the dolphin takes over);
    // current phase for detailed assertions (DolphinState order).
    bool on_shore() const { return state == WALK_IN || state == INTO_POOL; }
    int phase_for_test() const { return (int)state; } // DolphinState order
    int phase_ticks_for_test() const
    {
        return state == INTO_POOL ? into_pool_ticks : jump_ticks;
    }

private:
    enum DolphinState
    {
        WALK_IN,   // shore strip: walking in, dolphin carried
        INTO_POOL, // leaping into the water (anim_jumpinpool one-shot)
        RIDING,    // mounted, fast
        JUMPING,   // mid-air over the first plant
        SWIMMING   // dolphin gone, wading at plain pace
    };
    DolphinState state = WALK_IN;
    int into_pool_ticks = 0; // ticks elapsed in the INTO_POOL leap
    int jump_ticks = 0;      // ticks elapsed since the vault started
    float jump_carry = 0.0f; // fractional px of the mid-jump grind
    bool armless = false;

    // Pool edge reached: start the leap-in (end-of-leap switches to riding).
    void enter_pool();
};
