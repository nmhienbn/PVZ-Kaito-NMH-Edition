#include "sunflower.hpp"
#define SUNFLOWER_WIDTH 90
#define SUNFLOWER_HEIGHT 100
#define SUNFLOWER_FRAME 2

#define SUNFLOWER_G_WIDTH 90
#define SUNFLOWER_G_HEIGHT 100

extern int game_state;
extern Map cells;
extern window win;

/*
Sunflower constructor
*/
Sunflower::Sunflower(const int &_row, const int &_col)
{
    type = SUNFLOWER_TYPE;
    row = _row;
    col = _col;
    health = PLANT_HEALTH_LIMIT[SUNFLOWER_TYPE];
    sec_for_prepare = SUN_GEN_SUNFLOWER_CLK_COUNT;
    directory_num = SUNFLOWER_SHEET_DIRECTORY;
    frame = 0;
    attacked_time = 0;
}

/*
Sunflower destructor
*/
Sunflower::~Sunflower()
{
}

/*
@return true if sunflower gen a new sun.
*/
bool Sunflower::is_gen_sun()
{
    return directory_num == SUNFLOWER_HAPPY_DIRECTORY && frame == 30 * SUNFLOWER_FRAME;
}

/*
Change sunflower appearance when gen a new sun or normal
*/
void Sunflower::determine_appearance()
{
    // Gen new sun
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
    // Normal
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

/*Generate sun from a sunflower.
Update time to have next sun.
Init a new sun.
Then add it into vector<Sun>.
*/
void Sunflower::gen_sun_from_a_sunflower(vector<Sun> &suns)
{
    // Update time to have next sun.
    --sec_for_prepare;
    determine_appearance();
    if (sec_for_prepare <= 0)
    {
        if (is_gen_sun())
        {
            // Reset time to have next sun.
            sec_for_prepare = SUN_GEN_SUNFLOWER_CLK_COUNT;
            // Init a new sun.
            Sun temp;
            temp.final_col = col;
            temp.final_row = row;
            temp.y_location = cells[row][col].y1 - 20;
            temp.wait_seconds = 0;
            // Then add it into vector<Sun>.
            suns.push_back(temp);
        }
    }
}

/*
Display sunflower in row
*/
void Sunflower::display(const int &_row)
{

    if (row == _row)
    {
        // current frame
        int sframe = frame / SUNFLOWER_FRAME;
        // current column in source image
        int scol = sframe % all_img[SUNFLOWER_SHEET_DIRECTORY].c_sheet;
        // current row in source image
        int srow = sframe / all_img[SUNFLOWER_SHEET_DIRECTORY].c_sheet;
        // Sunflower
        win.draw_png(directory_num, SUNFLOWER_WIDTH * scol, SUNFLOWER_HEIGHT * srow,
                     SUNFLOWER_WIDTH, SUNFLOWER_HEIGHT,
                     cells[row][col].x1 - 3, cells[row][col].y1 - 11,
                     SUNFLOWER_G_WIDTH, SUNFLOWER_G_HEIGHT);
        // Blink
        if (attacked_time)
        {
            win.draw_png(blink_of[directory_num],
                         SUNFLOWER_WIDTH * scol, SUNFLOWER_HEIGHT * srow,
                         SUNFLOWER_WIDTH, SUNFLOWER_HEIGHT,
                         cells[row][col].x1 - 3, cells[row][col].y1 - 11,
                         SUNFLOWER_G_WIDTH, SUNFLOWER_G_HEIGHT);
            if (check_status(game_state, IS_PAUSED) == false)
                attacked_time--;
        }
        // Next frame
        if (check_status(game_state, IS_PAUSED) == false)
            if (++frame >= SUNFLOWER_FRAME * all_img[SUNFLOWER_SHEET_DIRECTORY].n_sheet)
            {
                frame = 0;
            }
    }
}
