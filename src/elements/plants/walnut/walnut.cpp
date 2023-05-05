#include "walnut.hpp"
#define WALNUT_WIDTH 68
#define WALNUT_HEIGHT 84
#define WALNUT_G_WIDTH 68
#define WALNUT_G_HEIGHT 84
#define WALNUT_FRAME 6

extern int game_state;
extern Map cells;
extern window win;

/*
Wall-nut constructor
*/
Walnut::Walnut(const int &_row, const int &_col)
{
    type = WALNUT_TYPE;
    row = _row;
    col = _col;
    health = PLANT_HEALTH_LIMIT[WALNUT_TYPE];
    directory_num = WALNUT_1_DIRECTORY;
    frame = 0;
    attacked_time = 0;
}

/*
Wall-nut destructor
*/
Walnut::~Walnut()
{
}

/* Change wall-nut appearance
health <= (1/5) * health: 5
health <= (2/5) * health: 4
health <= (3/5) * health: 3
health <= (4/5) * health: 2
health <= (5/5) * health: 1
*/
void Walnut::determine_appearance()
{
    for (int i = 1; i <= 5; i++)
    {
        if (health <= PLANT_HEALTH_LIMIT[WALNUT_TYPE] * i / 5)
        {
            directory_num = WALNUT_1_DIRECTORY + 5 - i;
            if (frame >= WALNUT_FRAME * all_img[directory_num].n_sheet)
            {
                frame = 0;
            }
            return;
        }
    }
}

/*
Display walnut in row
*/
void Walnut::display(const int &_row)
{
    if (row == _row)
    {
        determine_appearance();
        // current frame
        int sframe = frame / WALNUT_FRAME;
        // current row in source image
        int srow = sframe / all_img[directory_num].c_sheet;
        // current column in source image
        int scol = sframe % all_img[directory_num].c_sheet;
        // Wall-nut
        win.draw_png(directory_num, WALNUT_WIDTH * scol, WALNUT_HEIGHT * srow,
                     WALNUT_WIDTH, WALNUT_HEIGHT,
                     cells[row][col].x1 + 5, cells[row][col].y1 + 3,
                     WALNUT_G_WIDTH, WALNUT_G_HEIGHT);
        // Blink
        if (attacked_time)
        {
            win.draw_png(blink_of[directory_num], WALNUT_WIDTH * scol, WALNUT_HEIGHT * srow,
                         WALNUT_WIDTH, WALNUT_HEIGHT,
                         cells[row][col].x1 + 5, cells[row][col].y1 + 3,
                         WALNUT_G_WIDTH, WALNUT_G_HEIGHT);
            if (check_status(game_state, IS_PAUSED) == false)
                attacked_time--;
        }
        // Next frame
        if (check_status(game_state, IS_PAUSED) == false)
        {
            if (++frame >= WALNUT_FRAME * all_img[directory_num].n_sheet)
            {
                frame = 0;
            }
        }
    }
}
