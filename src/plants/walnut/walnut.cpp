#include "walnut.hpp"
#define WALNUT_WIDTH 116
#define WALNUT_HEIGHT 143
#define WALNUT_G_WIDTH 68
#define WALNUT_G_HEIGHT 68
#define WALNUT_FRAME 6

extern int game_state;
extern Map cells;
extern window win;

Walnut::Walnut(const int &_row, const int &_col)
{
    row = _row;
    col = _col;
    health = PLANT_HEALTH_LIMIT[WALNUT_TYPE];
    directory_num = WALNUT_1_DIRECTORY;
    frame = 0;
    attacked_time = 0;
}
Walnut::~Walnut()
{
}
/*
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
Display walnut
Render sprite sheet of exactly status.
*/
void Walnut::display(const int &_row)
{
    if (row == _row)
    {
        determine_appearance();
        int sframe = frame / WALNUT_FRAME;
        int srow = sframe / all_img[directory_num].c_sheet;
        int scol = sframe % all_img[directory_num].c_sheet;
        win.draw_png(directory_num, WALNUT_WIDTH * scol, WALNUT_HEIGHT * srow,
                     WALNUT_WIDTH, WALNUT_HEIGHT,
                     cells[row][col].x1 + 5, cells[row][col].y1 + 3,
                     WALNUT_G_WIDTH, WALNUT_G_HEIGHT);

        if (attacked_time)
        {
            win.draw_png(blink_of[directory_num], WALNUT_WIDTH * scol, WALNUT_HEIGHT * srow,
                         WALNUT_WIDTH, WALNUT_HEIGHT,
                         cells[row][col].x1 + 5, cells[row][col].y1 + 3,
                         WALNUT_G_WIDTH, WALNUT_G_HEIGHT);
            if (check_status(game_state, IS_PAUSED) == false)
                attacked_time--;
        }

        if (check_status(game_state, IS_PAUSED) == false)
        {
            if (++frame >= WALNUT_FRAME * all_img[directory_num].n_sheet)
            {
                frame = 0;
            }
        }
    }
}
