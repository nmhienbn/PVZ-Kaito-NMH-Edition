#include "player_click.hpp"
#include "elements/plants/plant_type.hpp"

extern Level level;
extern Elements game_characters;

extern Player player;
extern Map cells;
extern Window win;

/*Handle all user click:
    If player click on sun: handle sun click, then return;
    Else if player click on shovel: player is shoveling.
    Else if player is choosing a plant and click on a tile:
        specify plant type and create a new plant there.
*/
void handle_user_click(const int &mouse_x, const int &mouse_y)
{
    // If player click on sun: handle sun click, then return;
    if (pick_sun_if_clicked_on(mouse_x, mouse_y))
    {
        play_sound_effect(SUN_CLICK_MUSIC_DIRECTORY);
        return;
    }

    // Else if player is shoveling: handle remove plant.
    if (player.is_shoveling)
    {
        if (click_is_in_frontyard(mouse_x, mouse_y))
        {
            remove_plant_if_clicked_on(game_characters.plants, mouse_x, mouse_y);
        }
        player.is_shoveling = false;
        return;
    }

    // Else if player click on plant seed: find type of that plant, then return;
    if (Shovel_bar.is_mouse_in(mouse_x, mouse_y))
    {
        play_sound_effect(SHOVEL_MUSIC_DIRECTORY);
        player.is_shoveling = true;
        remove_chosen_plant();
        return;
    }

    // Else if player is choosing a plant and click on a tile:
    //     specify plant type and create a new plant there.
    if (is_a_plant_seed_clicked_on(mouse_x, mouse_y))
    {
        which_plant_is_chosen(mouse_y);
        return;
    }
    if (player.is_choosing_a_plant())
    {
        if (click_is_in_frontyard(mouse_x, mouse_y))
        {
            create_new_plant(mouse_x, mouse_y);
            return;
        }
        else
        {
            remove_chosen_plant();
            return;
        }
    }
}

/*
Check if the mouse is in ICON BAR (the bar that contains plant seed)
Depend on level.
*/
bool is_a_plant_seed_clicked_on(const int &mouse_x, const int &mouse_y)
{
    // Now, just y2 of the last plant seed is not known.
    // x1, x2, y1 are known.
    if (!is_in(plant_seed[0].x1, mouse_x, plant_seed[0].x2) ||
        mouse_y < plant_seed[0].y1)
        return false;
    for (int i = PLANT_COUNT - 1; i >= 0; i--)
    {
        if (level.level_num >= level_unlock_new_plant[i])
        {
            return (mouse_y <= plant_seed[i].y2);
        }
    }
    return false;
}

/*
Change is_a_plant_chosen status and chosen plant
    Show announcement when player has not enough sun or plant has not refresh.
    Otherwise, determine which plant is chosen, play sound.
        Note that double choose on a plant seed will cancel the selection.
*/
void change_chosen_status(const int &i)
{
    if (level.level_num < level_unlock_new_plant[i])
        return;
    if (player.sun_count < PLANT_SUN_COST[i])
    {
        // not enough sun
        remove_chosen_plant();
        player.sun_count_change_color_times = 4;
        play_sound_effect(BUZZER_MUSIC_DIRECTORY);
        win.show_announcer_text("YOU DO NOT HAVE ENOUGH SUNS!");
    }
    else if (player.seed_packets[i].remaining_time)
    {
        // plant has not refresh
        remove_chosen_plant();
        win.show_announcer_text("YOU MUST WAIT FOR PLANT SEED TO REFRESH!");
    }
    else
    {
        if (SeedPacket::chosen_plant == i)
        {
            // double choose: remove chosen plant
            SeedPacket::chosen_plant = PLANT_COUNT;
        }
        else
        {
            // specify chosen plant
            SeedPacket::chosen_plant = (PlantType)i;
            play_sound_effect(SEED_LIFT_MUSIC_DIRECTORY);
            player.is_shoveling = false;
        }
    }
}

/*Find which plant is chosen.
For all plant seeds' coordinate.
 */
