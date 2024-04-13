#include "image_directory.hpp"
#include "elements/Map/Map.hpp"

int walk_of(const int &img_dir)
{
    switch (img_dir)
    {
    case ZOMBIE_EATING_DIRECTORY:
        return ZOMBIE_WALK_DIRECTORY;
    case ARMLESS_ZOMBIE_EATING_DIRECTORY:
        return ARMLESS_ZOMBIE_WALK_DIRECTORY;

    case FLAG_ZOMBIE_EATING_1_DIRECTORY:
        return FLAG_ZOMBIE_WALK_1_DIRECTORY;
    case FLAG_ZOMBIE_EATING_2_DIRECTORY:
        return FLAG_ZOMBIE_WALK_2_DIRECTORY;
    case FLAG_ZOMBIE_EATING_3_DIRECTORY:
        return FLAG_ZOMBIE_WALK_3_DIRECTORY;

    case CONE_ZOMBIE_EATING_1_DIRECTORY:
        return CONE_ZOMBIE_WALK_1_DIRECTORY;
    case CONE_ZOMBIE_EATING_2_DIRECTORY:
        return CONE_ZOMBIE_WALK_2_DIRECTORY;
    case CONE_ZOMBIE_EATING_3_DIRECTORY:
        return CONE_ZOMBIE_WALK_3_DIRECTORY;

    case BUCKET_ZOMBIE_EATING_1_DIRECTORY:
        return BUCKET_ZOMBIE_WALK_1_DIRECTORY;
    case BUCKET_ZOMBIE_EATING_2_DIRECTORY:
        return BUCKET_ZOMBIE_WALK_2_DIRECTORY;
    case BUCKET_ZOMBIE_EATING_3_DIRECTORY:
        return BUCKET_ZOMBIE_WALK_3_DIRECTORY;

    case DOOR_ZOMBIE_EATING_1_DIRECTORY:
        return DOOR_ZOMBIE_WALK_1_DIRECTORY;
    case DOOR_ZOMBIE_EATING_2_DIRECTORY:
        return DOOR_ZOMBIE_WALK_2_DIRECTORY;
    case DOOR_ZOMBIE_EATING_3_DIRECTORY:
        return DOOR_ZOMBIE_WALK_3_DIRECTORY;
    }
    return NULL_DIRECTORY;
}

Image::Image(string _dir)
{
    img_dir = _dir;
    n_sheet = 0;
    c_sheet = 0;
    alpha = 255;
}
Image::Image(string _dir, int _n, int _c)
{
    img_dir = _dir;
    n_sheet = _n;
    c_sheet = _c;
    alpha = 255;
}
Image::Image(string _dir, int _n, int _c, int _a)
{
    img_dir = _dir;
    n_sheet = _n;
    c_sheet = _c;
    alpha = _a;
}