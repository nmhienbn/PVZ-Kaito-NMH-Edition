#include "draw/render_elements.hpp"

extern Level level;
extern Elements game_characters;
extern Icons icons;
extern Player player;
extern Map cells;
extern window win;

/*
Draw the choosing level screen without handle the mouse-over event
(mouse-over event : the level blink and text turn into green)
*/
void render_choose_level_no_mouse()
{
    win.clear_renderer();
    win.draw_png_scale(CHOOSE_LEVELS_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    // Button
    display_button(RENAME_BUTTON, RENAME_DIRECTORY);
    display_button(RESET_LEVEL_BUTTON, RESET_LEVEL_DIRECTORY);
    display_button(QUIT_BUTTON, QUIT_DIRECTORY);
    // Text
    for (int i = 1; i <= LEVEL_COUNT; i++)
    {
        if (player.unlocked_level >= i)
        {
            win.show_text("Level " + to_string(i), LEVEL_BUTTON[i].x1 + 60, LEVEL_BUTTON[i].y1 + 130);
        }
        else
        {
            win.show_text("Level " + to_string(i), LEVEL_BUTTON[i].x1 + 60, LEVEL_BUTTON[i].y1 + 130, BLACK);

            display_level_is_locked(LEVEL_BUTTON[i]);
        }
    }
}

/*
Display game layout:
    + Background (playground).
    + Sun and Player's sun count.
    + Shovel.
    + Plant seeds (Chosen or not).
    + Menu icon.
*/
void display_game_layout()
{
    // Background
    win.draw_bg(BLACK_SCREEN_DIRECTORY);
    win.draw_bg(level.background_directory);

    // Sun bar and player's sun count
    win.draw_png_scale(SUN_BAR_DIRECTORY, 5, 5, SUN_BAR_WIDTH, SUN_BAR_HEIGHT);
    win.show_text(std::to_string(player.sun_count), 90, 33, (player.sun_count_change_color_times & 1 ? RED : BLACK), "contm.ttf");

    // Shovel
    display_button(Shovel_bar, SHOVEL_BAR_DIRECTORY);
    Shovel_bar.blink();

    // Plant seed
    display_icons_in_icon_bar();

    // Menu icon (to show pause menu)
    display_button(MENU_ICON, MENU_ICON_DIRECTORY);
}

/*
Display the plants' seed:
    + Player hasn't unlocked yet: no display.
    + Display plant seed bar base on the number of plant.
    + Dim if player doesn't have enough sun or it is chosen.
    + Render a black screen to display remaining time for it to refresh.
    + Normal if player have enough sun.
*/
void display_icons_in_icon_bar()
{
    int plant_seed_dir[] = {
        PEASHOOTER_ICON_BRIGHT_DIRECTORY,
        SUNFLOWER_ICON_BRIGHT_DIRECTORY,
        WALNUT_ICON_BRIGHT_DIRECTORY,
        SNOWPEA_ICON_BRIGHT_DIRECTORY,
        CHERRYBOMB_ICON_BRIGHT_DIRECTORY,
    };
    // Count number of unlocked plant.
    int num_plants = 0;
    for (int i = 0; i < PLANT_COUNT; i++)
    {
        if (level.level_num >= level_unlock_new_plant[i])
        {
            num_plants = i;
        }
        else
        {
            break;
        }
    }
    // Plant seed bar
    win.draw_png(ICON_BAR_DIRECTORY, 15, 85, ICON_BAR_WIDTH, plant_seed[num_plants].y2 - 80);
    // Plant seed
    for (int i = 0; i <= num_plants; i++)
    {
        // Not enough sun or is chosen
        if (player.sun_count < plant_sun_cost[i] || icons.is_plant_chosen[i])
        {
            plant_seed_dir[i]++;
        }
        win.draw_png_scale(plant_seed_dir[i], plant_seed[i].x1, plant_seed[i].y1, ICON_WIDTH, ICON_HEIGHT);
        if (icons.plant_remaining_time[i] == 0)
        {
            if (icons.is_plant_chosen[i] == false)
                plant_seed[i].blink();
        }
        else
        {
            // Remaining time
            win.draw_png(BLACK_SCREEN_DIRECTORY, plant_seed[i].x1, plant_seed[i].y1, ICON_WIDTH,
                         icons.plant_remaining_time[i] * ICON_HEIGHT / plant_loading_time[i]);
        }
    }
}

/*
Display the game element: plants, zombies and the suns.
Updated: render order.
*/
void display_game_elements()
{
    display_peashooters(game_characters.peashooters);
    display_sunflowers(game_characters.sunflowers);
    display_walnuts(game_characters.walnuts);
    display_snowpeas(game_characters.snowpeas);
    display_cherrybombs(game_characters.cherrybombs);

    display_dead_zombies(game_characters.dead_zombies);
    display_zombies(game_characters.zombies);

    display_peas(game_characters.peas);
    display_suns(game_characters.suns);
}

/*
Display the game when paused.
*/
void display_game_paused_elements()
{
    display_game_layout();
    display_game_elements();
    win.draw_bg(BLACK_SCREEN_DIRECTORY);
}

/*
If any plant seed is chosen: render it at the mouse position.
*/
void display_chosen_plant()
{
    int _x = 0, _y = 0;
    SDL_GetMouseState(&_x, &_y);
    if (player.is_shoveling)
    {
        win.draw_png_scale(SHOVEL_DIRECTORY, _x, _y - ELEMENT_HEIGHT,
                           ELEMENT_WIDTH, ELEMENT_HEIGHT);
        return;
    }
    _x -= ELEMENT_WIDTH >> 1;
    _y -= ELEMENT_HEIGHT >> 1;
    for (int i = 0; i < PLANT_COUNT; i++)
    {
        if (icons.is_plant_chosen[i])
        {
            win.draw_png_scale(PEASHOOTER_DIRECTORY + i, _x, _y,
                               ELEMENT_WIDTH, ELEMENT_HEIGHT);
        }
    }
}

/*
Make row when column blink when player is
about to plant or shovel in that row/column.
*/
void blink_row_and_col()
{
    int _x, _y;
    SDL_GetMouseState(&_x, &_y);
    int right_bound = cells[0][8].x2;
    int left_bound = cells[0][0].x1;
    // Get level's row limit
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
    // If mouse is over a tile
    if (_x > left_bound && _x < right_bound &&
        _y > upper_bound && _y < lower_bound)
    {
        // Blink row
        for (int y = 0; y < VERT_BLOCK_COUNT; y++)
        {
            if (_y > cells[y][0].y1 && _y < cells[y][0].y2)
            {
                win.draw_png(front_yard_r + y, 0, 0,
                             WINDOW_WIDTH, WINDOW_HEIGHT);
                break;
            }
        }
        // Blink column
        for (int x = 0; x < HORIZ_BLOCK_COUNT; x++)
            if (_x > cells[0][x].x1 && _x < cells[0][x].x2)
            {
                win.draw_png(front_yard_c + x, 0, 0,
                             WINDOW_WIDTH, WINDOW_HEIGHT);
                break;
            }
    }
}