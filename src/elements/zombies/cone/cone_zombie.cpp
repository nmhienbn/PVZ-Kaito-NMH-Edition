#include "cone_zombie.hpp"

const map<int, int> cone_of{
    {ZOMBIE_CREDIT1_DIRECTORY, 0},
    {ZOMBIE_CREDIT2_DIRECTORY, 22},
    {ZOMBIE_WALK1_DIRECTORY, 44},
    {ZOMBIE_WALK2_DIRECTORY, 91},
    {ZOMBIE_EATING_DIRECTORY, 138}};

const int CONE_WIDTH = 88;
const int CONE_HEIGHT = 68;
const int CONE_X_BIAS = 70;
const int CONE_Y_BIAS = -12;

extern Window win;

ConeZombie::~ConeZombie()
{
}

void ConeZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = *ZOMBIE_HEALTH_LIMIT[CONE_TYPE].rbegin();
    directory_num = ZOMBIE_WALK_DIRECTORY;
    armor_dir = CONE1_DIRECTORY;
}

void ConeZombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - dir_height);
    directory_num = ZOMBIE_CREDIT_DIRECTORY;
    armor_dir = CONE1_DIRECTORY;
}

void ConeZombie::armor_drop(vector<ZombiePart> &zombie_parts)
{
    if (health == *ZOMBIE_HEALTH_LIMIT[NORMAL_TYPE].rbegin())
    {
        type = NORMAL_TYPE;
        zombie_parts.push_back(ZombiePart(CONE_DROP_DIRECTORY, HEAD_ZOMBIE_FRAME, row, x_location + 80,
                                          HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT, cold_time > 0));
    }
}

void ConeZombie::render_zombie()
{
    Zombie::render_zombie();
    if (armor_dir == NULL_DIRECTORY)
        return;

    // current frame
    int sframe = frame / ZOMBIE_FRAME + cone_of.at(directory_num);
    // current column in source image
    int scol = sframe % all_img[armor_dir].c_sheet;
    // current row in source image
    int srow = sframe / all_img[armor_dir].c_sheet;

    int x_loc = x_location + CONE_X_BIAS;
    int y_loc = y_location + CONE_Y_BIAS;

    win.draw_png(armor_dir, CONE_WIDTH * scol, CONE_HEIGHT * srow,
                 CONE_WIDTH, CONE_HEIGHT,
                 x_loc, y_loc,
                 CONE_WIDTH, CONE_HEIGHT);

    // zombie cold
    if (cold_time)
    {
        win.set_texture_color(blink_of[armor_dir], 0, 75, 255);
        win.set_texture_alpha(blink_of[armor_dir], 120);
        win.draw_png(blink_of[armor_dir], CONE_WIDTH * scol, CONE_HEIGHT * srow,
                     CONE_WIDTH, CONE_HEIGHT,
                     x_loc, y_loc,
                     CONE_WIDTH, CONE_HEIGHT);
        win.set_texture_color(blink_of[armor_dir], 255, 255, 255);
        win.set_texture_alpha(blink_of[armor_dir], 70);
    }

    // zombie attacked
    if (attacked_time)
    {
        win.draw_png(blink_of[armor_dir], CONE_WIDTH * scol, CONE_HEIGHT * srow,
                     CONE_WIDTH, CONE_HEIGHT,
                     x_loc, y_loc,
                     CONE_WIDTH, CONE_HEIGHT);
    }
}

void ConeZombie::display_credited(const int &_minus_x)
{
    // current frame
    int sframe = frame / ZOMBIE_FRAME / 2;
    // current column in source image
    int scol = sframe % all_img[directory_num].c_sheet;
    // current row in source image
    int srow = sframe / all_img[directory_num].c_sheet;
    win.draw_png(directory_num, dir_width * scol, dir_height * srow,
                 dir_width, dir_height,
                 x_location - _minus_x, y_location,
                 dir_width, dir_height);

    sframe = frame / ZOMBIE_FRAME / 2 + cone_of.at(directory_num);
    scol = sframe % all_img[armor_dir].c_sheet;
    srow = sframe / all_img[armor_dir].c_sheet;
    int x_loc = x_location + CONE_X_BIAS - _minus_x;
    int y_loc = y_location + CONE_Y_BIAS;

    win.draw_png(armor_dir, CONE_WIDTH * scol, CONE_HEIGHT * srow,
                 CONE_WIDTH, CONE_HEIGHT,
                 x_loc, y_loc,
                 CONE_WIDTH, CONE_HEIGHT);

    // zombie next frame
    if (++frame >= 2 * ZOMBIE_FRAME * all_img[directory_num].n_sheet)
    {
        frame = 0;
    }
}
