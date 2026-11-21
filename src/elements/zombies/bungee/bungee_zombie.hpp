#pragma once
#include "../zombie_struct.hpp"

/*
Bungee Zombie (roof): drops out of the sky over a lawn tile, hangs there for
3 s (repvz mPhaseCounter = 300 cs), then either
    * STEAL (wave bungee): yanks the plant under it off the lawn and flies
      away; the target is a weighted grid pick (planted tile 10000:1 over
      empty ones, no double-booking a tile, the last sun-maker is spared)
      and with nothing valid it simply rises back empty-handed, or
    * DELIVER (Bungee Blitz 5-5 waves / roof final-wave ambush): drops a
      carried zombie onto the tile and flies away.
It hangs high off the ground; peas can hit it while it hangs (pea x-overlap
reaches it), but once ascending it is out of danger. When it flies off it
never "dies", so despawn_ready() asks the update loop to erase it.
*/
class BungeeZombie : public Zombie
{
public:
    ~BungeeZombie() override;
    void init() override;
    bool on_move_tick() override;
    bool on_reach_plant(const int &col) override;
    bool can_eat_plant() const override;
    bool despawn_ready() const override;
    void determine_appearance(vector<ZombiePart> &) override;
    void add_zombie_die(vector<ZombiePart> &zombie_parts) override;
    using Zombie::Zombie;

    // Blitz override: hang over a specific tile delivering `carry` (-1: just
    // leave after hanging; used cells picked by init's steal logic).
    void force_target(const int &row, const int &col, const int &carry);
    // Test/debug: the tile the rope currently aims at / drop payload armed.
    int debug_target_col() const { return target_col; }
    bool debug_carrying() const { return carry_type >= 0; }
    int debug_phase() const { return (int)phase; }
    int debug_hang_dy() const { return hang_dy; }

protected:
    void render_zombie() override;

private:
    // decide steal vs deliver with the live lawn once the rope is down
    void resolve_target();

private:
    enum Phase
    {
        DESCEND,
        HANG,
        ACT, // grab the plant or release the carried zombie
        ASCEND,
        GONE
    };
    Phase phase = DESCEND;
    int target_col = 4;
    int carry_type = -1;  // zombie type delivered on DELIVER mode (-1: steal)
    bool forced = false;  // force_target called: don't re-resolve at hang time
    int hang_dy = -420;   // rope top off-screen; 0 = hanging over the tile
    int hang_ticks = 0;
    float anim_t = 0.0f;
    Phase anim_phase = GONE; // anim clock reset tracker
};
