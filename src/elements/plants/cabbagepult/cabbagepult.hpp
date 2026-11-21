#pragma once
#include "../pult/pult_base.hpp"

// Cabbage-pult: lobs cabbages in an arc (20 dmg); the pult design ignores the
// roof slope that blocks straight shooters.
class CabbagePult : public PultBase
{
public:
    CabbagePult(const int &_row, const int &_col);
    ~CabbagePult();

protected:
    int idle_dir() const override { return CABBAGEPULT_DIRECTORY; }
    int attack_dir() const override { return CABBAGEPULT_ATTACK_DIRECTORY; }
    // Cabbagepult anim_shooting is 76..104 (29 frames); the cabbage sprite's
    // authored whip exit is f=-1 from frame 87 (local 11). The sheet state
    // paces 5 ticks/frame with n_sheet == 29, so local == sframe ==
    // frame/5: first tick on the fling pose is frame 55.
    int fire_tick() const override { return 55; }
    int lobbed_kind() const override { return LOBBED_CABBAGE; }
    const PlantAssets &idle_assets() const override;
    const PlantAssets &attack_assets() const override;
};
