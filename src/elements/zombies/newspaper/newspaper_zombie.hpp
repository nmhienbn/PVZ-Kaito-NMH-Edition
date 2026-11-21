#pragma once
#include "../zombie_struct.hpp"

// Health stages on the clone's 20:1 effective scale, exact absolute values
// (original: 150 paper shield + 270 body = 420 total; paper crack art at
// shield < 100/< 50 absorbed (repvz GetShieldDamageIndex, Zombie.cpp:5871),
// paper break at body-only, arm off at body < 180) -- they mirror the
// "newspaper" entry of resources/dat/zombies.json.
const float NEWSPAPER_TOTAL_HEALTH = 21.0f;
const float NEWSPAPER_PAPER_DMG2_HEALTH = 18.5f;
const float NEWSPAPER_PAPER_DMG3_HEALTH = 16.0f;
const float NEWSPAPER_PAPER_BREAK_HEALTH = 13.5f; // paper destroyed (rage starts)
const float NEWSPAPER_ARM_HEALTH = 9.0f;
// anim_gasp is 10 reanim frames; the stop-and-gasp beat is ~1.5 s in the
// original (100 tps) -> 90 ticks here before the rage sprint.
const int NEWSPAPER_GASP_TICKS = 90;
// Rage speed: mad newspaper runs at RandRangeFloat(0.89, 0.91) px/cs in the
// original (PHASE_NEWSPAPER_MAD branch of PickRandomSpeed, repvz
// Zombie.cpp:1210-1214; mid-band 0.90 here) -> 49.1 px/s under
// HEAD_SPEED_SCALE, the fastest walker.
const float NEWSPAPER_RAGE_PX = CS_VEL(0.90f) * HEAD_SPEED_SCALE;

/*
Newspaper Zombie (PvZ1): the newspaper is a front shield; once it is torn
off the zombie stops for a beat (anim_gasp + shred SFX), then rages forward
at double speed with the mad head.
*/
class NewspaperZombie : public Zombie
{
public:
    ~NewspaperZombie();
    void init() override;
    void make_credit() override;
    bool on_move_tick() override;
    bool can_eat_plant() const override;
    float speed_px_per_tick() const override;
    void determine_appearance(vector<ZombiePart> &zombie_parts) override;
    void add_zombie_die(vector<ZombiePart> &zombie_parts) override;
    using Zombie::Zombie;

    bool papered() const { return has_paper; }

private:
    bool has_paper = true;
    int rage_pause = 0; // stop-and-gasp ticks after the paper is destroyed
};
