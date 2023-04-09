#include "zombies/zombie.hpp"
#include <algorithm>

extern bool is_paused;
extern Map cells;
extern window win;
/*
Check if Zombie is in query tile or not to change its status.
@param zombie: query zombie
@param row: query row
@param col: query column
@return true if zombie is not in query tile.
*/
bool has_zombie_reached_element(const Zombie &zombie, const int &row, const int &col)
{
    int right_limit = cells[row][col].x2 - BLOCK_WIDTH;
    int left_limit = cells[row][col].x1 - BLOCK_WIDTH;
    int zombie_new_location = zombie.x_location - (zombie.cold_time ? ZOMBIE_COLD_DX : ZOMBIE_DX);
    if (zombie.row == row &&
        is_in(left_limit, zombie_new_location, right_limit))
        return true;
    return false;
}

/*
Check if zombie reached a planted tile.
If true, Update zombie status.
*/
bool has_zombie_reached_plant(Zombie &zombie)
{
    for (int row = 0; row < VERT_BLOCK_COUNT; row++)
        for (int col = 0; col < HORIZ_BLOCK_COUNT; col++)
        {
            if (cells[row][col].is_planted && has_zombie_reached_element(zombie, row, col))
            {
                zombie.is_moving = false;
                zombie.change_zombie_eating_status();
                return true;
            }
        }
    return false;
}

/*
Check if zombie reach any plant or not.
Update zombies' moving status.
@return true if zombie reach any plant
*/
bool has_zombie_reached_any_plant(Zombie &zombie)
{
    if (has_zombie_reached_plant(zombie))
        return true;
    zombie.is_moving = true;
    zombie.change_zombie_eating_status();
    return false;
}

/*
Check if zombie can move or not:
    + If zombie go out background: false (because you have lose)
    But we don't need to do this because the has_player_lose will check it and end the game.
    + If zombie.is_moving is false: false (of course)
    + If zombie reached any element: has_zombie_reached_any_plant will decide it
*/
bool can_zombie_move(Zombie &zombie)
{
    // int left_bound = cells[0][0].x1 - BLOCK_WIDTH;
    // int zombie_new_location = zombie.x_location - ZOMBIE_DX * (zombie.cold_time ? ZOMBIE_COLD_DX : ZOMBIE_DX);
    // if (zombie_new_location < left_bound)
    //     return false;
    if (!zombie.is_moving)
        return false;
    if (has_zombie_reached_any_plant(zombie))
        return false;
    return true;
}

/*
Update all zombies' moving status.
*/
void update_moving_status_for_zombies(vector<Zombie> &zombies)
{
    for (auto &zombie : zombies)
    {
        has_zombie_reached_any_plant(zombie);
    }
}

/*
Move the zombie: For all zombie:
    If zombie can move, its location -= their speed.
    Note that a zombie can be slow if it's cold.
*/
void move_zombies(vector<Zombie> &zombies)
{
    for (auto &zombie : zombies)
    {
        if (can_zombie_move(zombie))
        {
            zombie.x_location -= (zombie.cold_time ? ZOMBIE_COLD_DX : ZOMBIE_DX);
        }
    }
}

/*
Display zombie of exactly status.
*/
void display_zombies(vector<Zombie> &zombies)
{
    std::sort(zombies.begin(), zombies.end());
    for (int i = (int)zombies.size() - 1; i >= 0; i--)
    {
        int row = zombies[i].row;
        int y_location = cells[row][0].y1 - 45;
        // win.draw_png_scale(zombies[i].directory_num, zombies[i].x_location, y_location, ELEMENT_WIDTH, ELEMENT_HEIGHT);
        int z_frame = 100 / N_SHEET[zombies[i].directory_num];
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
        if (is_paused == false)
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

/*
Display armor drop.
*/
void display_armor_drop(DeadZombie &dead_zombie)
{
    int row = dead_zombie.row;
    int y_location = cells[row][0].y1 - 45;
    int frame = dead_zombie.frame / ZOMBIE_DIE_FRAME;

    int scol = frame % C_SHEET[dead_zombie.head];
    int srow = frame / C_SHEET[dead_zombie.head];
    win.draw_png(dead_zombie.head, HEAD_ZOMBIE_WIDTH * scol, HEAD_ZOMBIE_HEIGHT * srow, HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT, dead_zombie.x_location + 80, y_location - 40, HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT);
}

/*
Display burnt zombie (after explosion)
*/
void display_burnt_zombie(DeadZombie &dead_zombie)
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

/*
Display dead zombie independently.
Add armor drop and burnt effect.
*/
void display_dead_zombies(vector<DeadZombie> &dead_zombies)
{
    for (int i = 0; i < (int)dead_zombies.size(); i++)
    {
        if (dead_zombies[i].body < 0)
        {
            display_armor_drop(dead_zombies[i]);
            if (is_paused == false)
                if (++dead_zombies[i].frame >= ZOMBIE_DIE_FRAME * N_SHEET[dead_zombies[i].head])
                {
                    dead_zombies.erase(dead_zombies.begin() + i);
                    i--;
                }
        }
        else if (dead_zombies[i].head < 0)
        {
            display_burnt_zombie(dead_zombies[i]);
            if (is_paused == false)
                if (++dead_zombies[i].frame >= ZOMBIE_BURNT_FRAME * N_SHEET[dead_zombies[i].body])
                {
                    dead_zombies.erase(dead_zombies.begin() + i);
                    i--;
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

            if (is_paused == false)
                if (++dead_zombies[i].frame >= ZOMBIE_DIE_FRAME * N_SHEET[dead_zombies[i].head])
                {
                    dead_zombies.erase(dead_zombies.begin() + i);
                    i--;
                }
        }
    }
}

/*
Update time for all zombies' next bite.
*/
void update_zombie_next_bite(vector<Zombie> &zombies)
{
    for (auto &zombie : zombies)
        if (zombie.bite_time)
            zombie.bite_time--;
}
