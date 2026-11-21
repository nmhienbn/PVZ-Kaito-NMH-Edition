#pragma once
#include "../zombie_struct.hpp"

/*
Balloon zombie (PvZ1 4-3+): flies in over the lawn, above the plants. While
flying (is_flying()) it ignores plant blocking and ground peas pass under it
(BulletVsZombie skips flyers); only cactus spikes / Blover wind call
pop_balloon(). Popping drops it exactly where it was flying -- over the pool
it sinks (drowns) at once; on land it walks and eats like any zombie. If it
reaches the house still flying it takes the brain (the regular lose check).
*/
class BalloonZombie : public Zombie
{
public:
    ~BalloonZombie();
    void init() override;
    void make_credit() override;
    bool on_move_tick() override;
    bool on_reach_plant(const int &col) override;
    bool can_eat_plant() const override;
    bool is_flying() const override;
    void pop_balloon(vector<ZombiePart> &zombie_parts) override;
    /*The rect keeps its (36, 30) offsets but rides the body's altitude
    (repvz GetZombieRect offsets by GetDrawPos mBodyY = -mAltitude,
    Zombie.cpp:8391/6028), so the flying box sits where the body is drawn.*/
    void zrect_off(int &x, int &y, int &w, int &h) const override;
    /*Balloon popped over the pool: the zombie drowned without a fight.*/
    bool despawn_ready() const override { return landed_in_water; }
    void determine_appearance(vector<ZombiePart> &zombie_parts) override;
    void add_zombie_die(vector<ZombiePart> &zombie_parts) override;
    using Zombie::Zombie;

protected:
    void render_zombie() override;

private:
    enum BalloonState
    {
        FLYING,  // balloon up, immune to ground peas
        POPPING, // balloon popped: fall one-shot, then walk
        WALKING  // on the ground, plain-zombie rules
    };
    BalloonState state = FLYING;
    int pop_ticks = 0;
    bool inflate_played = false;
    bool landed_in_water = false; // popped above the pool: drowned
    int fly_offset() const; // px the body floats above its ground position
};
