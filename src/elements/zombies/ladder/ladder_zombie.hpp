#pragma once
#include "../zombie_struct.hpp"

/*
Ladder Zombie (roof): carries a metal ladder that works like a screen-door
shield (bullets chew the ladder first; on the clone's 20:1 scale the ladder
soaks hits down to LADDER_BREAK_HP, then it's gone). At the first DEFENSIVE
plant (Wall-nut / Tall-nut / Pumpkin) it plants the ladder on that tile:
from then on the tile stops blocking zombies (Map Block::has_ladder), every
zombie climbs over the plant, and the ladder zombie itself walks on bare
handed. The ladder dies together with its plant (delete_plant clears it).
*/
class LadderZombie : public Zombie
{
public:
    ~LadderZombie() override;
    void init() override;
    void make_credit() override;
    bool on_move_tick() override;
    bool on_reach_plant(const int &col) override;
    bool can_eat_plant() const override;
    /*Carrying the ladder the zombie runs (PHASE_LADDER_CARRYING rolls
    RandRangeFloat(0.79, 0.81) px/cs in the original, repvz PickRandomSpeed
    Zombie.cpp:1206-1209; mid-band 0.80 here -> 43.6 px/s under
    HEAD_SPEED_SCALE); bare-handed it walks at the plain spawn roll.*/
    float speed_px_per_tick() const override
    {
        return has_ladder ? CS_VEL(0.80f) * HEAD_SPEED_SCALE : Zombie::speed_px_per_tick();
    }
    void determine_appearance(vector<ZombiePart> &) override;
    void add_zombie_die(vector<ZombiePart> &zombie_parts) override;
    using Zombie::Zombie;

protected:
    void render_zombie() override;

private:
    bool has_ladder = true;
    int place_ticks = 0; // anim_placeladder one-shot against the plant
    float anim_t = 0.0f;
    bool was_eating = false;
};
