#include "render_elements.h"

void render_choose_level_no_mouse(window &win, Player &player)
{
    win.clear_renderer();
    win.draw_png_scale(CHOOSE_LEVELS_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    display_button(win, RENAME_BUTTON, RENAME_DIRECTORY);
    display_button(win, RESET_LEVEL_BUTTON, RESET_LEVEL_DIRECTORY);
    display_button(win, QUIT_BUTTON, QUIT_DIRECTORY);

    for (int i = 1; i <= LEVEL_COUNT; i++)
    {
        if (player.unlocked_level >= i)
        {
            win.show_text("Level " + to_string(i), LEVEL_BUTTON[i].x1 + 60, LEVEL_BUTTON[i].y1 + 130);
        }
        else
        {
            win.show_text("Level " + to_string(i), LEVEL_BUTTON[i].x1 + 60, LEVEL_BUTTON[i].y1 + 130, BLACK);

            display_level_is_locked(win, LEVEL_BUTTON[i]);
        }
    }
}
/*
Display game layout:
    + Background (playground)
    + Sun and Player's Sun Count.
    + Plants' seed (Chosen or not).
*/
void display_game_layout(window &win, Player &player, Icons &icons, Level &level)
{
    win.draw_bg(BLACK_SCREEN_DIRECTORY);
    win.draw_bg(level.background_directory);
    win.draw_png_scale(ICON_BAR_DIRECTORY, 15, 85, ICON_BAR_WIDTH, ICON_BAR_HEIGHT);
    win.draw_png_scale(SUN_BAR_DIRECTORY, 5, 5, SUN_BAR_WIDTH, SUN_BAR_HEIGHT);
    display_button(win, Shovel_bar, SHOVEL_BAR_DIRECTORY);
    Shovel_bar.blink(win);
    display_icons_in_icon_bar(icons, player, win, level);
    display_button(win, MENU_ICON, MENU_ICON_DIRECTORY);

    win.show_text(std::to_string(player.sun_count), 90, 33, BLACK, "contm.ttf");
}

/*
Display the plants' seed:
    + Dim if player doesn't have enough sun or it is chosen. - Updated
    + Render a black screen to display remaining time for it to refresh. - Updated
    + Normal if player have enough sun.
*/
void display_icons_in_icon_bar(Icons &icons, Player &player, window &win, Level &level)
{
    int plant_seed_dir[] = {
        PEASHOOTER_ICON_BRIGHT_DIRECTORY,
        SUNFLOWER_ICON_BRIGHT_DIRECTORY,
        WALNUT_ICON_BRIGHT_DIRECTORY,
        SNOWPEA_ICON_BRIGHT_DIRECTORY,
        CHERRYBOMB_ICON_BRIGHT_DIRECTORY,
    };
    for (int i = 0; i < PLANT_COUNT; i++)
    {
        if (player.sun_count < plant_sun_cost[i] || icons.is_plant_chosen[i])
        {
            plant_seed_dir[i]++;
        }
        if (level.level_num >= level_unlock_new_plant[i])
        {
            win.draw_png_scale(plant_seed_dir[i], plant_seed[i].x1, plant_seed[i].y1, ICON_WIDTH, ICON_HEIGHT);
            if (icons.plant_remaining_time[i] == 0)
            {
                if (icons.is_plant_chosen[i] == false)
                    plant_seed[i].blink(win);
            }
            else
            {
                win.draw_png(BLACK_SCREEN_DIRECTORY, plant_seed[i].x1, plant_seed[i].y1, ICON_WIDTH,
                             icons.plant_remaining_time[i] * ICON_HEIGHT / plant_loading_time[i]);
            }
        }
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
    display_cherrybombs(win, elements.cherrybombs, cells);

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
    display_cherrybombs(win, elements.cherrybombs, cells, true);
    display_peas(win, elements.peas, cells);
    win.draw_bg(BLACK_SCREEN_DIRECTORY);
}

/* New function:
If any plant seed is chosen: render it at the mouse position.
*/
void display_chosen_plant(window &win, Player &player, Icons &icons)
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
    for (int i = 0; i < PLANT_COUNT; i++)
    {
        if (icons.is_plant_chosen[i])
        {
            win.draw_png_scale(PEASHOOTER_DIRECTORY + i, _x, _y, ELEMENT_WIDTH, ELEMENT_HEIGHT);
        }
    }
}

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Display Game Buttons
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

void display_button(window &win, const Button &button, const int &button_directory)
{
    win.draw_png(button_directory, button.x1, button.y1, button.x2 - button.x1, button.y2 - button.y1);
}

/*
Display transparent black when level is locked
*/
void display_level_is_locked(window &win, const Button &button)
{
    win.draw_png_scale(BLACK_SCREEN_DIRECTORY, button.x1, button.y1, button.x2 - button.x1, button.y2 - button.y1);
}

void blink_row_and_col(window &win, Map &cells, Level &level)
{
    int _x, _y;
    SDL_GetMouseState(&_x, &_y);
    int right_bound = cells[0][8].x2;
    int left_bound = cells[0][0].x1;
    int upper_bound;
    int lower_bound;
    if (level.level_num == 1)
    {
        upper_bound = cells[2][0].y1;
        lower_bound = cells[2][0].y2;
    }
    else if (level.level_num == 2)
    {
        upper_bound = cells[1][0].y1;
        lower_bound = cells[3][0].y2;
    }
    else
    {
        upper_bound = cells[0][0].y1;
        lower_bound = cells[4][0].y2;
    }
    if (_x > left_bound && _x < right_bound &&
        _y > upper_bound && _y < lower_bound)
    {
        win.set_texture_alpha(WHITE_SCREEN_DIRECTORY, 120);
        // Blink row
        for (int y = 0; y < VERT_BLOCK_COUNT; y++)
        {
            if (_y > cells[y][0].y1 && _y < cells[y][0].y2)
            {
                win.draw_png(WHITE_SCREEN_DIRECTORY, cells[y][0].x1, cells[y][0].y1, WINDOW_WIDTH - cells[y][0].x1, BLOCK_HEIGHT);
                break;
            }
        }
        // Blink column
        for (int x = 0; x < HORIZ_BLOCK_COUNT; x++)
            if (_x > cells[0][x].x1 && _x < cells[0][x].x2)
            {
                win.draw_png(WHITE_SCREEN_DIRECTORY, cells[0][x].x1, cells[0][x].y1, BLOCK_WIDTH, cells[VERT_BLOCK_COUNT - 1][x].y2 - cells[0][x].y1);
                break;
            }

        win.set_texture_alpha(WHITE_SCREEN_DIRECTORY, 50);
    }
}