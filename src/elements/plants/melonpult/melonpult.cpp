#include "melonpult.hpp"
#include "elements/Map/Map.hpp"

// Sheet pacing: idle slower sway (icon dir, 2 tpf); attack reel paces at
// reanim fps 12 == 5 tpf; the loop is launch-rate capped at 180 ticks
// (original mLaunchRate 300 cs).
static PlantAssets MELONPULT_IDLE(2, 85, 81, 85, 78, 5, 8);
static PlantAssets MELONPULT_ATTACK(5, 85, 81, 85, 78, 5, 8);

MelonPult::MelonPult(const int &_row, const int &_col) : PultBase(_row, _col)
{
    type = MELONPULT_TYPE;
    health = PLANT_HEALTH_LIMIT[MELONPULT_TYPE];
    directory_num = idle_dir();
}

MelonPult::~MelonPult()
{
}

const PlantAssets &MelonPult::idle_assets() const
{
    return MELONPULT_IDLE;
}

const PlantAssets &MelonPult::attack_assets() const
{
    return MELONPULT_ATTACK;
}
