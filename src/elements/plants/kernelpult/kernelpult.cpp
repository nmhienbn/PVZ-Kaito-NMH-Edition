#include "kernelpult.hpp"
#include "elements/Map/Map.hpp"

// Sheet pacing: idle slower sway (icon dir, 2 tpf); attack reel paces at
// reanim fps 12 == 5 tpf; the loop is launch-rate capped at 180 ticks
// (original mLaunchRate 300 cs).
static PlantAssets KERNELPULT_IDLE(2, 85, 81, 85, 76, 5, 10);
static PlantAssets KERNELPULT_ATTACK(5, 85, 81, 85, 76, 5, 10);

KernelPult::KernelPult(const int &_row, const int &_col) : PultBase(_row, _col)
{
    type = KERNELPULT_TYPE;
    health = PLANT_HEALTH_LIMIT[KERNELPULT_TYPE];
    directory_num = idle_dir();
}

KernelPult::~KernelPult()
{
}

/*One lob in four comes out as butter (repvz Plant.cpp:786: Rand(4)==0, 25%).
The roll happens once per attack loop at the loop head (PultBase calls
pick_ammo() there), so the basket shows the thrown ammo through the windup.
*/
int KernelPult::pick_ammo() const
{
    if (forced_ammo >= 0)
    {
        int kind = forced_ammo;
        forced_ammo = -1;
        return kind;
    }
    return rand(0, 3) == 0 ? LOBBED_BUTTER : LOBBED_CORN;
}

const PlantAssets &KernelPult::idle_assets() const
{
    return KERNELPULT_IDLE;
}

const PlantAssets &KernelPult::attack_assets() const
{
    return KERNELPULT_ATTACK;
}
