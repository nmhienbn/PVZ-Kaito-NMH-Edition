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
    // win.draw_png_scale(ICON_BAR_DIRECTORY, 20, 100, ICON_BAR_WIDTH, ICON_BAR_HEIGHT);
    win.draw_png_scale(SUN_DIRECTORY, 5, 5, SUN_WIDTH, SUN_HEIGHT);
    win.draw_png_scale(SUN_COUNT_TEXT_FIELD_DIRECTORY, 85, 33, SUN_COUNT_WIDTH, SUN_COUNT_HEIGHT);
    win.show_text(std::to_string(player.sun_count), 100, 33);
    display_icons_in_icon_bar(icons, player, win);
}

/*
Display the plants' seed:
    + Dim if player doesn't have enough sun or it has not refresh.
    + Normal if player have enough sun.
*/
void display_icons_in_icon_bar(Icons icons, Player player, window &win)
{
    string peashooter_icon = PEASHOOTER_ICON_BRIGHT_DIRECTORY;
    string sunflower_icon = SUNFLOWER_ICON_BRIGHT_DIRECTORY;
    string walnut_icon = WALNUT_ICON_BRIGHT_DIRECTORY;

    if (player.sun_count < 100)
        peashooter_icon = PEASHOOTER_ICON_DIM_DIRECTORY;
    if (player.sun_count < 50)
        sunflower_icon = SUNFLOWER_ICON_DIM_DIRECTORY;
    if (player.sun_count < 50)
        walnut_icon = WALNUT_ICON_DIM_DIRECTORY;

    win.draw_png_scale(sunflower_icon, ICON_BAR_X1 + 3, SUNFLOWER_ICON_Y1, ICON_WIDTH, ICON_HEIGHT);
    win.draw_png(BLACK_SCREEN_DIRECTORY, ICON_BAR_X1 + 3, SUNFLOWER_ICON_Y1, ICON_WIDTH, icons.sunflower_remaining_time * ICON_HEIGHT / SUNFLOWER_LOADING);

    win.draw_png_scale(peashooter_icon, ICON_BAR_X1 + 3, PEASHOOTER_ICON_Y1, ICON_WIDTH, ICON_HEIGHT);
    win.draw_png(BLACK_SCREEN_DIRECTORY, ICON_BAR_X1 + 3, PEASHOOTER_ICON_Y1, ICON_WIDTH, icons.peashooter_remaining_time * ICON_HEIGHT / PEASHOOTER_LOADING);

    win.draw_png_scale(walnut_icon, ICON_BAR_X1 + 3, WALNUT_ICON_Y1, ICON_WIDTH, ICON_HEIGHT);
    win.draw_png(BLACK_SCREEN_DIRECTORY, ICON_BAR_X1 + 3, WALNUT_ICON_Y1, ICON_WIDTH, icons.walnut_remaining_time * ICON_HEIGHT / WALNUT_LOADING);
    return;
}

/*
Display the game element: plants, zombies and the suns.
*/
void display_game_elements(window &win, Elements &elements, Map &map)
{
    display_zombies(win, elements.zombies, map);
    display_peashooters(win, elements.peashooters, map);
    display_sunflowers(win, elements.sunflowers, map);
    display_walnuts(win, elements.walnuts, map);
    display_peas(win, elements.peas, map);
    display_suns(win, elements.suns, map);
}

/*
If player lose, display this.
*/
void display_losing_message(window &win)
{
    win.draw_bg(BLACK_SCREEN_DIRECTORY);
    win.draw_bg(BACKGROUND_DIM_DIRECTORY);
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

void display_zombies(window &win, vector<Zombie> &zombies, Map &map)
{
    for (int i = 0; i < zombies.size(); i++)
    {
        int row = zombies[i].row;
        int y_location = map[row][0].y1;
        win.draw_png_scale(zombies[i].directory, zombies[i].x_location, y_location, ELEMENT_WIDTH, ELEMENT_HEIGHT);
    }
}

void display_peashooters(window &win, vector<Peashooter> &peashooters, Map &map)
{
    for (int i = 0; i < peashooters.size(); i++)
    {
        int col = peashooters[i].col;
        int row = peashooters[i].row;
        win.draw_png_scale(PEASHOOTER_DIRECTORY, map[row][col].x1 + 9, map[row][col].y1 + 9, ELEMENT_WIDTH, ELEMENT_HEIGHT);
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

void display_sunflowers(window &win, vector<Sunflower> &sunflowers, Map &map)
{
    for (int i = 0; i < sunflowers.size(); i++)
    {
        int col = sunflowers[i].col;
        int row = sunflowers[i].row;
        win.draw_png_scale(SUNFLOWER_DIRECTORY, map[row][col].x1 + 9, map[row][col].y1 + 9, ELEMENT_WIDTH, ELEMENT_HEIGHT);
    }
}

void display_walnuts(window &win, vector<Walnut> &walnuts, Map &map)
{
    for (int i = 0; i < walnuts.size(); i++)
    {
        int col = walnuts[i].col;
        int row = walnuts[i].row;
        win.draw_png_scale(walnuts[i].directory, map[row][col].x1 + 9, map[row][col].y1 + 9, ELEMENT_WIDTH, ELEMENT_HEIGHT);
    }
}
