#include "render_elements.h"
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

    win.show_text(std::to_string(player.sun_count), 90, 33, BLACK, "contm.ttf");
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
    int snowpea_icon = SNOWPEA_ICON_BRIGHT_DIRECTORY;

    if (player.sun_count < 100 || icons.is_peashooter_chosen)
        peashooter_icon = PEASHOOTER_ICON_DIM_DIRECTORY;
    if (player.sun_count < 50 || icons.is_sunflower_chosen)
        sunflower_icon = SUNFLOWER_ICON_DIM_DIRECTORY;
    if (player.sun_count < 50 || icons.is_walnut_chosen)
        walnut_icon = WALNUT_ICON_DIM_DIRECTORY;
    if (player.sun_count < 150 || icons.is_snowpea_chosen)
        snowpea_icon = SNOWPEA_ICON_DIM_DIRECTORY;

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

    if (level.level_num >= 4)
    {
        win.draw_png_scale(snowpea_icon, ICON_BAR_X1 + 3, SNOWPEA_ICON_Y1, ICON_WIDTH, ICON_HEIGHT);
        win.draw_png(BLACK_SCREEN_DIRECTORY, ICON_BAR_X1 + 3, SNOWPEA_ICON_Y1, ICON_WIDTH, icons.snowpea_remaining_time * ICON_HEIGHT / SNOWPEA_LOADING);
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
    display_snowpeas(win, elements.snowpeas, cells);

    display_dead_zombies(win, elements.dead_zombies, cells);
    display_zombies(win, elements.zombies, cells);

    display_peas(win, elements.peas, cells);
    display_suns(win, elements.suns, cells);
}

/*
Display the game element when paused: plants, zombies and the suns.
*/
void display_game_paused_elements(window &win, Elements &elements, Map &cells, Level &level)
{
    display_zombies(win, elements.zombies, cells, true);
    display_dead_zombies(win, elements.dead_zombies, cells, true);
    display_peashooters(win, elements.peashooters, cells, true);
    display_sunflowers(win, elements.sunflowers, cells, true);
    display_walnuts(win, elements.walnuts, cells, true);
    display_snowpeas(win, elements.snowpeas, cells, true);
    display_peas(win, elements.peas, cells);
    win.draw_bg(BLACK_SCREEN_DIRECTORY);
}

void display_game_pause(window &win, Elements &elements, Map &cells, Level &level)
{
    win.draw_bg(level.background_directory);
    display_game_paused_elements(win, elements, cells, level);
    display_menu(win);
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
    else if (icons.is_snowpea_chosen)
    {
        win.draw_png_scale(SNOWPEA_DIRECTORY, _x, _y, ELEMENT_WIDTH, ELEMENT_HEIGHT);
    }
}

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Display Game Buttons
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

/*
Display Shovel bar
*/
void display_shovel(window &win, const Button &button)
{
    win.draw_png(SHOVEL_BAR_DIRECTORY, 0, 0, SHOVEL_WIDTH, SHOVEL_HEIGHT, button.x1, button.y1, button.x2 - button.x1, button.y2 - button.y1);
}

/*
Display in-game Menu Icon
*/
void display_menu_icon(window &win, const Button &button)
{
    win.draw_png(MENU_ICON_DIRECTORY, 0, 0, button.x2 - button.x1, button.y2 - button.y1, button.x1, button.y1, button.x2 - button.x1, button.y2 - button.y1);
}

/*
Display paused in-game Menu
*/
void display_menu(window &win, const Button &button)
{
    win.draw_png(MENU_DIRECTORY, 0, 0, button.x2 - button.x1, button.y2 - button.y1, button.x1, button.y1, button.x2 - button.x1, button.y2 - button.y1);
}

/*
Display transparent black when level is locked
*/
void display_level_is_locked(window &win, const Button &button)
{
    win.draw_png_scale(BLACK_SCREEN_DIRECTORY, button.x1, button.y1, button.x2 - button.x1, button.y2 - button.y1);
}
