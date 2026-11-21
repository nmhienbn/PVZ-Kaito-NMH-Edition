#pragma once
#include "../zombie_struct.hpp"

/*
Snorkel zombie (pool/fog water lanes): walks the right margin in from the
edge like the original's PHASE_SNORKEL_WALKING (repvz Zombie.cpp:1952-1977),
leaps into the water at the pool edge (mX in (700, 720], board offset +220 px
here) playing the snorkle doc's "anim_jumpinpool" once at 16 fps with the
splash beat at the 0.83 timed event, then swims underwater with the snorkel
("anim_swim") and only surfaces to chew a plant ("anim_eat"); while submerged,
straight shots pass over it (is_submerged). On this board the water lanes span
the whole row, so once it has surfaced on a lily-padded tile it re-submerges
when the plant is gone.

Run speed: the original rolls ZOMBIE_SNORKEL in the 0.66-0.68 px/cs
(66-68 px/s) bracket of PickRandomSpeed (Zombie.cpp:1122-1125) everywhere but
the leap -- shore walk-in (spawn roll), swim (re-rolled surfacing into
"anim_swim", Zombie_orig.cpp:1881) and the walk after climbing out alike;
the leap-in crawl alone is mVelX = 0.2 px/cs (Zombie.cpp:1956).
*/
class SnorkelZombie : public Zombie
{
public:
    ~SnorkelZombie();
    void init() override;
    void make_credit() override;
    bool on_move_tick() override;
    bool can_eat_plant() const override;
    bool is_submerged() const override;
    void pick_speed() override;
    float speed_px_per_tick() const override;
    void determine_appearance(vector<ZombiePart> &zombie_parts) override;
    void add_zombie_die(vector<ZombiePart> &zombie_parts) override;
    using Zombie::Zombie;

    // Test hooks: still on the way in (before the swim starts); current phase
    // for detailed assertions (SnorkelState order).
    bool on_shore() const { return state == WALK_SHORE || state == INTO_POOL; }
    int phase_for_test() const { return (int)state; } // SnorkelState order
    int phase_ticks_for_test() const { return into_pool_ticks; }

private:
    enum SnorkelState
    {
        WALK_SHORE, // shore strip: brisk walk-in (0.66-0.68 px/cs band)
        INTO_POOL,  // leaping into the water (anim_jumpinpool one-shot)
        IN_POOL     // swimming with the snorkel
    };
    SnorkelState state = WALK_SHORE;
    int into_pool_ticks = 0; // ticks elapsed in the INTO_POOL leap
    bool armless = false;
};
