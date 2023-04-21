#include "zombie_struct.hpp"

extern int game_state;
extern Map cells;
extern window win;

#define ARM_FRAME 1

/*Zombie constructor.
 */
Zombie::Zombie()
{
}

/*Zombie constructor.
Random row and delay time appear.
Set init status, heath and image to render.
Random first frame.
*/
Zombie::Zombie(int _type, int level_num)
{
    type = _type;
    // Random appear row
    if (level_num == 1)
        row = 2;
    else if (level_num == 2)
        row = rand(1, 3);
    else
        row = rand(0, 4);
    // Random delay time to appear
    x_location = WINDOW_WIDTH - ZOMBIE_EXACT_LOCATION + rand(10, 100);
    y_location = cells[row][0].y1 - 50;
    // Set init status
    is_moving = true;
    attacked_time = cold_time = 0;
    bite_time = BITE_CLK_COUNT;
    next_step_time = 0;

    switch (type)
    {
    case NORMAL_TYPE:
    {
        dir_width = 166;
        dir_height = 144;
        health = *ZOMBIE_HEALTH_LIMIT[NORMAL_TYPE].rbegin();
        directory_num = ZOMBIE_WALK_DIRECTORY;
        break;
    }

    case FLAG_TYPE:
    {
        dir_width = 166;
        dir_height = 144;
        health = *ZOMBIE_HEALTH_LIMIT[FLAG_TYPE].rbegin();
        directory_num = FLAG_ZOMBIE_WALK_1_DIRECTORY;
        break;
    }

    case CONE_TYPE:
    {
        dir_width = 166;
        dir_height = 144;
        health = *ZOMBIE_HEALTH_LIMIT[CONE_TYPE].rbegin();
        directory_num = CONE_ZOMBIE_WALK_1_DIRECTORY;
        break;
    }

    case BUCKET_TYPE:
    {
        dir_width = 166;
        dir_height = 144;
        health = *ZOMBIE_HEALTH_LIMIT[BUCKET_TYPE].rbegin();
        directory_num = BUCKET_ZOMBIE_WALK_1_DIRECTORY;
        break;
    }

    case DOOR_TYPE:
    {
        dir_width = 196;
        dir_height = 197;
        health = *ZOMBIE_HEALTH_LIMIT[DOOR_TYPE].rbegin();
        directory_num = DOOR_ZOMBIE_WALK_1_DIRECTORY;
        break;
    }

    default:
        break;
    }
    // Random first frame.
    frame = rand(0, ZOMBIE_FRAME * all_img[directory_num].n_sheet - 1);
}

int Zombie::get_health()
{
    return health;
}

/*Change zombie eating status
If is_moving is false, then the zombie must be eating.
Else the zombie must be moving.
*/
void Zombie::change_zombie_eating_status()
{
    if (is_moving == false)
    {
        auto it = eat_of.find(directory_num);
        if (it != eat_of.end())
        {
            frame = 0;
            directory_num = it->second;
        }
    }
    else
    {
        if (walk_of(directory_num) >= 0)
        {
            frame = 0;
            directory_num = walk_of(directory_num);
        }
    }
}

/*Determine zombies's appearance depend on their health:
Armor drop if necessary
*/
void Zombie::determine_appearance(vector<ZombiePart> &zombie_parts)
{
    // Degrade the zombie
    if (ZOMBIE_HEALTH_LIMIT[type].find(health) != ZOMBIE_HEALTH_LIMIT[type].end())
    {
        directory_num = degrade_of[directory_num];
        if (health <= 5)
        {
            zombie_parts.push_back(ZombiePart(ZOMBIE_ARM_DIRECTORY, 1, row, x_location,
                                              ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0));
        }
    }
    // Armor drop if necessary
    switch (type)
    {
    case CONE_TYPE:
    {
        if (health == *ZOMBIE_HEALTH_LIMIT[NORMAL_TYPE].rbegin())
        {
            type = NORMAL_TYPE;
            zombie_parts.push_back(ZombiePart(CONE_DROP_DIRECTORY, HEAD_ZOMBIE_FRAME, row, x_location + 80,
                                              HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT));
        }
        break;
    }

    case BUCKET_TYPE:
    {
        if (health == *ZOMBIE_HEALTH_LIMIT[NORMAL_TYPE].rbegin())
        {
            type = NORMAL_TYPE;
            zombie_parts.push_back(ZombiePart(BUCKET_DROP_DIRECTORY, HEAD_ZOMBIE_FRAME, row, x_location + 80,
                                              HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT));
        }
        break;
    }

    case DOOR_TYPE:
    {
        if (health == *ZOMBIE_HEALTH_LIMIT[NORMAL_TYPE].rbegin())
        {
            type = NORMAL_TYPE;
            dir_width = 166;
            dir_height = 144;
        }
        break;
    }

    default:
        break;
    }
}

/*
Decrease zombie health and change their appearance if necessary.
@return true if zombie die (health = 0).
*/
bool Zombie::decrease_health(vector<ZombiePart> &zombie_parts)
{
    health--;
    attacked_time = MAX_TIME_BLINK;
    determine_appearance(zombie_parts);
    if (health == 0)
    {
        return true;
    }
    return false;
}

