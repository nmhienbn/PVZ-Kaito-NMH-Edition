#include "draw/render_elements.hpp"
#define SUN_BAR_WIDTH 180
#define SUN_BAR_HEIGHT 88

extern Level level;
extern Elements game_characters;
extern Icons icons;
extern Player player;
extern Map cells;
extern window win;

/*Display choosing level screen:
    Rename player
    Reset level
    Quit game.
    These button will blink *if is_mouse_needed*
    Levels:
        Transparent black and black text if is locked.
        White text if is unlocked.
        If is_mouse_needed: Transparent white and green text if mouse is over.
*/
void display_choose_level(const bool &is_mouse_needed)
{
    win.clear_renderer();
    win.draw_png_scale(CHOOSE_LEVELS_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    // Buttons
    display_button(RENAME_BUTTON, RENAME_DIRECTORY);
    display_button(RESET_LEVEL_BUTTON, RESET_LEVEL_DIRECTORY);
    display_button(QUIT_BUTTON, QUIT_DIRECTORY);
    if (is_mouse_needed)
    {
        RENAME_BUTTON.blink();
        RESET_LEVEL_BUTTON.blink();
        QUIT_BUTTON.blink();
    }
    // Level appearance
    for (int i = 1; i <= LEVEL_COUNT; i++)
    {
        string lvl_txt = "LEVEL " + to_string(i);
        int w, h;
        TTF_SizeText(win.get_font(HOUSEOFTERROR_TTF, 40), lvl_txt.c_str(), &w, &h);
        int kc = (LEVEL_BUTTON[i].x2 - LEVEL_BUTTON[i].x1 - w) / 2;
        if (player.unlocked_level >= i)
        {
            if (is_mouse_needed)
            {
                int _x = 0, _y = 0;
                SDL_GetMouseState(&_x, &_y);
                if (LEVEL_BUTTON[i].is_mouse_in(_x, _y))
                {

                    win.show_text_shadowed(lvl_txt, LEVEL_BUTTON[i].x1 + kc, LEVEL_BUTTON[i].y1 + 120, GREEN, HOUSEOFTERROR_TTF, 40);
                    win.draw_png(WHITE_SCREEN_DIRECTORY, LEVEL_BUTTON[i].x1, LEVEL_BUTTON[i].y1,
                                 LEVEL_BUTTON[i].x2 - LEVEL_BUTTON[i].x1, LEVEL_BUTTON[i].y2 - LEVEL_BUTTON[i].y1);
                }
                else
                    win.show_text_shadowed(lvl_txt, LEVEL_BUTTON[i].x1 + kc, LEVEL_BUTTON[i].y1 + 120, WHITE, HOUSEOFTERROR_TTF, 40);
            }
            else
                win.show_text_shadowed(lvl_txt, LEVEL_BUTTON[i].x1 + kc, LEVEL_BUTTON[i].y1 + 120, WHITE, HOUSEOFTERROR_TTF, 40);
        }
        else
        {
            win.show_text("Level " + to_string(i), LEVEL_BUTTON[i].x1 + kc, LEVEL_BUTTON[i].y1 + 120, BLACK, HOUSEOFTERROR_TTF, 40);
            display_level_is_locked(LEVEL_BUTTON[i]);
            win.draw_png(LOCK_DIRECTORY, 0, 0, 69, 88, LEVEL_BUTTON[i].x2 - 25, LEVEL_BUTTON[i].y1 - 10, 35, 44);
        }
    }
}

/*Display game layout:
    + Background (playground).
    + Sun and Player's sun count.
    + Shovel. (and key to choose)
    + Plant seeds (Chosen or not).
    + Menu icon.
    + Game progress bar
*/
void display_game_layout()
{
    // Background
    win.draw_bg(BLACK_SCREEN_DIRECTORY);
    win.draw_bg(level.background_directory);

    // Sun bar and player's sun count
    win.draw_png_scale(SUN_BAR_DIRECTORY, 5, 5, SUN_BAR_WIDTH, SUN_BAR_HEIGHT);
    win.set_style(CONTM_TTF, 24, TTF_STYLE_BOLD);
    win.show_text(to_string(player.sun_count), 90, 33, (player.sun_count_change_color_times & 1 ? RED : BLACK), CONTM_TTF);

    // Shovel
    display_button(Shovel_bar, SHOVEL_BAR_DIRECTORY);
    Shovel_bar.blink();
    win.show_text_shadowed("S", Shovel_bar.x2 - 13, Shovel_bar.y1, WHITE, BRIANNE_TTF);

    // Plant seed
    display_icons_in_icon_bar();
    if (player.is_choosing_a_plant || player.is_shoveling)
    {
        blink_row_and_col();
    }
    display_game_elements();

    // Menu icon (to show pause menu)
    display_button(PAUSE_ICON, PAUSE_ICON_DIRECTORY);
    display_button(TURBO_ICON, TURBO_ICON_DIRECTORY);

    // Progress bar
    display_progress_bar();
}

/*Display the plants' seed:
    + Player hasn't unlocked yet: no display.
    + Display plant seed bar base on the number of plant.
    + Dim if player doesn't have enough sun or it is chosen.
    + Render a black screen to display remaining time for it to refresh.
    + Normal if player have enough sun.
    + Key to choose
*/
void display_icons_in_icon_bar()
{
    int plant_seed_dir[] = {
        PEASHOOTER_ICON_DIRECTORY,
        SUNFLOWER_ICON_DIRECTORY,
        WALNUT_ICON_DIRECTORY,
        SNOWPEA_ICON_DIRECTORY,
        CHERRYBOMB_ICON_DIRECTORY,
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
        win.draw_png_scale(plant_seed_dir[i], plant_seed[i].x1, plant_seed[i].y1, ICON_WIDTH, ICON_HEIGHT);
        // Not enough sun or is chosen
        if (player.sun_count < PLANT_SUN_COST[i] || icons.chosen_plant == i || icons.plant_remaining_time[i])
        {
            win.draw_png(BLACK_SCREEN_DIRECTORY, plant_seed[i].x1, plant_seed[i].y1, ICON_WIDTH, ICON_HEIGHT);
        }

        if (icons.plant_remaining_time[i] == 0)
        {
            if (icons.chosen_plant != i)
                plant_seed[i].blink();
            else
                win.draw_png_scale(SEED_CHOSEN_DIRECTORY, plant_seed[i].x1, plant_seed[i].y1, ICON_WIDTH, ICON_HEIGHT);
        }
        else
        {
            // Remaining time
            win.draw_png(BLACK_SCREEN_DIRECTORY, plant_seed[i].x1, plant_seed[i].y1, ICON_WIDTH,
                         icons.plant_remaining_time[i] * ICON_HEIGHT / PLANT_LOADING_TIME[i]);
        }
        win.show_text_shadowed(to_string(i + 1), plant_seed[i].x1 + ICON_WIDTH - 13, plant_seed[i].y1, WHITE, BRIANNE_TTF);

        int w, h;
        int fsize = 30;
        TTF_SizeText(win.get_font(PVZUI_TTF, fsize), to_string(PLANT_SUN_COST[i]).c_str(), &w, &h);
        w = plant_seed[i].x1 + ICON_WIDTH - 4 - w;
        h = plant_seed[i].y1 + ICON_HEIGHT - h;
        win.show_text(to_string(PLANT_SUN_COST[i]),
                      w + 2, h + 3, BLACK, PVZUI_TTF, fsize);
        win.show_text_shadowed(to_string(PLANT_SUN_COST[i]),
                               w, h, WHITE, PVZUI_TTF, fsize);
    }
}

/*Display shadow under: plants, zombies, bullets*/
void display_shadow()
{
    for (int row = 0; row < VERT_BLOCK_COUNT; row++)
    {
        for (int col = 0; col < HORIZ_BLOCK_COUNT; col++)
        {
            if (cells[row][col].is_planted)
            {
                win.draw_png(SHADOW_DIRECTORY, cells[row][col].x1 - 5, cells[row][col].y2 - 40, 86, 36);
            }
        }
    }
    for (const auto &zombie : game_characters.zombies)
    {
        win.draw_png(SHADOW_DIRECTORY, zombie->x_location + 85, cells[zombie->row][0].y2 - 40, 96, 40);
    }
    for (auto &bullet : game_characters.bullets)
    {
        bullet->display_shadow();
    }
}

/*Display the game element: plants, zombies and the suns.*/
void display_game_elements()
{
    // Shadows
    display_shadow();
    display_mowers(game_characters.mowers);

    for (int row = 0; row < VERT_BLOCK_COUNT; row++)
    {
        // Plants
        for (auto &plant : game_characters.plants)
            plant->display(row);

        // Zombies
        display_zombies(game_characters.zombies, row);
        display_zombie_parts(game_characters.zombie_parts, row);
    }
    // Others
    for (auto &bullet : game_characters.bullets)
        bullet->display();
    display_suns(game_characters.suns);
}

/*Display the game when paused.*/
void display_game_paused_elements()
{
    display_game_layout();
    display_game_elements();
    display_button(PAUSE_ICON, PAUSE_ICON_A_DIRECTORY);
    win.draw_bg(BLACK_SCREEN_DIRECTORY);
}

/*If any plant seed is chosen: render it (transparent) at the mouse position.*/
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
    if (PEASHOOTER_TYPE <= icons.chosen_plant && icons.chosen_plant < PLANT_COUNT)
    {
        win.draw_png_scale(PEASHOOTER_DIRECTORY + icons.chosen_plant, _x, _y,
                           ELEMENT_WIDTH, ELEMENT_HEIGHT);
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
        win.draw_png(front_yard_r + get_block_row(_y), 0, 0,
                     WINDOW_WIDTH, WINDOW_HEIGHT);
        // Blink column
        win.draw_png(front_yard_c + get_block_col(_x), 0, 0,
                     WINDOW_WIDTH, WINDOW_HEIGHT);
    }
}