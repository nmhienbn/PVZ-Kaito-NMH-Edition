#pragma once
#include "../zombie_struct.hpp"

/*
Imp (roof/Gargantuar): tiny, fast, frail. Normally only enters the lawn by
being thrown by a gargantuar (roster weight 0 in zombies.json): spawn_thrown
launches it in a parabola from the gargantuar's back ~2.5 tiles ahead, then
it lands and walks on at the plain per-zombie roll. Also constructible as a
plain walker.
*/
class ImpZombie : public Zombie
{
public:
    ~ImpZombie() override;
    void init() override;
    void make_credit() override;
    bool on_move_tick() override;
    bool on_reach_plant(const int &col) override;
    bool can_eat_plant() const override;
    void determine_appearance(vector<ZombiePart> &) override;
    void add_zombie_die(vector<ZombiePart> &zombie_parts) override;
    using Zombie::Zombie;

    // Thrown spawn: fly from (x_location) to landing_x over IMP_FLIGHT_TICKS.
    void spawn_thrown(const int &landing_x);

    /*repvz Zombie.cpp:1424 (the falling-height rect): while airborne the box
    drops its headroom (36, 17, 42, 115); standing imps use the default box.*/
    void zrect_off(int &x, int &y, int &w, int &h) const override;

protected:
    void render_zombie() override;

private:
    enum
    {
        WALKING,
        THROWN,
        LANDING
    } state = WALKING;
    int fly_ticks = 0;
    int fly_total = 1;
    int fly_from_x = 0, fly_to_x = 0;
    float anim_t = 0.0f;
};
