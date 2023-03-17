#include "draw_elements.h"
/*
Display game layout:
    + Background (playground)
    + Sun and Player's Sun Count.
    + Plants' seed (Chosen or not).
*/
void display_game_layout(window &win, Player player, Icons icons)
{
    win.draw_bg(BLACK_SCREEN_DIRECTORY);
    win.draw_bg(BACKGROUND_DIRECTORY);
    win.draw_png_scale(ICON_BAR_DIRECTORY, 15, 85, ICON_BAR_WIDTH, ICON_BAR_HEIGHT);
    win.draw_png_scale(SUN_BAR_DIRECTORY, 5, 5, SUN_BAR_WIDTH, SUN_BAR_HEIGHT);
    display_icons_in_icon_bar(icons, player, win);
    display_chosen_plant(win, player, icons);
    win.show_text(std::to_string(player.sun_count), 100, 33);
}

/*
Display the plants' seed:
    + Dim if player doesn't have enough sun or it is chosen. - Updated
    + Render a black screen to display remaining time for it to refresh. - Updated
    + Normal if player have enough sun.
*/
void display_icons_in_icon_bar(Icons icons, Player player, window &win)
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

    win.draw_png_scale(sunflower_icon, ICON_BAR_X1 + 3, SUNFLOWER_ICON_Y1, ICON_WIDTH, ICON_HEIGHT);
    win.draw_png(BLACK_SCREEN_DIRECTORY, ICON_BAR_X1 + 3, SUNFLOWER_ICON_Y1, ICON_WIDTH, icons.sunflower_remaining_time * ICON_HEIGHT / SUNFLOWER_LOADING);

    win.draw_png_scale(peashooter_icon, ICON_BAR_X1 + 3, PEASHOOTER_ICON_Y1, ICON_WIDTH, ICON_HEIGHT);
    win.draw_png(BLACK_SCREEN_DIRECTORY, ICON_BAR_X1 + 3, PEASHOOTER_ICON_Y1, ICON_WIDTH, icons.peashooter_remaining_time * ICON_HEIGHT / PEASHOOTER_LOADING);

    win.draw_png_scale(walnut_icon, ICON_BAR_X1 + 3, WALNUT_ICON_Y1, ICON_WIDTH, ICON_HEIGHT);
    win.draw_png(BLACK_SCREEN_DIRECTORY, ICON_BAR_X1 + 3, WALNUT_ICON_Y1, ICON_WIDTH, icons.walnut_remaining_time * ICON_HEIGHT / WALNUT_LOADING);
}

/*
Display the game element: plants, zombies and the suns.
*/
void display_game_elements(window &win, Elements &elements, Map &map)
{
    display_peashooters(win, elements.peashooters, map);
    display_sunflowers(win, elements.sunflowers, map);
    display_walnuts(win, elements.walnuts, map);

    display_zombies(win, elements.zombies, map);
    display_dead_zombies(win, elements.dead_zombies, map);

    display_peas(win, elements.peas, map);
    display_suns(win, elements.suns, map);
}

/*
If player lose, display this.
*/
void display_losing_message(window &win, Elements &elements, Map &map)
{
    win.draw_bg(BACKGROUND_DIRECTORY);
    display_zombies(win, elements.zombies, map);
    display_peashooters(win, elements.peashooters, map);
    display_sunflowers(win, elements.sunflowers, map);
    display_walnuts(win, elements.walnuts, map);
    display_peas(win, elements.peas, map);
    win.draw_bg(BLACK_SCREEN_DIRECTORY);
    win.draw_png_scale(LOSING_MESSAGE_DIRECTORY, WINDOW_WIDTH / 4, WINDOW_HEIGHT / 8 - 40, 500, 500);
}

/*
If player win, display this
*/
void display_winning_message(window &win)
{
    win.draw_bg(BLACK_SCREEN_DIRECTORY);
    win.draw_bg(BACKGROUND_DIM_DIRECTORY);
    win.draw_png_scale(WINNING_MESSAGE_DIRECTORY, WINDOW_WIDTH / 3, WINDOW_HEIGHT / 8 - 60, 300, 550);
}

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Display Game Elements
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

