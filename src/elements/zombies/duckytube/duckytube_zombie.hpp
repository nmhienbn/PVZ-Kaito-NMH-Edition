#pragma once
#include "../zombie_struct.hpp"

/*
Ducky-tube zombie (pool water lanes): the basic zombie in its inflatable
duck tube. Gameplay is the plain zombie's -- it just lives in a water lane
and rides the tube. Rendering goes through the plain reanim doc with the
duckytube/whitewater tracks revealed (reanimz Spec::ducky, keyed off the
DUCKYTUBE_* state slots).

Two entry points:
  + roster type "duckytube" (level 3-1 debut): spawned directly as this
    class, HP from the "duckytube" entry of zombies.json.
  + water wrap (wrap_in_duckytube in zombie_type.cpp): a normal/cone/bucket
    zombie assigned a water lane keeps its class and stats (armor tier rides
    along); only its walk/eat state slots swap to the DUCKYTUBE_* directories
    so the reanim unhides the tube.
*/
class DuckyTubeZombie : public Zombie
{
public:
    ~DuckyTubeZombie() override;
    void init() override;
    void make_credit() override;
    bool on_move_tick() override;
    void determine_appearance(vector<ZombiePart> &zombie_parts) override;
    void add_zombie_die(vector<ZombiePart> &zombie_parts) override;
    using Zombie::Zombie;

private:
    bool armless = false;      // arm popped off at critical health
    bool entered_water = false; // splash sound played (first tick on the lawn)
};
