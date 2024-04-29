#include "zombie_struct.hpp"
#include "draw/rsdl.hpp"
#include "elements/Map/Map.hpp"
#include "music/music.hpp"

extern int game_state;
extern Map cells;
extern Window win;

/*Corresponding walking of zombie eat image*/
static map<int, int> degrade_of = {
    {ZOMBIE_WALK1_DIRECTORY, ARMLESS_ZOMBIE_WALK1_DIRECTORY},
    {ZOMBIE_WALK2_DIRECTORY, ARMLESS_ZOMBIE_WALK2_DIRECTORY},
    {ZOMBIE_EATING_DIRECTORY, ARMLESS_ZOMBIE_EATING_DIRECTORY},

    {CONE1_DIRECTORY, CONE2_DIRECTORY},
    {CONE2_DIRECTORY, CONE3_DIRECTORY},
    {CONE3_DIRECTORY, NULL_DIRECTORY},

    {BUCKET1_DIRECTORY, BUCKET2_DIRECTORY},
    {BUCKET2_DIRECTORY, BUCKET3_DIRECTORY},
    {BUCKET3_DIRECTORY, NULL_DIRECTORY},

    {FLAG_ZOMBIE_WALK1_1_DIRECTORY, FLAG_ZOMBIE_WALK1_2_DIRECTORY},
    {FLAG_ZOMBIE_WALK2_1_DIRECTORY, FLAG_ZOMBIE_WALK2_2_DIRECTORY},
    {FLAG_ZOMBIE_EATING_1_DIRECTORY, FLAG_ZOMBIE_EATING_2_DIRECTORY},
    {FLAG_ZOMBIE_WALK1_2_DIRECTORY, FLAG_ZOMBIE_WALK1_3_DIRECTORY},
    {FLAG_ZOMBIE_WALK2_2_DIRECTORY, FLAG_ZOMBIE_WALK2_3_DIRECTORY},
    {FLAG_ZOMBIE_EATING_2_DIRECTORY, FLAG_ZOMBIE_EATING_3_DIRECTORY},
    {FLAG_ZOMBIE_WALK1_3_DIRECTORY, ARMLESS_ZOMBIE_WALK1_DIRECTORY},
    {FLAG_ZOMBIE_WALK2_3_DIRECTORY, ARMLESS_ZOMBIE_WALK2_DIRECTORY},
    {FLAG_ZOMBIE_EATING_3_DIRECTORY, ARMLESS_ZOMBIE_EATING_DIRECTORY},

    {DOOR_ZOMBIE_WALK1_1_DIRECTORY, DOOR_ZOMBIE_WALK1_2_DIRECTORY},
    {DOOR_ZOMBIE_WALK2_1_DIRECTORY, DOOR_ZOMBIE_WALK2_2_DIRECTORY},
    {DOOR_ZOMBIE_EATING_1_DIRECTORY, DOOR_ZOMBIE_EATING_2_DIRECTORY},
    {DOOR_ZOMBIE_WALK1_2_DIRECTORY, DOOR_ZOMBIE_WALK1_3_DIRECTORY},
    {DOOR_ZOMBIE_WALK2_2_DIRECTORY, DOOR_ZOMBIE_WALK2_3_DIRECTORY},
    {DOOR_ZOMBIE_EATING_2_DIRECTORY, DOOR_ZOMBIE_EATING_3_DIRECTORY},
    {DOOR_ZOMBIE_WALK1_3_DIRECTORY, ZOMBIE_WALK1_DIRECTORY},
    {DOOR_ZOMBIE_WALK2_3_DIRECTORY, ZOMBIE_WALK2_DIRECTORY},
    {DOOR_ZOMBIE_EATING_3_DIRECTORY, ZOMBIE_EATING_DIRECTORY}

};

/*Zombie constructor.
 */
Zombie::Zombie()
{
}

