#include "player_click.hpp"

extern Level level;
extern Elements game_characters;
extern Icons icons;
extern Player player;
extern Map cells;
extern window win;

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
            remove_element_if_clicked_on(mouse_x, mouse_y);
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
        which_plant_is_chosen(mouse_y, player.is_choosing_a_plant);
        return;
    }
    if (player.is_choosing_a_plant == true)
    {
        if (click_is_in_frontyard(mouse_x, mouse_y))
        {
            create_new_plant(mouse_x, mouse_y);
            player.is_choosing_a_plant = false;
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
Change is_a_plant_chosen status and chosen plant
    Show announcement when player has not enough sun or plant has not refresh.
    Otherwise, determine which plant is chosen, play sound.
        Note that double choose on a plant seed will cancel the selection.
*/
void change_chosen_status(const int &i, bool &is_a_plant_chosen)
{
    if (level.level_num < level_unlock_new_plant[i])
        return;
    is_a_plant_chosen = false;
    if (player.sun_count < PLANT_SUN_COST[i])
    {
        // not enough sun
        remove_chosen_plant();
        player.sun_count_change_color_times = 4;
        play_sound_effect(BUZZER_MUSIC_DIRECTORY);
        win.show_announcer_text("YOU DO NOT HAVE ENOUGH SUNS!");
    }
    else if (icons.plant_remaining_time[i])
    {
        // plant has not refresh
        remove_chosen_plant();
        win.show_announcer_text("YOU MUST WAIT FOR PLANT SEED TO REFRESH!");
    }
    else
    {
        if (icons.chosen_plant == i)
        {
            // double choose: remove chosen plant
            icons.chosen_plant = PLANT_COUNT;
        }
        else
        {
            // specify chosen plant
            icons.chosen_plant = i;
            play_sound_effect(SEED_LIFT_MUSIC_DIRECTORY);
            is_a_plant_chosen = true;
            player.is_shoveling = false;
        }
    }
}

/*Find which plant is chosen.
For all plant seeds' coordinate.
 */
void which_plant_is_chosen(int mouse_y, bool &is_a_plant_chosen)
{
    for (int i = 0; i < PLANT_COUNT; i++)
        if (level.level_num >= level_unlock_new_plant[i])
        {
            if (plant_seed[i].y1 < mouse_y && mouse_y < plant_seed[i].y2)
            {
                change_chosen_status(i, is_a_plant_chosen);
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
    player.is_choosing_a_plant = false;
    icons.chosen_plant = PLANT_COUNT;
}

/*Plant the plant in the given row and col.
Reset player_choosing_plant status.
Increase plant's remaining time.
Decrease player's sun count.
Tick that the tile is planted.
Play sound effect.
*/
template <class Plant_type>
void plant_new_plant(const int &type, const int &row, const int &col)
{
    icons.chosen_plant = PLANT_COUNT;
    icons.plant_remaining_time[type] = PLANT_LOADING_TIME[type];
    game_characters.plants.push_back(new Plant_type(row, col));
    player.sun_count -= PLANT_SUN_COST[type];
    cells[row][col].is_planted = 1;
    play_sound_effect(PLANT_PLANT_MUSIC_DIRECTORY);
}

#define PLANT_PLANT_MACRO(type, enum_value)          \
    if (icons.chosen_plant == enum_value)            \
    {                                                \
        plant_new_plant<type>(enum_value, row, col); \
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
    PLANT_PLANT_MACRO(Peashooter, PEASHOOTER_TYPE)
    PLANT_PLANT_MACRO(Sunflower, SUNFLOWER_TYPE)
    PLANT_PLANT_MACRO(Walnut, WALNUT_TYPE)
    PLANT_PLANT_MACRO(Snowpea, SNOWPEA_TYPE)
    PLANT_PLANT_MACRO(CherryBomb, CHERRYBOMB_TYPE)
}

#undef PLANT_PLANT_MACRO

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
Find the type of plant to remove.
*/
void remove_element_if_clicked_on(const int &mouse_x, const int &mouse_y)
{
    remove_plant_if_clicked_on(game_characters.plants, mouse_x, mouse_y);
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
