#include "draw_elements.h"
#include <algorithm>
/*
Display game layout:
    + Background (playground)
    + Sun and Player's Sun Count.
    + Plants' seed (Chosen or not).
*/
void display_game_layout(window &win, Player player, Icons icons, Level &level)
{
    win.draw_bg(BLACK_SCREEN_DIRECTORY);
    win.draw_bg(level.background_directory);
    win.draw_png_scale(ICON_BAR_DIRECTORY, 15, 85, ICON_BAR_WIDTH, ICON_BAR_HEIGHT);
    win.draw_png_scale(SUN_BAR_DIRECTORY, 5, 5, SUN_BAR_WIDTH, SUN_BAR_HEIGHT);
    display_shovel(win);
    display_icons_in_icon_bar(icons, player, win, level);
    display_menu_icon(win);

    win.show_text(std::to_string(player.sun_count), 85, 33, BLACK);
}

/*
Display the plants' seed:
    + Dim if player doesn't have enough sun or it is chosen. - Updated
    + Render a black screen to display remaining time for it to refresh. - Updated
    + Normal if player have enough sun.
*/
void display_icons_in_icon_bar(Icons icons, Player player, window &win, Level &level)
{
    int peashooter_icon = PEASHOOTER_ICON_BRIGHT_DIRECTORY;
    int sunflower_icon = SUNFLOWER_ICON_BRIGHT_DIRECTORY;
    int walnut_icon = WALNUT_ICON_BRIGHT_DIRECTORY;

    if (player.sun_count < 100 || icons.is_peashooter_chosen)
        peashooter_icon = PEASHOOTER_ICON_DIM_DIRECTORY;
    if (player.sun_count < 50 || icons.is_sunflower_chosen)
        sunflower_icon = SUNFLOWER_ICON_DIM_DIRECTORY;
    if (player.sun_count < 50 || icons.is_walnut_chosen)
        walnut_icon = WALNUT_ICON_DIM_DIRECTORY;

    win.draw_png_scale(peashooter_icon, ICON_BAR_X1 + 3, PEASHOOTER_ICON_Y1, ICON_WIDTH, ICON_HEIGHT);
    win.draw_png(BLACK_SCREEN_DIRECTORY, ICON_BAR_X1 + 3, PEASHOOTER_ICON_Y1, ICON_WIDTH, icons.peashooter_remaining_time * ICON_HEIGHT / PEASHOOTER_LOADING);

    if (level.level_num >= 2)
    {
        win.draw_png_scale(sunflower_icon, ICON_BAR_X1 + 3, SUNFLOWER_ICON_Y1, ICON_WIDTH, ICON_HEIGHT);
        win.draw_png(BLACK_SCREEN_DIRECTORY, ICON_BAR_X1 + 3, SUNFLOWER_ICON_Y1, ICON_WIDTH, icons.sunflower_remaining_time * ICON_HEIGHT / SUNFLOWER_LOADING);
    }

    if (level.level_num >= 3)
    {
        win.draw_png_scale(walnut_icon, ICON_BAR_X1 + 3, WALNUT_ICON_Y1, ICON_WIDTH, ICON_HEIGHT);
        win.draw_png(BLACK_SCREEN_DIRECTORY, ICON_BAR_X1 + 3, WALNUT_ICON_Y1, ICON_WIDTH, icons.walnut_remaining_time * ICON_HEIGHT / WALNUT_LOADING);
    }
}

/*
Display the game element: plants, zombies and the suns.
Updated: render order.
*/
void display_game_elements(window &win, Elements &elements, Map &cells)
{
    display_peashooters(win, elements.peashooters, cells);
    display_sunflowers(win, elements.sunflowers, cells);
    display_walnuts(win, elements.walnuts, cells);

    display_zombies(win, elements.zombies, cells);
    display_dead_zombies(win, elements.dead_zombies, cells);

    display_peas(win, elements.peas, cells);
    display_suns(win, elements.suns, cells);
}

/*
If player lose, display this.
*/
void display_losing_message(window &win, Elements &elements, Map &cells, Level &level)
{
    win.draw_bg(level.background_directory);
    display_zombies(win, elements.zombies, cells, true);
    display_dead_zombies(win, elements.dead_zombies, cells, true);
    display_peashooters(win, elements.peashooters, cells, true);
    display_sunflowers(win, elements.sunflowers, cells, true);
    display_walnuts(win, elements.walnuts, cells, true);
    display_peas(win, elements.peas, cells);
    win.draw_bg(BLACK_SCREEN_DIRECTORY);
    win.draw_png_scale(LOSING_MESSAGE_DIRECTORY, WINDOW_WIDTH / 4, WINDOW_HEIGHT / 8 - 40, 500, 500);
}

