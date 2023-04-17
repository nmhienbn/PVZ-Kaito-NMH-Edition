#include "walnut.hpp"
#define WALNUT_WIDTH 116
#define WALNUT_HEIGHT 143
#define WALNUT_G_WIDTH 68
#define WALNUT_G_HEIGHT 68
#define WALNUT_FRAME 6

extern int game_state;
extern Map cells;
extern window win;

/*
bite <= (1/5) * health: 1
bite <= (2/5) * health: 2
bite <= (3/5) * health: 3
bite <= (4/5) * health: 4
bite <= (5/5) * health: 5
*/
void determine_walnut_appearance(Walnut &walnut)
{
    for (int i = 1; i <= 5; i++)
    {
        if (walnut.bite <= PLANT_HEALTH_LIMIT[WALNUT_TYPE] * i / 5)
        {
            walnut.directory_num = WALNUT_1_DIRECTORY + i - 1;
            return;
        }
    }
}

/*
Display walnut
Render sprite sheet of exactly status.
*/
void display_walnuts(vector<Walnut> &walnuts, const int &_row)
{
    for (auto &walnut : walnuts)
        if (walnut.row == _row)
        {
            if (walnut.frame >= WALNUT_FRAME * all_img[walnut.directory_num].n_sheet)
            {
                walnut.frame = 0;
            }
            determine_walnut_appearance(walnut);
            int col = walnut.col;
            int row = walnut.row;

            int frame = walnut.frame / WALNUT_FRAME;
            int srow = frame / all_img[walnut.directory_num].c_sheet;
            int scol = frame % all_img[walnut.directory_num].c_sheet;
            win.draw_png(walnut.directory_num, WALNUT_WIDTH * scol, WALNUT_HEIGHT * srow,
                         WALNUT_WIDTH, WALNUT_HEIGHT,
                         cells[row][col].x1 + 5, cells[row][col].y1 + 3,
                         WALNUT_G_WIDTH, WALNUT_G_HEIGHT);

            if (walnut.is_attacked)
            {
                win.draw_png(blink_of[walnut.directory_num], WALNUT_WIDTH * scol, WALNUT_HEIGHT * srow,
                             WALNUT_WIDTH, WALNUT_HEIGHT,
                             cells[row][col].x1 + 5, cells[row][col].y1 + 3,
                             WALNUT_G_WIDTH, WALNUT_G_HEIGHT);
                if (check_status(game_state, IS_PAUSED) == false)
                    walnut.is_attacked--;
            }

            if (check_status(game_state, IS_PAUSED) == false)
            {
                if (++walnut.frame >= WALNUT_FRAME * all_img[walnut.directory_num].n_sheet)
                {
                    walnut.frame = 0;
                }
            }
        }
}
