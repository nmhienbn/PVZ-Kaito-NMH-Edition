#include "zombie.h"
#include <algorithm>

/*
Check if Zombie is in query tile or not.
@param zombie: query zombie
@param row: query row
@param col: query column
@return true if zombie is not in query tile.
*/
bool has_zombie_reached_element(const Zombie &zombie, const int &row, const int &col, Map &cells)
{
    int right_limit = cells[row][col].x2 - TILE_WIDTH;
    int left_limit = cells[row][col].x1 - TILE_WIDTH;
    int zombie_new_location = zombie.x_location - ZOMBIE_DX * (zombie.cold_time ? FREEZE_ZOMBIE_SLOW_TIMES : 1);
    if (zombie.row == row &&
        left_limit < zombie_new_location && zombie_new_location < right_limit)
        return true;
    return false;
}

/*
Check if zombie reach any plant or not.
Update zombies' moving status.
@return true if zombie reach any plant
*/
bool has_zombie_reached_any_plant(Elements &elements, Zombie &zombie, Map &cells)
{
    if (has_zombie_reached_plant(zombie, elements.peashooters, cells))
        return true;
    if (has_zombie_reached_plant(zombie, elements.sunflowers, cells))
        return true;
    if (has_zombie_reached_plant(zombie, elements.walnuts, cells))
        return true;
    if (has_zombie_reached_plant(zombie, elements.snowpeas, cells))
        return true;
    if (has_zombie_reached_plant(zombie, elements.cherrybombs, cells))
        return true;
    zombie.is_moving = true;
    zombie.change_zombie_eating_status();
    return false;
}

/*
Check if zombie can move or not:
    + If zombie go out background: false (because you have lose)
    + If zombie.is_moving is false: false (of course)
    + If zombie reached any element: has_zombie_reached_any_plant will decide it
*/
bool can_zombie_move(Zombie &zombie, Elements &elements, Map &cells)
{
    int left_bound = cells[0][0].x1 - TILE_WIDTH;
    int zombie_new_location = zombie.x_location - ZOMBIE_DX * (zombie.cold_time ? FREEZE_ZOMBIE_SLOW_TIMES : 1);
    if (zombie_new_location < left_bound)
        return false;
    if (!zombie.is_moving)
        return false;
    if (has_zombie_reached_any_plant(elements, zombie, cells))
        return false;
    return true;
}

/* I have changed this for better performance
Update all zombies' moving status.
*/
void update_moving_status_for_zombies(Elements &elements, Map &cells)
{
    for (auto &zombie : elements.zombies)
    {
        has_zombie_reached_any_plant(elements, zombie, cells);
    }
}

/*
Move the zombie: For all zombie:
    + If zombie can move, its location -= their speed.
*/
void move_zombies(vector<Zombie> &zombies, Elements &elements, Map &cells)
{
    for (int i = 0; i < (int)zombies.size(); i++)
    {
        if (can_zombie_move(zombies[i], elements, cells))
        {
            if (zombies[i].cold_time)
                zombies[i].x_location -= ZOMBIE_DX * FREEZE_ZOMBIE_SLOW_TIMES;
            else
                zombies[i].x_location -= ZOMBIE_DX;
        }
    }
}

/*Updated
Change to sprite sheet.
Render sprite sheet of exactly status.
*/
void display_zombies(window &win, vector<Zombie> &zombies, Map &cells, bool is_pause)
{
    std::sort(zombies.begin(), zombies.end());
    for (int i = (int)zombies.size() - 1; i >= 0; i--)
    {
        int row = zombies[i].row;
        int y_location = cells[row][0].y1 - 45;
        // win.draw_png_scale(zombies[i].directory_num, zombies[i].x_location, y_location, ELEMENT_WIDTH, ELEMENT_HEIGHT);
        int z_frame = 120 / N_SHEET[zombies[i].directory_num];
        int frame = zombies[i].frame / z_frame;
        int scol = frame % C_SHEET[zombies[i].directory_num];
        int srow = frame / C_SHEET[zombies[i].directory_num];
        win.draw_png(zombies[i].directory_num, ZOMBIE_WIDTH * scol, ZOMBIE_HEIGHT * srow, ZOMBIE_WIDTH, ZOMBIE_HEIGHT, zombies[i].x_location, y_location, ZOMBIE_G_WIDTH, ZOMBIE_G_HEIGHT);
        if (zombies[i].cold_time)
        {
            win.draw_png(cold_of[zombies[i].directory_num], ZOMBIE_WIDTH * scol, ZOMBIE_HEIGHT * srow, ZOMBIE_WIDTH, ZOMBIE_HEIGHT, zombies[i].x_location, y_location, ZOMBIE_G_WIDTH, ZOMBIE_G_HEIGHT);
            zombies[i].cold_time--;
        }
        if (zombies[i].is_attacked)
        {
            win.draw_png(blink_of[zombies[i].directory_num], ZOMBIE_WIDTH * scol, ZOMBIE_HEIGHT * srow, ZOMBIE_WIDTH, ZOMBIE_HEIGHT, zombies[i].x_location, y_location, ZOMBIE_G_WIDTH, ZOMBIE_G_HEIGHT);
            zombies[i].is_attacked--;
        }
        if (is_pause == false)
        {
            if (zombies[i].cold_time % 3 == 0)
                zombies[i].frame++;
            if (zombies[i].frame >= z_frame * N_SHEET[zombies[i].directory_num])
            {
                zombies[i].frame = 0;
            }
        }
    }
}