/*
If player win, display this
*/
void display_winning_message(window &win, Elements &elements, Map &cells, Level &level)
{
    win.draw_bg(level.background_directory);
    display_zombies(win, elements.zombies, cells, true);
    display_dead_zombies(win, elements.dead_zombies, cells, true);
    display_peashooters(win, elements.peashooters, cells, true);
    display_sunflowers(win, elements.sunflowers, cells, true);
    display_walnuts(win, elements.walnuts, cells, true);
    display_peas(win, elements.peas, cells);
    win.draw_bg(BLACK_SCREEN_DIRECTORY);
    win.draw_png_scale(WINNING_MESSAGE_DIRECTORY, WINDOW_WIDTH / 3, WINDOW_HEIGHT / 8 - 60, 300, 550);
}

void display_game_pause(window &win, Elements &elements, Map &cells, Level &level)
{
    win.draw_bg(level.background_directory);
    display_zombies(win, elements.zombies, cells, true);
    display_dead_zombies(win, elements.dead_zombies, cells, true);
    display_peashooters(win, elements.peashooters, cells, true);
    display_sunflowers(win, elements.sunflowers, cells, true);
    display_walnuts(win, elements.walnuts, cells, true);
    display_peas(win, elements.peas, cells);
    win.draw_bg(BLACK_SCREEN_DIRECTORY);
    display_menu(win);
}

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Display Game Elements
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

void display_suns(window &win, vector<Sun> suns, Map &cells)
{
    for (auto &sun : suns)
    {
        if (!sun.is_clicked)
        {
            int col = sun.final_col;
            sun.x_location = cells[0][col].x1;
        }
        win.draw_png_scale(SUN_DIRECTORY, sun.x_location, sun.y_location, ELEMENT_WIDTH, ELEMENT_HEIGHT);
    }
}

/*Updated
Change to sprite sheet.
Render sprite sheet of exactly status.
*/
void display_zombies(window &win, vector<Zombie> &zombies, Map &cells, bool is_pause)
{
    std::sort(zombies.begin(), zombies.end());
    for (int i = (int)zombies.size() - 1; i >= 0; i--)
    {
        int row = zombies[i].row;
        int y_location = cells[row][0].y1 - 25;
        // win.draw_png_scale(zombies[i].directory_num, zombies[i].x_location, y_location, ELEMENT_WIDTH, ELEMENT_HEIGHT);

        int zframe = ZOMBIE_FRAME;
        if (zombies[i].directory_num == ZOMBIE_EATING_DIRECTORY)
        {
            zframe = ZOMBIE_EATING_FRAME;
        }
        int frame = zombies[i].frame / zframe;
        int scol = frame % zombies[i].c_sheet;
        int srow = frame / zombies[i].c_sheet;
        win.draw_png(zombies[i].directory_num, ZOMBIE_WIDTH * scol, ZOMBIE_HEIGHT * srow, ZOMBIE_WIDTH, ZOMBIE_HEIGHT, zombies[i].x_location, y_location, ZOMBIE_G_WIDTH, ZOMBIE_G_HEIGHT);
        if (zombies[i].is_attacked)
        {
            win.draw_png(zombies[i].blink_directory_num, ZOMBIE_WIDTH * scol, ZOMBIE_HEIGHT * srow, ZOMBIE_WIDTH, ZOMBIE_HEIGHT, zombies[i].x_location, y_location, ZOMBIE_G_WIDTH, ZOMBIE_G_HEIGHT);
            zombies[i].is_attacked--;
        }
        if (is_pause == false)
            if (++zombies[i].frame >= zframe * zombies[i].n_sheet)
            {
                zombies[i].frame = 0;
            }
    }
}

