#pragma once
#include "../zombie_struct.hpp"

// Health on the clone's 20:1 scale, exact absolute ports (original: 500 /
// 270 default body; arm off below 2/3 of the body, Zombie.cpp:3892) --
// mirrors the "dancer"/"backup" entries of resources/dat/zombies.json.
const float DANCER_TOTAL_HEALTH = 25.0f;
const float DANCER_ARM_HEALTH = 100.0f / 6; // 333.3 orig ~ 16.67
const float BACKUP_TOTAL_HEALTH = 13.5f;
const float BACKUP_ARM_HEALTH = 9.0f;
// anim_armraise is 19 reanim frames ~= 95 game ticks of summons posing.
const int DANCER_RAISE_TICKS = 95;
// Once the formation broke, wait this long before calling the backups again.
const int DANCER_RESUMMON_CLK = 600;
// The dancer summons once it has stepped ~2 tiles onto the lawn.
const int DANCER_SUMMON_ENTER_X = 200;
// Dancer + backup walk pace: fixed 0.45 px/cs in the original (for
// DANCER/BACKUP_DANCER/POGO/FLAG, repvz PickRandomSpeed Zombie.cpp:1196-1200)
// -> 24.5 px/s under HEAD_SPEED_SCALE, brisk vs the plain 12.5-17.5 px/s.
const float DANCER_WALK_PX = CS_VEL(0.45f) * HEAD_SPEED_SCALE;

/*
Dancing Zombie (PvZ1 GOTY, disco look -- never the Jackson skin): walks in,
raises arms and calls four backup dancers, re-calling any that fell.
*/
class DancingZombie : public Zombie
{
public:
    ~DancingZombie();
    void init() override;
    void make_credit() override;
    bool on_move_tick() override;
    bool can_eat_plant() const override { return raise_left == 0; }
    float speed_px_per_tick() const override;
    void determine_appearance(vector<ZombiePart> &zombie_parts) override;
    void add_zombie_die(vector<ZombiePart> &zombie_parts) override;
    using Zombie::Zombie;

private:
    bool has_summoned = false;
    int raise_left = 0;
    int resummon_clk = DANCER_RESUMMON_CLK;
    void start_raise();
    void queue_missing_backups();
};

/*
Backup Dancer: own GOTY art (Zombie_backup.reanim, light-blue retro outfit;
the doc shares the disco track names). Walks/chews along with the formation.
*/
class BackupDancerZombie : public Zombie
{
public:
    ~BackupDancerZombie();
    void init() override;
    void make_credit() override;
    float speed_px_per_tick() const override;
    void determine_appearance(vector<ZombiePart> &zombie_parts) override;
    void add_zombie_die(vector<ZombiePart> &zombie_parts) override;
    using Zombie::Zombie;
};

/*Summoned backup dancers spill into this queue (the move loop must not
reallocate the zombie vector); handle_changes drains it between phases.*/
void queue_spawned_zombie(Zombie *z);
void drain_spawning_zombies(vector<Zombie *> &zombies);
