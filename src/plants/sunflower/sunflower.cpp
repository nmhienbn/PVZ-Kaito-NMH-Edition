#include "sunflower.hpp"
#define SUNFLOWER_WIDTH 174
#define SUNFLOWER_HEIGHT 172
#define SUNFLOWER_FRAME 2

#define SUNFLOWER_G_WIDTH 120
#define SUNFLOWER_G_HEIGHT 120

extern int game_state;
extern Map cells;
extern window win;

Sunflower::Sunflower(const int &_row, const int &_col)
{
    row = _row;
    col = _col;
    health = PLANT_HEALTH_LIMIT[SUNFLOWER_TYPE];
    sec_for_prepare = SUN_GEN_SUNFLOWER_CLK_COUNT;
    directory_num = SUNFLOWER_SHEET_DIRECTORY;
    frame = 0;
    attacked_time = 0;
}

Sunflower::~Sunflower()
{
}

/**
Generate suns from sunflowers not at the same time.
**/
void gen_sun_from_all_sunflowers(vector<Sunflower> &sunflowers, vector<Sun> &suns)
{
    for (auto &sunflower : sunflowers)
    {
        sunflower.gen_sun_from_a_sunflower(suns);
    }
}

bool Sunflower::is_gen_sun()
{
    return directory_num == SUNFLOWER_HAPPY_DIRECTORY && frame == 30 * SUNFLOWER_FRAME;
}

void Sunflower::determine_appearance()
{
    if (sec_for_prepare <= 0)
    {
        if (directory_num == SUNFLOWER_SHEET_DIRECTORY)
        {
            if (frame == 0)
            {
                directory_num = SUNFLOWER_HAPPY_DIRECTORY;
            }
        }
    }
    else
    {
        if (directory_num == SUNFLOWER_HAPPY_DIRECTORY)
        {
            if (frame == 0)
            {
                directory_num = SUNFLOWER_SHEET_DIRECTORY;
            }
        }
    }
    if (frame >= SUNFLOWER_FRAME * all_img[SUNFLOWER_SHEET_DIRECTORY].n_sheet)
    {
        frame = 0;
    }
}

/*
Generate sun from the sunflowers.
Init waiting time of sun.
Then add it into vector<Sun>.
Reset time to have next sun.
*/
void Sunflower::gen_sun_from_a_sunflower(vector<Sun> &suns)
{
    --sec_for_prepare;
    determine_appearance();
    if (sec_for_prepare <= 0)
    {
        if (is_gen_sun())
        {
            sec_for_prepare = SUN_GEN_SUNFLOWER_CLK_COUNT;
            Sun temp;
            temp.final_col = col;
            temp.final_row = row;
            temp.y_location = cells[row][col].y1 - 20;
            temp.wait_seconds = 0;
            suns.push_back(temp);
        }
    }
}

/*
Display sunflowers
*/
void Sunflower::display(const int &_row)
{

    if (row == _row)
    {
        int sframe = frame / SUNFLOWER_FRAME;
        int scol = sframe % all_img[SUNFLOWER_SHEET_DIRECTORY].c_sheet;
        int srow = sframe / all_img[SUNFLOWER_SHEET_DIRECTORY].c_sheet;
        win.draw_png(directory_num, SUNFLOWER_WIDTH * scol, SUNFLOWER_HEIGHT * srow,
                     SUNFLOWER_WIDTH, SUNFLOWER_HEIGHT,
                     cells[row][col].x1 - 22, cells[row][col].y1 - 28,
                     SUNFLOWER_G_WIDTH, SUNFLOWER_G_HEIGHT);

        if (check_status(game_state, IS_PAUSED) == false)
            if (++frame >= SUNFLOWER_FRAME * all_img[SUNFLOWER_SHEET_DIRECTORY].n_sheet)
            {
                frame = 0;
            }
        if (attacked_time)
        {
            win.draw_png(blink_of[directory_num],
                         SUNFLOWER_WIDTH * scol, SUNFLOWER_HEIGHT * srow,
                         SUNFLOWER_WIDTH, SUNFLOWER_HEIGHT,
                         cells[row][col].x1 - 22, cells[row][col].y1 - 28,
                         SUNFLOWER_G_WIDTH, SUNFLOWER_G_HEIGHT);
            if (check_status(game_state, IS_PAUSED) == false)
                attacked_time--;
        }
    }
}