void display_suns(window &win, vector<Sun> suns, Map &map)
{
    for (int i = 0; i < suns.size(); i++)
    {
        int col = suns[i].final_col;
        int x_location = map[0][col].x1;
        win.draw_png_scale(SUN_DIRECTORY, x_location, suns[i].y_location, ELEMENT_WIDTH, ELEMENT_HEIGHT);
    }
}

/*Updated
Change to sprite sheet
Add blink.
Add eating and dead status.
*/
void display_zombies(window &win, vector<Zombie> &zombies, Map &map)
{
    for (int i = 0; i < zombies.size(); i++)
    {
        int row = zombies[i].row;
        int y_location = map[row][0].y1 - 20;
        // win.draw_png_scale(zombies[i].directory_num, zombies[i].x_location, y_location, ELEMENT_WIDTH, ELEMENT_HEIGHT);
        int frame = zombies[i].frame / ZOMBIE_FRAME;
        int scol = frame % zombies[i].c_sheet;
        int srow = frame / zombies[i].c_sheet;
        win.draw_png(zombies[i].directory_num, ZOMBIE_WIDTH * scol, ZOMBIE_HEIGHT * srow, ZOMBIE_WIDTH, ZOMBIE_HEIGHT, zombies[i].x_location, y_location, ZOMBIE_G_WIDTH, ZOMBIE_G_HEIGHT);
        if (zombies[i].is_attacked)
        {
            win.draw_png(zombies[i].blink_directory_num, ZOMBIE_WIDTH * scol, ZOMBIE_HEIGHT * srow, ZOMBIE_WIDTH, ZOMBIE_HEIGHT, zombies[i].x_location, y_location, ZOMBIE_G_WIDTH, ZOMBIE_G_HEIGHT);
            zombies[i].is_attacked--;
        }
        if (++zombies[i].frame >= ZOMBIE_FRAME * zombies[i].n_sheet)
        {
            zombies[i].frame = 0;
        }
    }
}

/*Updated
Change to sprite sheet
*/
void display_peashooters(window &win, vector<Peashooter> &peashooters, Map &map)
{
    for (int i = 0; i < peashooters.size(); i++)
    {
        int col = peashooters[i].col;
        int row = peashooters[i].row;
        int frame = peashooters[i].frame / 4;
        int scol = frame % PEASHOOTER_C_SHEET;
        int srow = frame / PEASHOOTER_C_SHEET;
        win.draw_png(PEASHOOTER_SHEET_DIRECTORY, PEASHOOTER_WIDTH * scol, PEASHOOTER_HEIGHT * srow, PEASHOOTER_WIDTH, PEASHOOTER_HEIGHT, map[row][col].x1 + 9, map[row][col].y1 + 9, ELEMENT_WIDTH, ELEMENT_HEIGHT);
        if (++peashooters[i].frame >= 4 * PEASHOOTER_SHEET)
        {
            peashooters[i].frame = 0;
        }
    }
}

void display_peas(window &win, vector<Pea> &peas, Map &map)
{
    for (int i = 0; i < peas.size(); i++)
    {
        int row = peas[i].row;
        int y_location = map[row][0].y1 + 20;
        if (peas[i].x_location < (map[0][8].x2 - 35))
            win.draw_png_scale(PEA_DIRECTORY, peas[i].x_location, y_location, PEA_WIDTH, PEA_HEIGHT);
    }
}

