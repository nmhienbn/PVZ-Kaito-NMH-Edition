#pragma once
#include "../zombie_struct.hpp"

/*
Pogo Zombie (roof): bounces along on its pogo stick and hops over EVERY
plant instead of eating it (steady, fast pace). A Tall-nut stops it cold:
blocked mid-hop, it stands on the stick and chews through the wall of nuts.
The magnet-shroom hook is drop_pogo(): losing the stick turns it into a
plain walker.
*/
class PogoZombie : public Zombie
{
public:
    ~PogoZombie() override;
    void init() override;
    void make_credit() override;
    bool on_move_tick() override;
    bool on_reach_plant(const int &col) override;
    bool can_eat_plant() const override;
    float speed_px_per_tick() const override;
    void determine_appearance(vector<ZombiePart> &) override;
    void add_zombie_die(vector<ZombiePart> &zombie_parts) override;
    using Zombie::Zombie;

    // Magnet hook: yank the stick away. Safe to call only when has_pogo.
    void drop_pogo();
    bool has_stick() const { return has_pogo; }

protected:
    void render_zombie() override;

private:
    bool has_pogo = true;
    bool sound_played = false; // spawn "boing" (delayed: init runs for credits too)
    // one-plant hop (travel baked over the pogo bounce)
    int hop_ticks = 0;
    int hop_start_x = 0;
    int hop_travel = 0;
    // own animation clock for the free-segment render
    float anim_t = 0.0f;
    bool was_eating = false;
};
