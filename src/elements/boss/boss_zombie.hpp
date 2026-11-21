#pragma once
#include "../zombies/zombie_struct.hpp"

/*
Zomboss (level 5-10, special "boss"): the Zombot stands at the right edge of
the roof and never walks. The boss_fight controller drives its phases
(idle / stomp-spawn / boulder attack / RV throw / head-down window); this
class is the on-lawn body: it takes bullet hits (any lane, the body is huge),
never eats, and reports its death to the controller. Plus BoulderZombie, the
fire/ice boulder it rolls down a lane.

Approximations vs the original (documented; numbers live in the .cpp):
    * HP 2000 pea-units = the original's 40000 HP (Zombie.cpp:692).
    * Damage gates on the head-down window like the original (repvz
      Zombie.cpp EffectedByDamage): while not vulnerable the boss reports
      is_underground() so bullets, lobbed shots and plant targeting pass it
      by. Ice-shroom still chips 1 HP per use (that path hits every zombie
      unconditionally and is out of this folder's reach).
    * For the vulnerable window the whole body lunges to mid-lawn (the
      original lowers only the head; here moving the anchor is what lets
      the clone's nearest-first lobbed targeting reach the head at all).
    * A rolling roof cleaner on the Zombot's lane makes it yank the head
      back up: clone mowers delete any zombie they touch, which would
      otherwise silently insta-win the fight. The original's 5-10 also has
      roof cleaners (repvz Board.cpp InitLawnMowers does not exclude the
      final boss level), so mowers stay on the lawn.
    * Foot stomp (repvz BossStompAttack/BossStompContact,
      Zombie.cpp:9980-10043): every 55-65 s once damage stage 2 is reached
      the boss picks a 2-row band that holds a plant in a column >= 5 and
      flattens every plant of those two rows in columns 5-8. Unlocked at
      the stage-2 RV throw like the original (stomp counter is only armed
      at BossRVLanding).
    * Bungee raid (repvz BossBungeeAttack/BossBungeeSpawn,
      Zombie.cpp:10046-10070): on entering damage stage 1 (hp < 4/5 max,
      Zombie.cpp:5836-5851) and then every 40-50 s, three bungees dive
      for one tile each in three ADJACENT columns starting at col 0-2
      (mTargetCol+i), stealing whatever plant the rope lands on.
    * The boulder is INVULNERABLE: in the original it is a bare reanim,
      no health, no hitbox (Zombie.cpp:10271-10335); only the counters
      remove it -- an ice-shroom blast destroys a FIRE ball outright,
      anywhere on the lawn (Plant.cpp IceZombies, :4282-4286), and a
      jalapeno burning the ball's own row destroys an ICE ball
      (Plant.cpp BurnRow, :4290-4318). Here is_underground() makes every
      bullet / fume / cherry / doom / jalapeno-damage path pass over it
      (its zombie-slot "health" is vestigial), and the controller's
      per-tick sweeps in boss_fight.cpp apply both counters. It never
      rolls down the Zombot's own lane (out of the lowered head; also
      keeps the out-of-folder jalapeno whole-row burn from insta-killing
      the boss that way), and it SQUISHES the roof cleaner in its lane
      (orig: SquishMower, Zombie.cpp:10291-10299) instead of despawning
      before it; it rolls off the lawn past the cleaner, still well
      short of the house-loss line.
    * Jalapeno row-burn detection is presence-based (a pepper that stood
      last tick and is gone now has fired): a fused pepper crushed by the
      RV/stomp or shoveled reads as a burn. Documented approximation; the
      alternative needs a hook inside jalapeno.cpp (out of this folder).
*/
class BossZombie : public Zombie
{
public:
    enum Phase
    {
        PH_IDLE,
        PH_HEAD_ENTER,  // lowering (hydraulics)
        PH_HEAD_DOWN,   // head level with the lawn (vulnerable window)
        PH_HEAD_LEAVE,  // rising back
        PH_STOMP,       // spawn zombies
        PH_BALL,        // spit a fire/ice boulder
        PH_RV,          // hurl the RV
        PH_FOOT_STOMP,  // foot stomp: crush 2 rows x cols 5-8
        PH_BUNGEE       // send in the 3-bungee steal raid
    };

    ~BossZombie() override;
    void init() override;
    bool on_move_tick() override;
    bool on_reach_plant(const int &col) override;
    bool can_eat_plant() const override;
    /*Vulnerable only with the head (mostly) down; bullets/lobbed shots/plant
    targeting treat an invulnerable boss like a tunneler (pass over / not a
    target). The controller lifts the flag for the second half of the
    lowering animation too, matching the original's EffectedByDamage
    (anim_time >= 0.5 on head_enter, <= 0.5 on head_leave).*/
    bool vulnerable() const { return vuln; }
    void set_vulnerable(bool v) { vuln = v; }
    bool is_underground() const override { return !vulnerable(); }
    void determine_appearance(vector<ZombiePart> &) override;
    void add_zombie_die(vector<ZombiePart> &zombie_parts) override;
    using Zombie::Zombie;

    // One-shot phase switch; the controller owns the schedule.
    void set_phase(Phase p);
    Phase get_phase() const { return phase; }
    int max_health() const { return max_hp; }
    // Row-pair index (0-3) of the foot stomp target; set by the controller
    // before PH_FOOT_STOMP so the render picks anim_stomp_1..4.
    int stomp_row_pair = 0;

protected:
    void render_zombie() override;

private:
    Phase phase = PH_IDLE;
    bool vuln = false;
    int max_hp;
    float anim_t = 0.0f;
    float anim_rate = 0.2f;
};

/*
Fire/ice boulder rolled by Zomboss: barrels down one lane, flattening every
plant tile and the roof cleaner it crosses. Untouchable by bullets and every
damage-based instant-kill (is_underground): the original ball is a pure
reanim with no health at all, and only its element counter destroys it --
ice-shroom anywhere vs a FIRE ball (Plant.cpp:4282-4286), jalapeno on the
ball's row vs an ICE ball (Plant.cpp:4313-4318). Both counters fire from the
controller's sweep in boss_fight.cpp. It despawns as it leaves the lawn
(the original rolls off-screen), always short of the house-loss line.
*/
class BoulderZombie : public Zombie
{
public:
    explicit BoulderZombie(bool ice);
    ~BoulderZombie() override;
    void init() override;
    bool on_move_tick() override;
    bool on_reach_plant(const int &col) override;
    bool can_eat_plant() const override;
    bool despawn_ready() const override;
    void determine_appearance(vector<ZombiePart> &) override;
    void add_zombie_die(vector<ZombiePart> &zombie_parts) override;
    bool is_ice() const { return ice; }
    /*The ball takes no hits: peas/lobbed shots fly through, instant-kills
    look straight past it, lawn mowers never trigger on it (the original
    fireball carries no health and no zombie hitbox at all). The only ways
    out are the counter sweeps and the lawn-edge despawn.*/
    bool is_underground() const override { return true; }

protected:
    void render_zombie() override;

private:
    bool ice;
    bool gone = false;
    float anim_t = 0.0f;
};