void which_plant_is_chosen(int mouse_y)
{
    for (int i = 0; i < PLANT_COUNT; i++)
        if (level.level_num >= level_unlock_new_plant[i])
        {
            if (plant_seed[i].y1 < mouse_y && mouse_y < plant_seed[i].y2)
            {
                change_chosen_status(i);
                break;
            }
        }
}

/*
Check if mouse is in frontyard or not.
Depend on level:
    lv1: 1 row
    lv2: 3 rows
    lv > 3: 5 rows
*/
bool click_is_in_frontyard(const int &mouse_x, const int &mouse_y)
{
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
    if (mouse_x > left_bound && mouse_x < right_bound &&
        mouse_y > upper_bound && mouse_y < lower_bound)
        return true;
    return false;
}

/*
Find which row and column is chosen by click
Optimize to O(1) =))
*/
void determine_row_and_col_chosen_by_second_click(const int &mouse_x, const int &mouse_y, int &row, int &col)
{
    row = get_block_row(mouse_y);
    col = get_block_col(mouse_x);
}

/*
Restore the status that player is not choosing a plant
*/
void remove_chosen_plant()
{
    SeedPacket::chosen_plant = PLANT_COUNT;
}

/*Plant the plant in the given row and col.
Reset player_choosing_plant status.
Increase plant's remaining time.
Decrease player's sun count.
Tick that the tile is planted.
Play sound effect.
*/
void plant_new_plant(int type, const int &row, const int &col)
{
    remove_chosen_plant();
    if (type < 0 || type >= PLANT_COUNT)
        return;
    player.seed_packets[type].reset_remaining_time();
    game_characters.plants.push_back(init_plant(type, row, col));
    player.sun_count -= PLANT_SUN_COST[type];
    cells[row][col].is_planted = true;
    cells[row][col].is_block_zombie = true;
    play_sound_effect(PLANT_PLANT_MUSIC_DIRECTORY);
}

/*Find row and column then plant the plant.
    Cannot plant if the tile has planted.
    Second click to cancel choosing plant.
    Show notification if the tile has planted.
*/
void create_new_plant(const int &mouse_x, const int &mouse_y)
{
    int row, col;
    determine_row_and_col_chosen_by_second_click(mouse_x, mouse_y, row, col);
    if (row >= 0 && col >= 0 && cells[row][col].is_planted)
    {
        win.show_announcer_text("YOU CANNOT PLANT ON A PLANTED TILE!");
        remove_chosen_plant();
        return;
    }
    plant_new_plant(SeedPacket::chosen_plant, row, col);
}

/*
Remove plant if player is shoveling and has clicked on its tile
*/
void remove_plant_if_clicked_on(vector<Plants *> &plants, const int &mouse_x, const int &mouse_y)
{
    for (int i = 0, _ = plants.size(); i < _; i++)
    {
        if (is_click_made_in_element_block(plants[i]->get_row(), plants[i]->get_col(), mouse_x, mouse_y))
        {
            play_sound_effect(PLANT_PLANT_MUSIC_DIRECTORY);
            delete_plant(plants, i);
            return;
        }
    }
}

/*
Check if player click in the tile in 'row' and 'col'
*/
bool is_click_made_in_element_block(const int &row, const int &col, const int &mouse_x, const int &mouse_y)
{
    if (mouse_x > cells[row][col].x1 && mouse_x < cells[row][col].x2 &&
        mouse_y > cells[row][col].y1 && mouse_y < cells[row][col].y2)
        return true;
    return false;
}

/*
For all the suns to check if any sun is clicked on.
@return 'true' if sun is clicked on
*/
bool pick_sun_if_clicked_on(const int &mouse_x, const int &mouse_y)
{
    for (auto &sun : game_characters.suns)
    {
        int row = sun.final_row;
        int col = sun.final_col;
        int right_bound = cells[row][col].x2;
        int left_bound = cells[row][col].x1;
        int upper_bound = sun.y_location;
        int lower_bound = sun.y_location + ELEMENT_HEIGHT;
        if (mouse_x > left_bound && mouse_x < right_bound &&
            mouse_y > upper_bound && mouse_y < lower_bound)
        {
            sun.is_clicked = true;
            sun.x_location = left_bound;
            player.sun_count += NORMAL_SUN_VALUE;
            return true;
        }
    }
    return false;
}
