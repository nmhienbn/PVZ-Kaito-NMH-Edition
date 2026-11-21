#pragma once
#include "../zombie_struct.hpp"

/*
Catapult Zombie (roof): drives in (zomboni-style vehicle: peas thud into the
cart), and once it crosses the column-7 line (repvz mPosX <= 650,
Zombie.cpp:1588) with ammo and a target it parks and lobs basketballs at the
left-most plant of its lane every ~3 s -- spiky tiles are off the menu and a
plant at the nose (<100 px left) is out of the arc. Four balls crack a plain
plant (75 dmg vs 300 hp in the original). The magazine holds CATAPULT_AMMO
balls; when it runs dry or the target dies, the cart drives on and crushes
any plant in its way.
*/
class CatapultZombie : public Zombie
{
public:
    ~CatapultZombie() override;
    void init() override;
    void make_credit() override;
    bool on_move_tick() override;
    bool on_reach_plant(const int &col) override;
    bool can_eat_plant() const override;
    void determine_appearance(vector<ZombiePart> &) override;
    void add_zombie_die(vector<ZombiePart> &zombie_parts) override;
    using Zombie::Zombie;

protected:
    void render_zombie() override;

private:
    enum DriveState
    {
        DRIVING,  // rolling in from the right, crushing plants in the way
        SHOOTING, // parked, lobbing basketballs
        CRUSHING  // out of ammo: drives over plants
    };
    DriveState state = DRIVING;
    int ammo;
    int shot_clk = 0;      // ticks to the next ball
    int shoot_anim = 0;    // anim_shoot one-shot after a lob
    int ball_fires = 0;    // lobs so far (alternating 2/1 damage per ball)
    // ball arc in flight (-1 = none): travels from the basket to the target
    int ball_ticks = -1;
    int ball_total = 1;
    int ball_damage = 2;
    int ball_from_x = 0, ball_to_x = 0;
    int ball_row = 0, ball_col = 0; // target tile
    float anim_t = 0.0f;
    DriveState anim_state = CRUSHING; // anim clock reset tracker

    int target_col() const; // left-most valid target column, -1 if none

public:
    // Test/debug: 0 = DRIVING, 1 = SHOOTING, 2 = CRUSHING.
    int debug_state() const { return state; }
    int debug_ammo() const { return ammo; }
};
