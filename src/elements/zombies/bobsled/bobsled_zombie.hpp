#pragma once
#include "../zombie_struct.hpp"

/*
Bobsled team member: 4 of them spawn together at the end of a lane
(materialize_wave_plans packs the team, same row, staggered x) -- the
original gates the team on an ice trail being present; here the roster
simply pairs bobsled with zomboni from wave 5 on (zombies.json level
data), so the sled gang follows the trail the zomboni just laid.

While on the ice trail (ZomboniZombie::ice_front_x, x right of the ice
front) the member glides along at sled speed (60 px/s, repvz Zombie.cpp:502);
off the ice it gets up and walks like a plain zombie. The sled/frame art boarding sequence
(anim_push -> anim_jump) is not staged -- the team walks/runs in
BOBSLED_WALK art, documented approximation.
*/
class BobsledZombie : public Zombie
{
public:
    ~BobsledZombie() override;
    void init() override;
    void make_credit() override;
    float speed_px_per_tick() const override;
    void determine_appearance(vector<ZombiePart> &zombie_parts) override;
    void add_zombie_die(vector<ZombiePart> &zombie_parts) override;
    using Zombie::Zombie;
    /*repvz IsBobsledTeamWithSled: gliding on the ice trail == still in the
    sled (the clone folds the boarding frames into the on-ice glide).*/
    bool is_riding_vehicle() const override { return on_ice(); }
    /*repvz Zombie.cpp:496 / BobsledCrash d2647: in the sled the hitbox spans
    the whole craft (-50,0,275,115); after the ride it drops back to the
    default body box.*/
    void zrect_off(int &x, int &y, int &w, int &h) const override;

private:
    bool on_ice() const;
    bool armless = false;
};
