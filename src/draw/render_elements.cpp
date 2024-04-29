#include "draw/render_elements.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/Player/Player.hpp"
#include "elements/button/button.hpp"
#include "elements/elements.hpp"
#include "elements/mower/mower.hpp"
#include "elements/progress_bar/progress_bar.hpp"
#include "elements/zombies/zombie.hpp"
#include <deque>
using namespace std;
#define SUN_BAR_WIDTH 180
#define SUN_BAR_HEIGHT 88

extern Level level;
extern Elements game_characters;

extern Player player;
extern Map cells;
extern Window win;
extern deque<int> shown_level;

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
    win.draw_png_height_scaled(SUN_BAR_DIRECTORY, 5, 5, SUN_BAR_WIDTH);
    win.set_style(PVZUI_TTF, 26, TTF_STYLE_BOLD);
    win.show_text(to_string(player.sun_count), 100, 17,
                  (player.sun_count_change_color_times & 1 ? RED : BLACK),
                  PVZUI_TTF, 30);

    // Shovel
    Shovel_bar.display(SHOVEL_BAR_DIRECTORY);
    Shovel_bar.blink();
    win.show_text_outlined("S", Shovel_bar.x2 - 13, Shovel_bar.y1, WHITE, BRIANNE_TTF);

    // Plant seed
    display_seed_packets_bar();
    if (player.is_choosing_a_plant() || player.is_shoveling)
    {
        blink_row_and_col();
    }
    display_game_elements();

    // Menu icon (to show pause menu)
    PAUSE_ICON.display(PAUSE_ICON_DIRECTORY);
    TURBO_ICON.display(TURBO_ICON_DIRECTORY);

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
void display_seed_packets_bar()
{
    // Count number of unlocked plant.
    int num_plants = player.seed_packets.size();

    // Plant seed packets
    for (int i = 0; i < num_plants; i++)
    {
        player.seed_packets[i].display(plant_seed[i].x1, plant_seed[i].y1, player.sun_count);
        win.show_text_outlined(to_string(i + 1), plant_seed[i].x1 + ICON_WIDTH - 13, plant_seed[i].y1, WHITE, BRIANNE_TTF);
        plant_seed[i].blink();
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
    PAUSE_ICON.display(PAUSE_ICON_PRESS_DIRECTORY);
    win.draw_bg(BLACK_SCREEN_DIRECTORY);
}

/*If any plant seed is chosen: render it (transparent) at the mouse position.*/
void display_chosen_plant()
{
    int _x = 0, _y = 0;
    SDL_GetMouseState(&_x, &_y);
    if (player.is_shoveling)
    {
        win.draw_png_width_scaled(SHOVEL_DIRECTORY, _x, _y - ICON_HEIGHT, ICON_HEIGHT);
        return;
    }
    if (PEASHOOTER_TYPE <= SeedPacket::chosen_plant && SeedPacket::chosen_plant < PLANT_COUNT)
    {
        _x -= ICON_WIDTH >> 2;
        _y -= ICON_HEIGHT >> 1;
        win.draw_png_width_scaled(PEASHOOTER_DIRECTORY + SeedPacket::chosen_plant, _x, _y, ICON_HEIGHT);
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