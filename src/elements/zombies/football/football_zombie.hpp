#pragma once
#include "../zombie_struct.hpp"

// Health stages on the clone's 20:1 effective scale, exact absolute values
// (original: 1400 helmet + 270 body = 1670 total, Zombie.cpp:285/181) --
// they mirror the "football" entry of resources/dat/zombies.json. The helmet
// has a single art stage so it simply thins out with no degrade; the left
// arm drops once the helm is gone and the body nears its 2/3 mark.
const float FOOTBALL_TOTAL_HEALTH = 83.5f;
const float FOOTBALL_ARM_HEALTH = 9.0f;
// Charge speed: see FootballZombie::pick_speed (repvz 0.66-0.68 px/cs).

/*
Football Zombie (PvZ1): very tough, charges in fast, no special trick.
*/
class FootballZombie : public Zombie
{
public:
    ~FootballZombie();
    void init() override;
    void make_credit() override;
    void pick_speed() override;
    float speed_px_per_tick() const override;
    void determine_appearance(vector<ZombiePart> &zombie_parts) override;
    void add_zombie_die(vector<ZombiePart> &zombie_parts) override;
    using Zombie::Zombie;
};
