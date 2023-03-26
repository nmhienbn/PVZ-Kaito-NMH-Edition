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
        walnut.blink_directory_num = WALNUT_1_BLINK_DIRECTORY;
    }
    else if (walnut.bite <= WALNUT_BITE_LIMIT / 2)
    {
        walnut.directory_num = WALNUT_2_DIRECTORY;
        walnut.blink_directory_num = WALNUT_2_BLINK_DIRECTORY;
    }
    else if (walnut.bite <= WALNUT_BITE_LIMIT * 3 / 4)
    {
        walnut.directory_num = WALNUT_3_DIRECTORY;
        walnut.blink_directory_num = WALNUT_3_BLINK_DIRECTORY;
    }
    else
    {
        walnut.directory_num = WALNUT_4_DIRECTORY;
        walnut.blink_directory_num = WALNUT_4_BLINK_DIRECTORY;
    }
}

/*Updated:
Change to sprite sheet.
Render sprite sheet of exactly status.
*/
void display_walnuts(window &win, vector<Walnut> &walnuts, Map &cells, bool is_pause)
{
    for (int i = 0; i < walnuts.size(); i++)
    {
        determine_walnut_appearance(walnuts[i]);
        int col = walnuts[i].col;
        int row = walnuts[i].row;

        int frame = walnuts[i].frame / 30;
        int scol, srow = 0;
        if (walnuts[i].directory_num == WALNUT_1_DIRECTORY)
        {
            scol = frame % 3;
        }
        else
        {
            scol = frame & 1;
        }
        win.draw_png(walnuts[i].directory_num, WALNUT_WIDTH * scol, WALNUT_HEIGHT * srow, WALNUT_WIDTH, WALNUT_HEIGHT, cells[row][col].x1 - 5, cells[row][col].y1 + 9, ELEMENT_WIDTH, ELEMENT_HEIGHT);

        if (walnuts[i].is_attacked)
        {
            win.draw_png(walnuts[i].blink_directory_num, WALNUT_WIDTH * scol, WALNUT_HEIGHT * srow, WALNUT_WIDTH, WALNUT_HEIGHT, cells[row][col].x1 - 5, cells[row][col].y1 + 9, ELEMENT_WIDTH, ELEMENT_HEIGHT);
            walnuts[i].is_attacked--;
        }

        if (is_pause == false)
            ++walnuts[i].frame;
    }
}
