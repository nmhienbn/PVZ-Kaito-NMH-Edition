#include "cabbagepult.hpp"
#include "elements/Map/Map.hpp"

// Sheet pacing: idle rides the icon dir (n_sheet paces a slow sway, 2 tpf);
// the attack reel paces at reanim fps 12 == 5 tpf and the loop is launch-rate
// capped: the original lobbs every 300 cs (mLaunchRate) = 180 ticks.
static PlantAssets CABBAGEPULT_IDLE(2, 85, 81, 85, 76, 5, 10);
static PlantAssets CABBAGEPULT_ATTACK(5, 85, 81, 85, 76, 5, 10);

CabbagePult::CabbagePult(const int &_row, const int &_col) : PultBase(_row, _col)
{
    type = CABBAGEPULT_TYPE;
    health = PLANT_HEALTH_LIMIT[CABBAGEPULT_TYPE];
    directory_num = idle_dir();
}

CabbagePult::~CabbagePult()
{
}

const PlantAssets &CabbagePult::idle_assets() const
{
    return CABBAGEPULT_IDLE;
}

const PlantAssets &CabbagePult::attack_assets() const
{
    return CABBAGEPULT_ATTACK;
}