void Zombie::add_zombie_die(vector<ZombiePart> &zombie_parts)
{
    zombie_parts.push_back(ZombiePart(ZOMBIE_DIE_DIRECTORY, ZOMBIE_DIE_FRAME,
                                      row, x_location,
                                      ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0));
    zombie_parts.push_back(ZombiePart(ZOMBIE_HEAD_DIRECTORY, HEAD_ZOMBIE_FRAME,
                                      row, x_location + 80,
                                      HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT, cold_time > 0));
}

void Zombie::display(const int &_row)
{
    if (row == _row)
    {
        // zombie
        int sframe = frame / ZOMBIE_FRAME;
        int scol = sframe % all_img[directory_num].c_sheet;
        int srow = sframe / all_img[directory_num].c_sheet;
        win.draw_png(directory_num, dir_width * scol, dir_height * srow,
                     dir_width, dir_height,
                     x_location, y_location,
                     dir_width, dir_height);

        // zombie cold
        if (cold_time)
        {
            win.draw_png(cold_of[directory_num], dir_width * scol, dir_height * srow,
                         dir_width, dir_height,
                         x_location, y_location,
                         dir_width, dir_height);
            if (check_status(game_state, IS_PAUSED) == false)
                if (--cold_time == 0)
                {
                    bite_time /= 2;
                    next_step_time /= 2;
                }
        }

        // zombie attacked
        if (attacked_time)
        {
            win.draw_png(blink_of[directory_num], dir_width * scol, dir_height * srow,
                         dir_width, dir_height,
                         x_location, y_location,
                         dir_width, dir_height);
            if (check_status(game_state, IS_PAUSED) == false)
                attacked_time--;
        }

        // zombie next frame
        if (check_status(game_state, IS_PAUSED) == false)
        {
            if (cold_time % 2 == 0)
                frame++;
            if (frame >= ZOMBIE_FRAME * all_img[directory_num].n_sheet)
            {
                frame = 0;
            }
        }
    }
}

void Zombie::display2(const int &_minus_x)
{
    // zombie
    int sframe = frame / ZOMBIE_FRAME / 2;
    int scol = sframe % all_img[directory_num].c_sheet;
    int srow = sframe / all_img[directory_num].c_sheet;
    win.draw_png(directory_num, dir_width * scol, dir_height * srow,
                 dir_width, dir_height,
                 x_location - _minus_x, y_location,
                 dir_width, dir_height);

    // zombie next frame
    if (++frame >= 2 * ZOMBIE_FRAME * all_img[directory_num].n_sheet)
    {
        frame = 0;
    }
}

void Zombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - dir_height);
    switch (type)
    {
    case NORMAL_TYPE:
    {
        directory_num = rand(ZOMBIE_CREDIT1_DIRECTORY, ZOMBIE_CREDIT2_DIRECTORY);
        break;
    }

    case FLAG_TYPE:
    {
        directory_num = rand(ZOMBIE_CREDIT1_DIRECTORY, ZOMBIE_CREDIT2_DIRECTORY);
        break;
    }

    case CONE_TYPE:
    {
        directory_num = rand(CONE_ZOMBIE_CREDIT1_DIRECTORY, CONE_ZOMBIE_CREDIT2_DIRECTORY);
        break;
    }

    case BUCKET_TYPE:
    {
        directory_num = rand(BUCKET_ZOMBIE_CREDIT1_DIRECTORY, BUCKET_ZOMBIE_CREDIT2_DIRECTORY);
        break;
    }

    case DOOR_TYPE:
    {
        directory_num = rand(DOOR_ZOMBIE_CREDIT1_DIRECTORY, DOOR_ZOMBIE_CREDIT2_DIRECTORY);
        break;
    }

    default:
        break;
    }
}

ZombiePart::ZombiePart(const int &_img_num, const int &_frame_clk,
                       const int &_r, const int &_x,
                       const int &_w, const int &_h, const bool &_cold)
{
    img_num = _img_num;
    frame_clk = _frame_clk;
    row = _r;
    x_location = _x;
    width = _w;
    height = _h;
    is_cold = _cold;
}

/*
    @return 'true' if display finished
*/
bool ZombiePart::display(const int &_row)
{
    if (row == _row)
    {
        int y_location = cells[row][0].y1 - 50;
        int sframe = frame / frame_clk;
        int scol = sframe % all_img[img_num].c_sheet;
        int srow = sframe / all_img[img_num].c_sheet;
        win.draw_png(img_num, width * scol, height * srow,
                     width, height,
                     x_location, y_location,
                     width, height);
        if (is_cold)
            win.draw_png(cold_of[img_num], width * scol, height * srow,
                         width, height,
                         x_location, y_location,
                         width, height);

        if (check_status(game_state, IS_PAUSED) == false)
        {
            if (++frame >= frame_clk * all_img[img_num].n_sheet)
            {
                return true;
            }
        }
    }
    return false;
}