/*Updated
Change to sprite sheet
Add blink
*/
void display_sunflowers(window &win, vector<Sunflower> &sunflowers, Map &map)
{
    for (int i = 0; i < sunflowers.size(); i++)
    {
        int col = sunflowers[i].col;
        int row = sunflowers[i].row;
        // win.draw_png_scale(SUNFLOWER_DIRECTORY, map[row][col].x1 + 9, map[row][col].y1 + 9, ELEMENT_WIDTH, ELEMENT_HEIGHT);

        int frame = sunflowers[i].frame / 8;
        int scol = frame % SUNFLOWER_C_SHEET;
        int srow = frame / SUNFLOWER_C_SHEET;
        win.draw_png(SUNFLOWER_SHEET_DIRECTORY, SUNFLOWER_WIDTH * scol, SUNFLOWER_HEIGHT * srow, SUNFLOWER_WIDTH, SUNFLOWER_HEIGHT, map[row][col].x1 + 9, map[row][col].y1 + 9, ELEMENT_WIDTH, ELEMENT_HEIGHT);
        if (++sunflowers[i].frame >= 8 * SUNFLOWER_SHEET)
        {
            sunflowers[i].frame = 0;
        }
        if (sunflowers[i].is_attacked)
        {
            win.draw_png(SUNFLOWER_SHEET_BLINK_DIRECTORY, SUNFLOWER_WIDTH * scol, SUNFLOWER_HEIGHT * srow, SUNFLOWER_WIDTH, SUNFLOWER_HEIGHT, map[row][col].x1 + 9, map[row][col].y1 + 9, ELEMENT_WIDTH, ELEMENT_HEIGHT);
            sunflowers[i].is_attacked--;
        }
    }
}
/*
updated: change sprite sheet
add blink
*/
void display_walnuts(window &win, vector<Walnut> &walnuts, Map &map)
{
    for (int i = 0; i < walnuts.size(); i++)
    {
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
        win.draw_png(walnuts[i].directory_num, WALNUT_WIDTH * scol, WALNUT_HEIGHT * srow, WALNUT_WIDTH, WALNUT_HEIGHT, map[row][col].x1 - 5, map[row][col].y1 + 9, ELEMENT_WIDTH, ELEMENT_HEIGHT);

        if (walnuts[i].is_attacked)
        {
            win.draw_png(walnuts[i].blink_directory_num, WALNUT_WIDTH * scol, WALNUT_HEIGHT * srow, WALNUT_WIDTH, WALNUT_HEIGHT, map[row][col].x1 - 5, map[row][col].y1 + 9, ELEMENT_WIDTH, ELEMENT_HEIGHT);
            walnuts[i].is_attacked--;
        }
        ++walnuts[i].frame;
        // if (++walnuts[i].frame >= 8 * WALNUT_SHEET)
        // {
        //     walnuts[i].frame = 0;
        // }
        // win.draw_png_scale(walnuts[i].directory_num, map[row][col].x1 + 9, map[row][col].y1 + 9, ELEMENT_WIDTH, ELEMENT_HEIGHT);
    }
}

/* New function:

*/
void display_chosen_plant(window &win, Player player, Icons icons)
{
    int _x = 0, _y = 0;
    SDL_GetMouseState(&_x, &_y);
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

void display_dead_zombies(window &win, vector<DeadZombie> &dead_zombies, Map &map)
{
    for (int i = 0; i < dead_zombies.size(); i++)
    {
        int row = dead_zombies[i].row;
        int y_location = map[row][0].y1 - 20;
        // win.draw_png_scale(zombies[i].directory_num, zombies[i].x_location, y_location, ELEMENT_WIDTH, ELEMENT_HEIGHT);
        int frame = dead_zombies[i].frame / 10;
        if (frame <= HEAD_ZOMBIE_N_SHEET)
        {
            int scol = (frame >> 1) % HEAD_ZOMBIE_C_SHEET;
            int srow = dead_zombies[i].type_head;
            win.draw_png(ZOMBIE_HEAD_DIRECTORY, HEAD_ZOMBIE_WIDTH * scol, HEAD_ZOMBIE_HEIGHT * srow, HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT, dead_zombies[i].x_location, y_location, HEAD_ZOMBIE_G_WIDTH, HEAD_ZOMBIE_G_HEIGHT);
        }
        int scol = frame % DEAD_ZOMBIE_C_SHEET;
        int srow = frame / DEAD_ZOMBIE_C_SHEET;
        win.draw_png(ZOMBIE_DIE_DIRECTORY, DEAD_ZOMBIE_WIDTH * scol, DEAD_ZOMBIE_HEIGHT * srow, DEAD_ZOMBIE_WIDTH, DEAD_ZOMBIE_HEIGHT, dead_zombies[i].x_location, y_location, DEAD_ZOMBIE_WIDTH, DEAD_ZOMBIE_HEIGHT);

        if (++dead_zombies[i].frame >= 10 * DEAD_ZOMBIE_N_SHEET)
        {

            dead_zombies.erase(dead_zombies.begin() + i);
        }
    }
}