/*New function:
Display zombie dead independently.
*/
void display_dead_zombies(window &win, vector<DeadZombie> &dead_zombies, Map &cells, bool is_pause)
{
    for (int i = 0; i < dead_zombies.size(); i++)
    {
        int row = dead_zombies[i].row;
        int y_location = cells[row][0].y1 - 35;
        // win.draw_png_scale(zombies[i].directory_num, zombies[i].x_location, y_location, ELEMENT_WIDTH, ELEMENT_HEIGHT);
        int frame = dead_zombies[i].frame / ZOMBIE_DIE_FRAME;
        if (frame <= DEAD_ZOMBIE_N_SHEET)
        {
            int scol = frame % DEAD_ZOMBIE_C_SHEET;
            int srow = frame / DEAD_ZOMBIE_C_SHEET;
            win.draw_png(ZOMBIE_DIE_DIRECTORY, DEAD_ZOMBIE_WIDTH * scol, DEAD_ZOMBIE_HEIGHT * srow, DEAD_ZOMBIE_WIDTH, DEAD_ZOMBIE_HEIGHT, dead_zombies[i].x_location, y_location, DEAD_ZOMBIE_WIDTH, DEAD_ZOMBIE_HEIGHT);
        }
        int scol = frame % HEAD_ZOMBIE_C_SHEET;
        int srow = frame / HEAD_ZOMBIE_C_SHEET;
        win.draw_png(ZOMBIE_HEAD_DIRECTORY, HEAD_ZOMBIE_WIDTH * scol, HEAD_ZOMBIE_HEIGHT * srow, HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT, dead_zombies[i].x_location + 80, y_location - 40, HEAD_ZOMBIE_G_WIDTH, HEAD_ZOMBIE_G_HEIGHT);
        if (is_pause == false)
            if (++dead_zombies[i].frame >= ZOMBIE_DIE_FRAME * HEAD_ZOMBIE_N_SHEET)
            {

                dead_zombies.erase(dead_zombies.begin() + i);
            }
    }
}

/*Updated
Change to sprite sheet
*/
void display_peashooters(window &win, vector<Peashooter> &peashooters, Map &cells, bool is_pause)
{
    for (int i = 0; i < peashooters.size(); i++)
    {
        int col = peashooters[i].col;
        int row = peashooters[i].row;
        int frame = peashooters[i].frame;
        int scol = frame % PEASHOOTER_C_SHEET;
        int srow = frame / PEASHOOTER_C_SHEET;
        win.draw_png(peashooters[i].directory_num, PEASHOOTER_WIDTH * scol, PEASHOOTER_HEIGHT * srow, PEASHOOTER_WIDTH, PEASHOOTER_HEIGHT, cells[row][col].x1, cells[row][col].y1 + 5, ELEMENT_WIDTH, ELEMENT_HEIGHT);
        if (peashooters[i].is_attacked)
        {
            win.draw_png(peashooters[i].directory_num, PEASHOOTER_WIDTH * scol, PEASHOOTER_HEIGHT * srow, PEASHOOTER_WIDTH, PEASHOOTER_HEIGHT, cells[row][col].x1, cells[row][col].y1 + 5, ELEMENT_WIDTH, ELEMENT_HEIGHT);
            peashooters[i].is_attacked--;
        }
        if (is_pause == false)
            if (++peashooters[i].frame >= PEASHOOTER_N_SHEET)
            {
                peashooters[i].frame = 0;
            }
    }
}

void display_peas(window &win, vector<Pea> &peas, Map &cells)
{
    for (int i = 0; i < peas.size(); i++)
    {
        int row = peas[i].row;
        int y_location = cells[row][0].y1 + 20;
        int more_px = 0;
        if (peas[i].directory_num == PEA_EXPLODE_DIRECTORY)
            more_px += 25;
        win.draw_png_scale(peas[i].directory_num, peas[i].x_location, y_location, PEA_WIDTH + more_px, PEA_HEIGHT + more_px);
    }
}

