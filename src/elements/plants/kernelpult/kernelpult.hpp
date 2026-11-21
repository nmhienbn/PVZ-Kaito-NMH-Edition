#pragma once
#include "../pult/pult_base.hpp"

// Kernel-pult: lobs corn kernels (20 dmg); each lob has a 25% chance of
// being butter instead (40 dmg + the target is glued in place for ~4 s).
class KernelPult : public PultBase
{
public:
    KernelPult(const int &_row, const int &_col);
    ~KernelPult();

    // Headless-check seam (like BungeeZombie::force_target): pin the ammo
    // the NEXT attack loop rolls (LOBbed kind; -1 releases the pin) instead
    // of riding the time-seeded 25% butter roll.
    void force_next_ammo(int kind) { forced_ammo = kind; }

protected:
    int idle_dir() const override { return KERNELPULT_DIRECTORY; }
    int attack_dir() const override { return KERNELPULT_ATTACK_DIRECTORY; }
    // Cornpult anim_shooting is 18..60 (43 frames); the kernal sprite's
    // authored whip exit is f=-1 from frame 29 (local 11). The sheet state
    // paces 5 ticks/frame with n_sheet == 43, so local == sframe ==
    // frame/5: first tick on the fling pose is frame 55.
    int fire_tick() const override { return 55; }
    int lobbed_kind() const override { return LOBBED_CORN; }
    int pick_ammo() const override;
    const PlantAssets &idle_assets() const override;
    const PlantAssets &attack_assets() const override;

private:
    mutable int forced_ammo = -1; // see force_next_ammo
};