/*Zombie constructor.
Random row and delay time appear.
Set init status, heath and image to render. (override)
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
    // init zombie override
    init();
    // Random first frame.
    frame = rand(0, ZOMBIE_FRAME * all_img[directory_num].n_sheet - 1);
}

Zombie::~Zombie()
{
}

int Zombie::get_row() const
{
    return row;
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
        auto dir = eat_of(directory_num);
        if (dir >= 0)
        {
            frame = 0;
            directory_num = dir;
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
        if (type == CONE_TYPE || type == BUCKET_TYPE)
        {
            armor_dir = degrade_of[armor_dir];
        }
        else
        {
            directory_num = degrade_of[directory_num];
            if (health <= 5)
            {
                zombie_parts.push_back(ZombiePart(ZOMBIE_ARM_DIRECTORY, 1, row, x_location,
                                                  ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0));
            }
        }
    }
    // Armor drop if necessary
    armor_drop(zombie_parts);
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

/*
Add zombie die: body & head
*/
void Zombie::add_zombie_die(vector<ZombiePart> &zombie_parts)
{
    zombie_parts.push_back(ZombiePart(ZOMBIE_DIE_DIRECTORY, ZOMBIE_DIE_FRAME,
                                      row, x_location,
                                      ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0));
    zombie_parts.push_back(ZombiePart(ZOMBIE_HEAD_DIRECTORY, HEAD_ZOMBIE_FRAME,
                                      row, x_location + 80,
                                      HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT, cold_time > 0));
}

void Zombie::render_zombie()
{
    // zombie
    // current frame
    int sframe = frame / ZOMBIE_FRAME;
    win.draw_nth_frame(directory_num, sframe, dir_width, dir_height,
                       x_location, y_location, dir_width, dir_height);

    // zombie cold
    if (cold_time)
    {
        win.set_texture_color(blink_of(directory_num), 0, 75, 255);
        win.set_texture_alpha(blink_of(directory_num), 120);
        win.draw_nth_frame(blink_of(directory_num), sframe, dir_width, dir_height,
                           x_location, y_location, dir_width, dir_height);
        win.set_texture_color(blink_of(directory_num), 255, 255, 255);
        win.set_texture_alpha(blink_of(directory_num), 70);
    }

    // zombie attacked
    if (attacked_time)
    {
        win.draw_nth_frame(blink_of(directory_num), sframe, dir_width, dir_height,
                           x_location, y_location, dir_width, dir_height);
    }
}

/*
display zombie
*/
void Zombie::display(const int &_row)
{
    if (row == _row)
    {
        render_zombie();
        // zombie next frame
        if (check_status(game_state, IS_PAUSED) == false)
        {
            if (cold_time)
                if (--cold_time == 0)
                {
                    bite_time /= 2;
                    next_step_time /= 2;
                }
            if (attacked_time)
                attacked_time--;
            if (cold_time % 2 == 0)
                frame++;
            if (frame >= ZOMBIE_FRAME * all_img[directory_num].n_sheet)
                frame = 0;
        }
    }
}

/*
display credited zombie
*/
void Zombie::display_credited(const int &_minus_x)
{
    const int REAL_FRAME = ZOMBIE_FRAME * 2;
    // current frame
    int sframe = frame / REAL_FRAME;
    win.draw_nth_frame(directory_num, sframe, dir_width, dir_height,
                       x_location - _minus_x, y_location, dir_width, dir_height);

    // zombie next frame
    if (++frame >= REAL_FRAME * all_img[directory_num].n_sheet)
    {
        frame = 0;
    }
}

/*
Zombie part constructor
*/
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

/*@return 'true' if display finished*/
bool ZombiePart::display(const int &_row)
{
    if (row == _row)
    {
        int y_location = cells[row][0].y1 - 50;
        // current frame
        int sframe = frame / frame_clk;
        win.draw_nth_frame(img_num, sframe, width, height,
                           x_location, y_location, width, height);
        // cold
        if (is_cold)
        {
            win.set_texture_color(blink_of(img_num), 0, 75, 255);
            win.set_texture_alpha(blink_of(img_num), 120);
            win.draw_nth_frame(blink_of(img_num), sframe, width, height,
                               x_location, y_location, width, height);
            win.set_texture_color(blink_of(img_num), 255, 255, 255);
            win.set_texture_alpha(blink_of(img_num), 70);
        }
        // next frame
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