/*Updated
Change to sprite sheet
Add blink
*/
void display_sunflowers(window &win, vector<Sunflower> &sunflowers, Map &cells, bool is_pause)
{
    for (auto &sunflower : sunflowers)
    {
        if (sunflower.directory_num == SUNFLOWER_SHEET_DIRECTORY)
        {
            int col = sunflower.col;
            int row = sunflower.row;
            // win.draw_png_scale(SUNFLOWER_DIRECTORY, cells[row][col].x1 + 9, cells[row][col].y1 + 9, ELEMENT_WIDTH, ELEMENT_HEIGHT);

            int frame = sunflower.frame;
            int scol = frame % SUNFLOWER_C_SHEET;
            int srow = frame / SUNFLOWER_C_SHEET;
            win.draw_png(sunflower.directory_num, SUNFLOWER_WIDTH * scol, SUNFLOWER_HEIGHT * srow, SUNFLOWER_WIDTH, SUNFLOWER_HEIGHT, cells[row][col].x1, cells[row][col].y1 - 20, ELEMENT_WIDTH, ELEMENT_HEIGHT);

            if (is_pause == false)
                if (++sunflower.frame >= SUNFLOWER_N_SHEET)
                {
                    sunflower.frame = 0;
                }
            if (sunflower.is_attacked)
            {
                win.draw_png(sunflower.blink_directory_num, SUNFLOWER_WIDTH * scol, SUNFLOWER_HEIGHT * srow, SUNFLOWER_WIDTH, SUNFLOWER_HEIGHT, cells[row][col].x1, cells[row][col].y1 - 20, ELEMENT_WIDTH, ELEMENT_HEIGHT);
                sunflower.is_attacked--;
            }
        }
        else
        {
            int col = sunflower.col;
            int row = sunflower.row;
            // win.draw_png_scale(SUNFLOWER_DIRECTORY, cells[row][col].x1 + 9, cells[row][col].y1 + 9, ELEMENT_WIDTH, ELEMENT_HEIGHT);

            int frame = sunflower.frame / SUNFLOWER_F_SHEET;
            int scol = frame % SUNFLOWER_H_C_SHEET;
            int srow = frame / SUNFLOWER_H_C_SHEET;
            win.draw_png(sunflower.directory_num, SUNFLOWER_H_WIDTH * scol, SUNFLOWER_H_HEIGHT * srow, SUNFLOWER_H_WIDTH, SUNFLOWER_H_HEIGHT, cells[row][col].x1, cells[row][col].y1 - 20, ELEMENT_WIDTH, ELEMENT_HEIGHT);

            if (is_pause == false)
                if (++sunflower.frame >= SUNFLOWER_F_SHEET * SUNFLOWER_H_N_SHEET)
                {
                    sunflower.frame = 0;
                }
            if (sunflower.is_attacked)
            {
                win.draw_png(sunflower.blink_directory_num, SUNFLOWER_H_WIDTH * scol, SUNFLOWER_H_HEIGHT * srow, SUNFLOWER_H_WIDTH, SUNFLOWER_H_HEIGHT, cells[row][col].x1, cells[row][col].y1 - 20, ELEMENT_WIDTH, ELEMENT_HEIGHT);
                sunflower.is_attacked--;
            }
        }
    }
}

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

/* New function:
If any plant seed is chosen: render it at the mouse position.
*/
void display_chosen_plant(window &win, Player player, Icons icons)
{
    int _x = 0, _y = 0;
    SDL_GetMouseState(&_x, &_y);
    if (player.is_shoveling)
    {
        win.draw_png_scale(SHOVEL_DIRECTORY, _x, _y - ELEMENT_HEIGHT, ELEMENT_WIDTH, ELEMENT_HEIGHT);
        return;
    }
    _x -= ELEMENT_WIDTH >> 1;
    _y -= ELEMENT_HEIGHT >> 1;
    if (icons.is_peashooter_chosen)
    {
        win.draw_png_scale(PEASHOOTER_DIRECTORY, _x, _y, ELEMENT_WIDTH, ELEMENT_HEIGHT);
    }
    else if (icons.is_sunflower_chosen)
    {
        win.draw_png_scale(SUNFLOWER_DIRECTORY, _x, _y, ELEMENT_WIDTH, ELEMENT_HEIGHT);
    }
    else if (icons.is_walnut_chosen)
    {
        win.draw_png_scale(WALNUT_DIRECTORY, _x, _y, ELEMENT_WIDTH, ELEMENT_HEIGHT);
    }
}

void display_ready_set_plant(window &win, int image_num, Level &level)
{
    win.draw_bg(level.background_directory);
    win.draw_png(image_num, (WINDOW_WIDTH - READY_WIDTH) >> 1, (WINDOW_HEIGHT - READY_HEIGHT) >> 1, READY_WIDTH, READY_HEIGHT);
}
void display_shovel(window &win, const Button &button)
{
    win.draw_png(SHOVEL_BAR_DIRECTORY, 0, 0, SHOVEL_WIDTH, SHOVEL_HEIGHT, button.x1, button.y1, button.x2 - button.x1, button.y2 - button.y1);
}

void display_menu_icon(window &win, const Button &button)
{
    win.draw_png(MENU_ICON_DIRECTORY, 0, 0, button.x2 - button.x1, button.y2 - button.y1, button.x1, button.y1, button.x2 - button.x1, button.y2 - button.y1);
}

void display_menu(window &win, const Button &button)
{
    win.draw_png(MENU_DIRECTORY, 0, 0, button.x2 - button.x1, button.y2 - button.y1, button.x1, button.y1, button.x2 - button.x1, button.y2 - button.y1);
}

void display_level_is_locked(window &win, const Button &button)
{
    win.draw_png_scale(BLACK_SCREEN_DIRECTORY, button.x1, button.y1, button.x2 - button.x1, button.y2 - button.y1);
}
