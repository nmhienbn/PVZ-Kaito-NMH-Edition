#pragma once
#include "../zombie_struct.hpp"

/*
Zombie Yeti (PvZ1 rare cameo; the adventure 4-10 replay zombie): shuffles
onto the lawn like a slow walker (0.4 px/cs, fixed -- repvz PickRandomSpeed,
Zombie.cpp:1191-1195), and after RandRangeInt(1500, 2000) centiseconds
(= 900..1200 game ticks here; Zombie.cpp:223 mPhaseCounter on
ZombieInitialize) it stops advancing and RUNS AWAY to the right at 0.8 px/cs
(PHASE_YETI_RUNNING, UpdateYeti Zombie.cpp:4046-4055 + speed 1188-1190),
passing plants without eating them. Past the right screen edge it despawns
(like the turned digger). Diamonds on kill (DropLoot, Zombie.cpp:7362-7368):
one diamond pickup pops out on death (add_zombie_die hook below), paid as
sun through the coin-sun pipeline since this clone has no money HUD.
Skins: own reanim doc (SKIN_YETI); no armor tiers (single 68-hp pool,
zombies.json "yeti": 1350 raw / 20).
*/
class YetiZombie : public Zombie
{
public:
    ~YetiZombie();
    void init() override;
    void make_credit() override;
    bool on_move_tick() override;
    /*Fleeing yetis gait right past a planted tile without stopping to chew.*/
    bool on_reach_plant(const int &col) override;
    bool can_eat_plant() const override { return !fleeing; }
    /*Ran off the right edge of the screen: gone without a fight (the update
    loop erases despawn-ready zombies).*/
    bool despawn_ready() const override { return fled_off_screen; }
    float speed_px_per_tick() const override;
    void add_zombie_die(vector<ZombiePart> &zombie_parts) override;
    bool is_fleeing() const { return fleeing; }
    using Zombie::Zombie;

private:
    int flee_clk = 0;       // ticks until the yeti bolts (900..1200)
    bool fleeing = false;   // PHASE_YETI_RUNNING: marching back out, right
    bool fled_off_screen = false;
};
