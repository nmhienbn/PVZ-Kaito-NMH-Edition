#pragma once
#include "../zombie_struct.hpp"

/*
Digger zombie (PvZ1 4-6+): appears at the right edge already underground
(anim_dig; most body parts are empty in the data by design -- only the
hardhat/drill/dirt show), tunneling all the way to the leftmost column,
immune to plants, peas and mowers (is_underground()). At the left edge it
surfaces (anim_drill + anim_landing), stands dizzy for a moment (anim_dizzy),
then turns around and walks RIGHT, munching the lawn from the back. It never
heads back to the house: past the right screen edge it simply despawns
(pending_removal). The right-walk reuses the hypnotized pattern (x += step,
plants eaten with the same block-overlap rule).

Magnet-shroom (lose_pickaxe, repvz Zombie.cpp DiggerLoseAxe +
UpdateZombieDigger's *_WITHOUT_AXE phases): yanking the pickaxe mid-tunnel
stalls the digger underground for ~2 s, then it rises in place and keeps
walking LEFT as an ordinary zombie (no dizzy beat, no turnaround).
*/
class DiggerZombie : public Zombie
{
public:
    ~DiggerZombie();
    void init() override;
    void make_credit() override;
    bool on_move_tick() override;
    bool on_reach_plant(const int &col) override;
    bool can_eat_plant() const override;
    bool is_underground() const override;
    /*Walked off the right edge of the screen: gone without a fight.*/
    bool despawn_ready() const override { return walked_off; }
    void determine_appearance(vector<ZombiePart> &zombie_parts) override;
    void add_zombie_die(vector<ZombiePart> &zombie_parts) override;
    /*Burned without the pickaxe: the axe-less charred crumble
    (repvz Zombie.cpp:8865-8869 anim_crumble_noaxe).*/
    int charred_kind() const override;
    /*Magnet hook: yank the pickaxe (repvz DiggerLoseAxe). Mid-tunnel the
    digger stalls, then rises in place and walks left without the axe; on
    the surface it simply never digs in.*/
    void lose_pickaxe();
    /*The pickaxe is still there to steal (magnet eligibility).*/
    bool has_pickaxe() const { return pickaxe; }
    /*Stealing is only possible before the rise (matches the original's
    tunnel-pause branch) -- surfaced diggers finished using the axe.*/
    bool can_lose_pickaxe() const;
    using Zombie::Zombie;

protected:
    void render_zombie() override;

private:
    enum TunnelState
    {
        WALK_IN,      // surface entry, heading left like any zombie
        TUNNELING,    // underground run to the leftmost column
        TUNNEL_STALL, // axe stolen mid-tunnel: brief underground surprise pause
        RISING,       // drill breaks the surface (altitude arc in the original)
        DIZZY,        // stands still, catching its breath
        WALK_BACK,    // turned around: walks right, eats from behind
        WALK_FORWARD  // axe lost: keeps walking left like an ordinary zombie
    };
    TunnelState state = WALK_IN;
    int state_ticks = 0;      // TUNNEL_STALL/RISING/DIZZY countdown
    bool pickaxe = true;      // magnet can yank it (see lose_pickaxe)
    bool walked_off = false;  // past the right screen edge (swept next tick)
    const char *render_segment() const; // current special segment (nullptr = normal sheets)
};
