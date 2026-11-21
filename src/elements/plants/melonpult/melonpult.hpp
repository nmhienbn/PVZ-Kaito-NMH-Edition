#pragma once
#include "../pult/pult_base.hpp"

// Melon-pult: lobs heavy melons (80 dmg) that splash their 3x3 landing
// neighbourhood for about half damage.
class MelonPult : public PultBase
{
public:
    MelonPult(const int &_row, const int &_col);
    ~MelonPult();

protected:
    int idle_dir() const override { return MELONPULT_DIRECTORY; }
    int attack_dir() const override { return MELONPULT_ATTACK_DIRECTORY; }
    // Melonpult anim_shooting is 16..53 (38 frames); its loop-seam duplicate
    // trims playback to 16..52 (37 frames), and the melon sprite's authored
    // whip exit is f=-1 from frame 29 (play-local 13). The sheet state paces
    // 5 ticks/frame with n_sheet == 38, so local == sframe*37/38: the first
    // tick whose sampled frame is the fling pose is frame 67.
    int fire_tick() const override { return 67; }
    int lobbed_kind() const override { return LOBBED_MELON; }
    const PlantAssets &idle_assets() const override;
    const PlantAssets &attack_assets() const override;
};
