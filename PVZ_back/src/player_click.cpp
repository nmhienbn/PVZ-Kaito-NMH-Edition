#include "player_click.h"
/* Need update: remove a plant
Handle all user click
If player click on sun: handle sun click, then return;
If player click on plant seed: find type of that plant, then return;
If player is choosing a plant and click on a tile: create a new plant there.
*/
void handle_user_click(Player &player, Level &level, Icons &icons, Elements &elements, Map &cells, const int &mouse_x, const int &mouse_y)
{
    if (pick_sun_if_clicked_on(elements, cells, mouse_x, mouse_y, player))
    {
        play_sound_effect(SUN_CLICK_MUSIC_DIRECTORY);
        return;
    }
    if (player.is_shoveling)
    {
        if (click_is_in_frontyard(cells, level, mouse_x, mouse_y))
        {
            play_sound_effect(SHOVEL_MUSIC_DIRECTORY);
            remove_element_if_clicked_on(cells, elements, mouse_x, mouse_y);
        }
        player.is_shoveling = false;
        return;
    }
    if (is_shovel_clicked(mouse_x, mouse_y))
    {
        play_sound_effect(SHOVEL_MUSIC_DIRECTORY);
        player.is_shoveling = true;
        player.is_choosing_a_plant = false;
        return;
    }
    if (is_a_plant_seed_clicked_on(level, mouse_x, mouse_y))
    {
        play_sound_effect(SEED_LIFT_MUSIC_DIRECTORY);
        which_plant_is_chosen(player, icons, mouse_y, player.is_choosing_a_plant);
        return;
    }
    if (player.is_choosing_a_plant == true)
    {
        if (click_is_in_frontyard(cells, level, mouse_x, mouse_y))
        {
            play_sound_effect(PLANT_PLANT_MUSIC_DIRECTORY);
            create_new_plant(player, cells, elements, icons, mouse_x, mouse_y);
            player.is_choosing_a_plant = false;
            return;
        }
        else
        {
            remove_chosen_plant(player, icons);
            return;
        }
    }
    // remove a plant
}

/*Need update:
Check if the mouse is in ICON BAR (the bar that contains plant seed)
*/
bool is_a_plant_seed_clicked_on(Level &level, const int &mouse_x, const int &mouse_y)
{
    for (int i = PLANT_COUNT - 1; i >= 0; i--)
    {
        if (level.level_num >= level_unlock_new_plant[i])
        {
            return ICON_BAR_LV[i].is_mouse_in(mouse_x, mouse_y);
        }
    }
    return false;
}

/*
Find which plant is chosen.
Updated: Double click on a plant seed will cancel the selection.
*/
void which_plant_is_chosen(Player &player, Icons &icons, int mouse_y, bool &is_a_plant_chosen)
{
    is_a_plant_chosen = false;
    for (int i = 0; i < PLANT_COUNT; i++)
    {
        if (plant_seed[i].y1 < mouse_y && mouse_y < plant_seed[i].y2 &&
            player.sun_count >= plant_sun_cost[i] && !icons.plant_remaining_time[i])
        {
            icons.is_plant_chosen[i] ^= 1;
            if (icons.is_plant_chosen[i])
                is_a_plant_chosen = true;
            for (int j = 0; j < PLANT_COUNT; j++)
                if (j != i)
                {
                    icons.is_plant_chosen[j] = false;
                }
            break;
        }
    }
}

/*
Check if mouse is in frontyard or not
*/
bool click_is_in_frontyard(Map &cells, Level &level, const int &mouse_x, const int &mouse_y)
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
Optimize to O(m + n) =))
*/
void determine_row_and_col_chosen_by_second_click(Map &cells, const int &mouse_x, const int &mouse_y, int &row, int &col)
{
    for (int y = 0; y < VERT_BLOCK_COUNT; y++)
    {
        if (mouse_y > cells[y][0].y1 && mouse_y < cells[y][0].y2)
        {
            row = y;
            break;
        }
    }
    for (int x = 0; x < HORIZ_BLOCK_COUNT; x++)
        if (mouse_x > cells[0][x].x1 && mouse_x < cells[0][x].x2)
        {
            col = x;
            break;
        }
}

/*New function:
Remove chosen plant.
*/
void remove_chosen_plant(Player &player, Icons &icons)
{
    player.is_choosing_a_plant = false;
    icons.reset_is_chosen();
}

