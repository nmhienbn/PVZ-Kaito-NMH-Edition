#include "walnut.h"

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

/*Updated:
Change to sprite sheet.
Render sprite sheet of exactly status.
*/
void display_walnuts(window &win, vector<Walnut> &walnuts, Map &cells, bool is_pause)
{
    for (auto &walnut : walnuts)
    {
        determine_walnut_appearance(walnut);
        int col = walnut.col;
        int row = walnut.row;

        int frame = walnut.frame / 30;
        int scol, srow = 0;
        if (walnut.directory_num == WALNUT_1_DIRECTORY)
        {
            scol = frame % 3;
        }
        else
        {
            scol = frame & 1;
        }
        win.draw_png(walnut.directory_num, WALNUT_WIDTH * scol, WALNUT_HEIGHT * srow, WALNUT_WIDTH, WALNUT_HEIGHT, cells[row][col].x1 - 5, cells[row][col].y1 + 9, ELEMENT_WIDTH, ELEMENT_HEIGHT);

        if (walnut.is_attacked)
        {
            win.draw_png(blink_of[walnut.directory_num], WALNUT_WIDTH * scol, WALNUT_HEIGHT * srow, WALNUT_WIDTH, WALNUT_HEIGHT, cells[row][col].x1 - 5, cells[row][col].y1 + 9, ELEMENT_WIDTH, ELEMENT_HEIGHT);
            walnut.is_attacked--;
        }

        if (is_pause == false)
            ++walnut.frame;
    }
}
