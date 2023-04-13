#include "draw/render_elements.hpp"
#define SUN_BAR_WIDTH 180
#define SUN_BAR_HEIGHT 360

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
    display_button(RENAME_BUTTON, RENAME_DIRECTORY);
    display_button(RESET_LEVEL_BUTTON, RESET_LEVEL_DIRECTORY);
    display_button(QUIT_BUTTON, QUIT_DIRECTORY);
    if (is_mouse_needed)
    {
        RENAME_BUTTON.blink();
        RESET_LEVEL_BUTTON.blink();
        QUIT_BUTTON.blink();
    }

    for (int i = 1; i <= LEVEL_COUNT; i++)
    {
        string lvl_txt = "Level " + to_string(i);
        int w, h;
        TTF_SizeText(win.get_font(FREESANS_TTF, 24), lvl_txt.c_str(), &w, &h);
        int kc = (LEVEL_BUTTON[i].x2 - LEVEL_BUTTON[i].x1 - w) / 2;
        if (player.unlocked_level >= i)
        {
            if (is_mouse_needed)
            {
                int _x = 0, _y = 0;
                SDL_GetMouseState(&_x, &_y);
                if (LEVEL_BUTTON[i].is_mouse_in(_x, _y))
                {

                    win.show_text_shadowed(lvl_txt, LEVEL_BUTTON[i].x1 + kc, LEVEL_BUTTON[i].y1 + 130, GREEN);
                    win.draw_png(WHITE_SCREEN_DIRECTORY, LEVEL_BUTTON[i].x1, LEVEL_BUTTON[i].y1,
                                 LEVEL_BUTTON[i].x2 - LEVEL_BUTTON[i].x1, LEVEL_BUTTON[i].y2 - LEVEL_BUTTON[i].y1);
                }
                else
                    win.show_text_shadowed(lvl_txt, LEVEL_BUTTON[i].x1 + kc, LEVEL_BUTTON[i].y1 + 130, WHITE);
            }
            else
                win.show_text_shadowed(lvl_txt, LEVEL_BUTTON[i].x1 + kc, LEVEL_BUTTON[i].y1 + 130, WHITE);
        }
        else
        {
            win.show_text("Level " + to_string(i), LEVEL_BUTTON[i].x1 + kc, LEVEL_BUTTON[i].y1 + 130, BLACK);

            display_level_is_locked(LEVEL_BUTTON[i]);
        }
    }
}

/*
Display game layout:
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
    display_button(MENU_ICON, MENU_ICON_DIRECTORY);

    // Progress bar
    display_progress_bar();
}

/*
Display the plants' seed:
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
        if (player.sun_count < plant_sun_cost[i] || icons.is_plant_chosen[i] || icons.plant_remaining_time[i])
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
        win.show_text_shadowed(to_string(i + 1), plant_seed[i].x1 + ICON_WIDTH - 13, plant_seed[i].y1, WHITE, BRIANNE_TTF);
    }
}

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
    for (auto &zombie : game_characters.zombies)
    {
        win.draw_png(SHADOW_DIRECTORY, zombie.x_location + 70, cells[zombie.row][0].y2 - 40, 96, 40);
    }
    for (auto &pea : game_characters.peas)
    {
        win.draw_png(PEA_SHADOW_DIRECTORY, pea.x_location, cells[pea.row][0].y2 - 13, 21, 9);
    }
}

/*
Display the game element: plants, zombies and the suns.
*/
void display_game_elements()
{
    // Shadows
    display_shadow();

    for (int row = 0; row < VERT_BLOCK_COUNT; row++)
    {
        // Plants
        display_peashooters(game_characters.peashooters, row);
        display_sunflowers(game_characters.sunflowers, row);
        display_walnuts(game_characters.walnuts, row);
        display_snowpeas(game_characters.snowpeas, row);
        display_cherrybombs(game_characters.cherrybombs, row);

        // Zombies
        display_dead_zombies(game_characters.dead_zombies, row);
        display_zombies(game_characters.zombies, row);

        // Mowers
        display_mower(row);
    }
    // Others
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
If any plant seed is chosen: render it (transparent) at the mouse position.
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