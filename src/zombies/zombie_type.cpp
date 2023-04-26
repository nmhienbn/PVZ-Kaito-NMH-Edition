#include "zombie_type.hpp"

Zombie *init(int _type, int level_num)
{
    Zombie *res = nullptr;
    switch (_type)
    {
    case NORMAL_TYPE:
        res = new NormalZombie;
        break;
    case FLAG_TYPE:
        res = new FlagZombie;
        break;
    case CONE_TYPE:
        res = new ConeZombie;
        break;
    case BUCKET_TYPE:
        res = new BucketZombie;
        break;
    case DOOR_TYPE:
        res = new DoorZombie;
        break;

    default:
        break;
    }
    if (res == nullptr)
    {
        cout << "allocate new zombie error! " << _type;
        exit(1234);
    }

    res->type = _type;
    // Random appear row
    if (level_num == 1)
        res->row = 2;
    else if (level_num == 2)
        res->row = rand(1, 3);
    else
        res->row = rand(0, 4);
    // Random delay time to appear
    res->x_location = WINDOW_WIDTH - ZOMBIE_EXACT_LOCATION + rand(10, 100);
    res->y_location = cells[res->row][0].y1 - 50;
    // Set init status
    res->is_moving = true;
    res->attacked_time = res->cold_time = 0;
    res->bite_time = BITE_CLK_COUNT;
    res->next_step_time = 0;

    res->init();
    // Random first frame.
    res->frame = rand(0, ZOMBIE_FRAME * all_img[res->directory_num].n_sheet - 1);
    return res;
}