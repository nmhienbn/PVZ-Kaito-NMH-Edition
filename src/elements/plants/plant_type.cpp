#include "plant_type.hpp"

Plants *init_plant(const int &type, const int &row, const int &col)
{
    switch (type)
    {
    case PEASHOOTER_TYPE:
        return new Peashooter(row, col);
    case SUNFLOWER_TYPE:
        return new Sunflower(row, col);
    case WALLNUT_TYPE:
        return new WallNut(row, col);
    case SNOWPEA_TYPE:
        return new SnowPea(row, col);
    case POTATOMINE_TYPE:
        return new PotatoMine(row, col);
    case CHERRYBOMB_TYPE:
        return new CherryBomb(row, col);

    default:
        return nullptr;
    }
}