/*New function:
Display armor drop.
*/
void display_armor_drop(window &win, DeadZombie &dead_zombie, Map &cells)
{
    int row = dead_zombie.row;
    int y_location = cells[row][0].y1 - 45;
    int frame = dead_zombie.frame / ZOMBIE_DIE_FRAME;

    int scol = frame % C_SHEET[dead_zombie.head];
    int srow = frame / C_SHEET[dead_zombie.head];
    win.draw_png(dead_zombie.head, HEAD_ZOMBIE_WIDTH * scol, HEAD_ZOMBIE_HEIGHT * srow, HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT, dead_zombie.x_location + 80, y_location - 40, HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT);
}

void display_burnt_zombie(window &win, DeadZombie &dead_zombie, Map &cells)
{
    int row = dead_zombie.row;
    int y_location = cells[row][0].y1 - 45;
    int frame = dead_zombie.frame / ZOMBIE_BURNT_FRAME;
    if (frame <= N_SHEET[dead_zombie.body])
    {
        int scol = frame % C_SHEET[dead_zombie.body];
        int srow = frame / C_SHEET[dead_zombie.body];
        win.draw_png(dead_zombie.body, ZOMBIE_WIDTH * scol, ZOMBIE_HEIGHT * srow, ZOMBIE_WIDTH, ZOMBIE_HEIGHT, dead_zombie.x_location, y_location, ZOMBIE_WIDTH, ZOMBIE_HEIGHT);
    }
}

/*New function:
Display zombie dead independently.
*/
void display_dead_zombies(window &win, vector<DeadZombie> &dead_zombies, Map &cells, bool is_pause)
{
    for (int i = 0; i < (int)dead_zombies.size(); i++)
    {
        if (dead_zombies[i].body < 0)
        {
            display_armor_drop(win, dead_zombies[i], cells);
            if (is_pause == false)
                if (++dead_zombies[i].frame >= ZOMBIE_DIE_FRAME * N_SHEET[dead_zombies[i].head])
                {

                    dead_zombies.erase(dead_zombies.begin() + i);
                }
        }
        else if (dead_zombies[i].head < 0)
        {
            display_burnt_zombie(win, dead_zombies[i], cells);
            if (is_pause == false)
                if (++dead_zombies[i].frame >= ZOMBIE_BURNT_FRAME * N_SHEET[dead_zombies[i].body])
                {
                    dead_zombies.erase(dead_zombies.begin() + i);
                }
        }
        else
        {
            int row = dead_zombies[i].row;
            int y_location = cells[row][0].y1 - 45;
            // win.draw_png_scale(zombies[i].directory_num, zombies[i].x_location, y_location, ELEMENT_WIDTH, ELEMENT_HEIGHT);
            int frame = dead_zombies[i].frame / ZOMBIE_DIE_FRAME;
            if (frame <= N_SHEET[dead_zombies[i].body])
            {
                int scol = frame % C_SHEET[dead_zombies[i].body];
                int srow = frame / C_SHEET[dead_zombies[i].body];
                win.draw_png(dead_zombies[i].body, ZOMBIE_WIDTH * scol, ZOMBIE_HEIGHT * srow, ZOMBIE_WIDTH, ZOMBIE_HEIGHT, dead_zombies[i].x_location, y_location, ZOMBIE_WIDTH, ZOMBIE_HEIGHT);
                if (dead_zombies[i].is_cold)
                    win.draw_png(cold_of[dead_zombies[i].body], ZOMBIE_WIDTH * scol, ZOMBIE_HEIGHT * srow, ZOMBIE_WIDTH, ZOMBIE_HEIGHT, dead_zombies[i].x_location, y_location, ZOMBIE_WIDTH, ZOMBIE_HEIGHT);
            }
            int scol = frame % C_SHEET[dead_zombies[i].head];
            int srow = frame / C_SHEET[dead_zombies[i].head];
            win.draw_png(dead_zombies[i].head, HEAD_ZOMBIE_WIDTH * scol, HEAD_ZOMBIE_HEIGHT * srow, HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT, dead_zombies[i].x_location + 80, y_location - 40, HEAD_ZOMBIE_G_WIDTH, HEAD_ZOMBIE_G_HEIGHT);
            if (dead_zombies[i].is_cold)
                win.draw_png(cold_of[dead_zombies[i].head], HEAD_ZOMBIE_WIDTH * scol, HEAD_ZOMBIE_HEIGHT * srow, HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT, dead_zombies[i].x_location + 80, y_location - 40, HEAD_ZOMBIE_G_WIDTH, HEAD_ZOMBIE_G_HEIGHT);

            if (is_pause == false)
                if (++dead_zombies[i].frame >= ZOMBIE_DIE_FRAME * N_SHEET[dead_zombies[i].head])
                {

                    dead_zombies.erase(dead_zombies.begin() + i);
                }
        }
    }
}

void update_zombie_next_bite(vector<Zombie> &zombies)
{
    for (auto &zombie : zombies)
        if (zombie.bite_time)
            zombie.bite_time--;
}
