#pragma once
#include "../zombie_struct.hpp"

/*
Gargantuar (roof): huge HP pool (zombies.json 150 hits = the original's 3000
body points, Zombie.cpp:358), it SMASHES plants flat with its weapon instead
of eating (one slam kills the tile, incl. pumpkin contents). Below half
health it hurls its imp to the anchored landing spot (x ~447 -- around tile
2, vs the original's mPosX - 360 formula; shorter over the roof, never when
the gargantuar itself is already at the doorstep) and keeps stomping. Plays
gargantuar_thump on every smash and gargantudeath when it finally goes down.
*/
class GargantuarZombie : public Zombie
{
public:
    ~GargantuarZombie() override;
    void init() override;
    void make_credit() override;
    bool on_move_tick() override;
    bool on_reach_plant(const int &col) override;
    bool can_eat_plant() const override;
    // Same pace as a plain zombie (base speed_px_per_tick covers it).
    void determine_appearance(vector<ZombiePart> &) override;
    void add_zombie_die(vector<ZombiePart> &zombie_parts) override;
    using Zombie::Zombie;

protected:
    void render_zombie() override;

private:
    enum
    {
        WALKING,
        SMASHING,
        THROWING
    } state = WALKING;
    int smash_ticks = 0;   // anim_smash one-shot; the hit lands mid-swing
    int smash_col = -1;    // tile being smashed
    bool smash_hit_done = false;
    int throw_ticks = 0;   // anim_throw one-shot
    bool imp_thrown = false;
    float anim_t = 0.0f;
    float anim_rate = 0.2f; // ticks scaled so one-shots fit their window
};
