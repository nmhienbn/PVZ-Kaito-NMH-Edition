#pragma once
#include "../zombie_struct.hpp"

/*
Pole Vaulting Zombie (PvZ1): runs in fast with the pole, vaults over the
first plant it runs into, drops the pole and walks like a plain zombie.
The vault (anim_jump) bakes the forward flight into the reanim; the logical
box is slid along by poleVaultTravelPx() over VAULT_TICKS and the state can
never vault twice. A Tall-nut is the one exception: the vault bonks against
it (repvz Zombie.cpp:1892-1903), the zombie keeps the pole but walks on at
normal pace and chews the tall-nut down.
*/
class PoleVaultingZombie : public Zombie
{
public:
    ~PoleVaultingZombie();
    void init() override;
    void make_credit() override;
    bool on_move_tick() override;
    bool on_reach_plant(const int &col) override;
    bool can_eat_plant() const override;
    /*repvz PHASE_POLEVAULTER_IN_VAULT: airborne mid-vault.*/
    bool is_vaulting() const override { return state == VAULTING; }
    /*repvz attack-box swaps (Zombie.cpp:335 pre-vault, :8406 in-vault,
    :1797 post-vault) -- the pole tip leads while running; after the drop the
    body box (-ish) is the mouth again.*/
    void attack_x_span(int &left, int &width) const override;
    float speed_px_per_tick() const override;
    void determine_appearance(vector<ZombiePart> &zombie_parts) override;
    void add_zombie_die(vector<ZombiePart> &zombie_parts) override;
    using Zombie::Zombie;

private:
    enum PoleState
    {
        RUNNING,  // pole in hand, fast
        VAULTING, // mid-air over the plant
        WALKING   // pole dropped, plain-zombie speed
    };
    PoleState state = RUNNING;
    int vault_ticks = 0;
    int vault_start_x = 0;
    int vault_travel = 0;
    bool armless = false;
};