template <class Plant_type>
void plant_new_plant(Player &player, Map &cells, Icons &icons, vector<Plant_type> &vec_plant, const int &type, const int &row, const int &col)
{
    icons.is_plant_chosen[type] = false;
    icons.plant_remaining_time[type] = plant_loading_time[type];
    Plant_type temp;
    temp.row = row;
    temp.col = col;
    temp.bite = 0;
    vec_plant.push_back(temp);
    player.sun_count -= plant_sun_cost[type];
    cells[row][col].is_planted = 1;
}

/*Need update: Show notification if the tile has planted
Updated:
    Cannot plant if the tile has planted.
    Second click to cancel choosing plant.
*/
void create_new_plant(Player &player, Map &cells, Elements &elements, Icons &icons, const int &mouse_x, const int &mouse_y)
{
    int row, col;
    determine_row_and_col_chosen_by_second_click(cells, mouse_x, mouse_y, row, col);
    if (cells[row][col].is_planted)
    {
        remove_chosen_plant(player, icons);
        return;
    }
    if (icons.is_plant_chosen[PEASHOOTER])
    {
        plant_new_plant<Peashooter>(player, cells, icons, elements.peashooters, PEASHOOTER, row, col);
    }
    else if (icons.is_plant_chosen[SUNFLOWER])
    {
        plant_new_plant<Sunflower>(player, cells, icons, elements.sunflowers, SUNFLOWER, row, col);
    }
    else if (icons.is_plant_chosen[WALNUT])
    {
        plant_new_plant<Walnut>(player, cells, icons, elements.walnuts, WALNUT, row, col);
    }
    else if (icons.is_plant_chosen[SNOWPEA])
    {
        plant_new_plant<Snowpea>(player, cells, icons, elements.snowpeas, SNOWPEA, row, col);
    }
    else if (icons.is_plant_chosen[CHERRYBOMB])
    {
        plant_new_plant<CherryBomb>(player, cells, icons, elements.cherrybombs, CHERRYBOMB, row, col);
    }
}
/*
Remove plant if click on its tile
*/

template <class Plant_type>
void remove_plant_if_clicked_on(Map &cells, Elements &elements, const int &mouse_x, const int &mouse_y)
{
    for (int i = 0, _ = elements.sunflowers.size(); i < _; i++)
    {
        if (is_click_made_in_element_block(elements.sunflowers[i].row, elements.sunflowers[i].col, mouse_x, mouse_y, cells))
        {
            remove_plant(cells, elements.sunflowers, i);
            return;
        }
    }
}
void remove_element_if_clicked_on(Map &cells, Elements &elements, const int &mouse_x, const int &mouse_y)
{
    for (int i = 0; i < (int)elements.sunflowers.size(); i++)
    {
        if (is_click_made_in_element_block(elements.sunflowers[i].row, elements.sunflowers[i].col, mouse_x, mouse_y, cells))
        {
            remove_plant(cells, elements.sunflowers, i);
            return;
        }
    }
    for (int i = 0; i < (int)elements.peashooters.size(); i++)
    {
        if (is_click_made_in_element_block(elements.peashooters[i].row, elements.peashooters[i].col, mouse_x, mouse_y, cells))
        {
            remove_plant(cells, elements.peashooters, i);
            return;
        }
    }
    for (int i = 0; i < (int)elements.walnuts.size(); i++)
    {
        if (is_click_made_in_element_block(elements.walnuts[i].row, elements.walnuts[i].col, mouse_x, mouse_y, cells))
        {
            remove_plant(cells, elements.walnuts, i);
            return;
        }
    }
    for (int i = 0; i < (int)elements.snowpeas.size(); i++)
    {
        if (is_click_made_in_element_block(elements.snowpeas[i].row, elements.snowpeas[i].col, mouse_x, mouse_y, cells))
        {
            remove_plant(cells, elements.snowpeas, i);
            return;
        }
    }
}

/*
Check if player click in the tile in 'row' and 'col'
*/
bool is_click_made_in_element_block(int row, int col, const int &mouse_x, const int &mouse_y, Map &cells)
{
    if (mouse_x > cells[row][col].x1 && mouse_x < cells[row][col].x2 &&
        mouse_y > cells[row][col].y1 && mouse_y < cells[row][col].y2)
        return true;
    return false;
}

/* Update: change void into bool
For all the suns to check if any sun is clicked on.
*/
bool pick_sun_if_clicked_on(Elements &elements, Map &cells, const int &mouse_x, const int &mouse_y, Player &player)
{
    for (auto &sun : elements.suns)
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

bool is_shovel_clicked(const int &mouse_x, const int &mouse_y)
{
    return Shovel_bar.is_mouse_in(mouse_x, mouse_y);
}
