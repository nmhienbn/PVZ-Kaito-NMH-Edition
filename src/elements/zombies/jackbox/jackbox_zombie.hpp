#pragma once
#include "../zombie_struct.hpp"
#include <unordered_set>

/*
Jack-in-the-box zombie (PvZ1 4-x): walks in cranking the music box. After a
random walking distance (the original arms the popper at 450+Rand(300) px,
1/20 chance it pops 3x earlier) the box pops: anim_pop plays while the clown
surprise sounds, then the box explodes -- the blast kills every plant in the
8 tiles around it AND burns the zombies inside its 90-px blast circle, one
row up/down (repvz Zombie.cpp:2070-2086 + KillAllZombiesInRadius); the popper
dies with it (DieNoLoot: no corpse, no head pop). Getting shot dead before
the pop cancels the whole thing (plain death).
Magnet-shroom can steal the box (steal_jackbox): no more pop, plain body.

Own per-tick bookkeeping for the box music loop and the fuse lives in
on_move_tick (observer mode): it watches the x delta the base stepper just
produced; only POPPING fully handles the tick.
*/
class JackboxZombie : public Zombie
{
public:
    ~JackboxZombie();
    void init() override;
    void make_credit() override;
    bool on_move_tick() override;
    bool on_reach_plant(const int &col) override;
    bool can_eat_plant() const override;
    void determine_appearance(vector<ZombiePart> &zombie_parts) override;
    void add_zombie_die(vector<ZombiePart> &zombie_parts) override;
    /*The box blew up with the zombie inside: no corpse, swept next tick.*/
    bool despawn_ready() const override { return blown_up; }
    /*Magnet-shroom pulls the box away: the fuse is gone with it.*/
    void steal_jackbox();
    /*Still cranking the box (magnet can steal it only in this phase).*/
    bool has_box() const { return state == RUNNING; }
    /*Fast-class speed: box in hand the clown hurries (original
    RandRangeFloat(0.66, 0.68) px/cs vs the plain band -- repvz
    PickRandomSpeed, Zombie.cpp:1201-1205; mid-band 0.67 here -> 36.5 px/s
    under HEAD_SPEED_SCALE); once the box is gone it shuffles at the plain
    spawn roll.*/
    float speed_px_per_tick() const override
    {
        return state == STOLEN ? Zombie::speed_px_per_tick() : CS_VEL(0.67f) * HEAD_SPEED_SCALE;
    }
    using Zombie::Zombie;

protected:
    void render_zombie() override;

private:

    enum JackState
    {
        RUNNING, // box in hands, fuse ticking (distance-based)
        POPPING, // anim_pop one-shot; explodes on the last beat
        STOLEN   // box magneted off: ordinary zombie, box art hidden
    };
    JackState state = RUNNING;
    int fuse_distance = 500; // px to walk before the pop
    int last_x = -1;         // previous tick's x (fuse observer)
    int pop_ticks = 0;       // POPPING countdown
    int music_clk = 0;       // jackinthebox.ogg loop bookkeeping
    bool blown_up = false;   // explosion resolved; the sweeper erases the body
    void explode(vector<ZombiePart> &zombie_parts);
};

/*Blast victims of a popping jackbox are queued during explode() (which runs
inside move_zombies' iteration) and erased here — called once per frame from
update_zombies_status before the status pass.*/
void drain_jack_explosion_kills(vector<Zombie *> &zombies);
