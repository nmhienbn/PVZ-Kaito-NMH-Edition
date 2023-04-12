#include "walnut.hpp"
#define WALNUT_WIDTH 185
#define WALNUT_HEIGHT 185
#define WALNUT_G_WIDTH 95
#define WALNUT_G_HEIGHT 95
#define WALNUT_FRAME 60

extern int game_state;
extern Map cells;
extern window win;

/*
bite <= (1/4) * WALNUT_BITE_LIMIT: 1
bite <= (2/4) * WALNUT_BITE_LIMIT: 2
bite <= (3/4) * WALNUT_BITE_LIMIT: 3
bite <= (4/4) * WALNUT_BITE_LIMIT: 4
*/
void determine_walnut_appearance(Walnut &walnut)
{
    if (walnut.bite <= WALNUT_BITE_LIMIT / 4)
    {
        walnut.directory_num = WALNUT_1_DIRECTORY;
    }
    else if (walnut.bite <= WALNUT_BITE_LIMIT / 2)
    {
        walnut.directory_num = WALNUT_2_DIRECTORY;
    }
    else if (walnut.bite <= WALNUT_BITE_LIMIT * 3 / 4)
    {
        walnut.directory_num = WALNUT_3_DIRECTORY;
    }
    else
    {
        walnut.directory_num = WALNUT_4_DIRECTORY;
    }
}

/*
Display walnut
Render sprite sheet of exactly status.
*/
void display_walnuts(vector<Walnut> &walnuts)
{
    for (auto &walnut : walnuts)
    {
        determine_walnut_appearance(walnut);
        int col = walnut.col;
        int row = walnut.row;

        int frame = walnut.frame / WALNUT_FRAME;
        int srow = frame / C_SHEET[walnut.directory_num];
        int scol = frame % C_SHEET[walnut.directory_num];
        win.draw_png(walnut.directory_num, WALNUT_WIDTH * scol, WALNUT_HEIGHT * srow,
                     WALNUT_WIDTH, WALNUT_HEIGHT,
                     cells[row][col].x1 - 5, cells[row][col].y1,
                     WALNUT_G_WIDTH, WALNUT_G_HEIGHT);

        if (walnut.is_attacked)
        {
            win.draw_png(blink_of[walnut.directory_num], WALNUT_WIDTH * scol, WALNUT_HEIGHT * srow,
                         WALNUT_WIDTH, WALNUT_HEIGHT, cells[row][col].x1 - 5, cells[row][col].y1,
                         WALNUT_G_WIDTH, WALNUT_G_HEIGHT);
            walnut.is_attacked--;
        }

        if (check_status(game_state, IS_PAUSED) == false)
        {
            if (++walnut.frame >= WALNUT_FRAME * N_SHEET[walnut.directory_num])
            {
                walnut.frame = 0;
            }
        }
    }
}
