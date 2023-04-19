#include "zombies/zombie.hpp"
#include <algorithm>

#define ZOMBIE_EATING_FRAME 10

#define ZOMBIE_DIE_FRAME 2
#define ZOMBIE_BURNT_FRAME 6
#define DEAD_ZOMBIE_FRAMES 12

#define HEAD_ZOMBIE_FRAME 15
#define HEAD_ZOMBIE_WIDTH 150
#define HEAD_ZOMBIE_HEIGHT 187
#define HEAD_ZOMBIE_G_WIDTH 125
#define HEAD_ZOMBIE_G_HEIGHT 155

extern int game_state;
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
    int right_limit = cells[row][col].x2 - 1;
    int left_limit = cells[row][col].x1;
    int zombie_new_location = zombie.x_location - ZOMBIE_DX;
    if (zombie.row == row &&
        is_in(left_limit, zombie_new_location + 95, right_limit))
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
        if (zombie.next_step_time)
        {
            zombie.next_step_time--;
            continue;
        }
        zombie.next_step_time = ZOMBIE_CLK_COUNT;
        if (zombie.cold_time)
            zombie.next_step_time += ZOMBIE_CLK_COUNT;
        if (can_zombie_move(zombie))
        {
            zombie.x_location -= ZOMBIE_DX;
        }
    }
}

/*
Display zombie of exactly status.
*/
void display_zombies(vector<Zombie> &zombies, const int &_row)
{
    stable_sort(zombies.begin(), zombies.end());
    for (auto &zombie : zombies)
    {
        zombie.display(_row);
    }
}

/*
Display armor drop.
*/
void display_armor_drop(DeadZombie &dead_zombie)
{
    int row = dead_zombie.row;
    int y_location = cells[row][0].y1 - 50;
    int frame = dead_zombie.frame / HEAD_ZOMBIE_FRAME;

    int scol = frame % all_img[dead_zombie.head].c_sheet;
    int srow = frame / all_img[dead_zombie.head].c_sheet;
    win.draw_png(dead_zombie.head, HEAD_ZOMBIE_WIDTH * scol, HEAD_ZOMBIE_HEIGHT * srow, HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT, dead_zombie.x_location + 80, y_location - 40, HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT);
}

/*
Display burnt zombie (after explosion)
*/
void display_burnt_zombie(DeadZombie &dead_zombie)
{
    int row = dead_zombie.row;
    int y_location = cells[row][0].y1 - 50;
    int frame = dead_zombie.frame / ZOMBIE_BURNT_FRAME;
    if (frame <= all_img[dead_zombie.body].n_sheet)
    {
        int scol = frame % all_img[dead_zombie.body].c_sheet;
        int srow = frame / all_img[dead_zombie.body].c_sheet;
        win.draw_png(dead_zombie.body, ZOMBIE_WIDTH * scol, ZOMBIE_HEIGHT * srow, ZOMBIE_WIDTH, ZOMBIE_HEIGHT, dead_zombie.x_location, y_location, ZOMBIE_WIDTH, ZOMBIE_HEIGHT);
    }
}

/*
Display dead zombie independently.
Add armor drop and burnt effect.
*/
void display_dead_zombies(vector<DeadZombie> &dead_zombies, const int &_row)
{
    for (int i = 0; i < (int)dead_zombies.size(); i++)
        if (dead_zombies[i].row == _row)
        {
            if (dead_zombies[i].body < 0)
            {
                display_armor_drop(dead_zombies[i]);
                if (check_status(game_state, IS_PAUSED) == false)
                    if (++dead_zombies[i].frame >= HEAD_ZOMBIE_FRAME * all_img[dead_zombies[i].head].n_sheet)
                    {
                        dead_zombies.erase(dead_zombies.begin() + i);
                        i--;
                    }
            }
            else if (dead_zombies[i].head < 0)
            {
                display_burnt_zombie(dead_zombies[i]);
                if (check_status(game_state, IS_PAUSED) == false)
                    if (++dead_zombies[i].frame >= ZOMBIE_BURNT_FRAME * all_img[dead_zombies[i].body].n_sheet)
                    {
                        dead_zombies.erase(dead_zombies.begin() + i);
                        i--;
                    }
            }
            else
            {
                int row = dead_zombies[i].row;
                int y_location = cells[row][0].y1 - 50;
                // win.draw_png_scale(zombies[i].directory_num, zombies[i].x_location, y_location, ELEMENT_WIDTH, ELEMENT_HEIGHT);
                int frame = dead_zombies[i].frame / ZOMBIE_DIE_FRAME;
                if (frame <= all_img[dead_zombies[i].body].n_sheet)
                {
                    int scol = frame % all_img[dead_zombies[i].body].c_sheet;
                    int srow = frame / all_img[dead_zombies[i].body].c_sheet;
                    win.draw_png(dead_zombies[i].body, ZOMBIE_WIDTH * scol, ZOMBIE_HEIGHT * srow, ZOMBIE_WIDTH, ZOMBIE_HEIGHT, dead_zombies[i].x_location, y_location, ZOMBIE_WIDTH, ZOMBIE_HEIGHT);
                    if (dead_zombies[i].is_cold)
                        win.draw_png(cold_of[dead_zombies[i].body], ZOMBIE_WIDTH * scol, ZOMBIE_HEIGHT * srow, ZOMBIE_WIDTH, ZOMBIE_HEIGHT, dead_zombies[i].x_location, y_location, ZOMBIE_WIDTH, ZOMBIE_HEIGHT);
                }
                int scol = frame % all_img[dead_zombies[i].head].c_sheet;
                int srow = frame / all_img[dead_zombies[i].head].c_sheet;
                win.draw_png(dead_zombies[i].head, HEAD_ZOMBIE_WIDTH * scol, HEAD_ZOMBIE_HEIGHT * srow, HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT, dead_zombies[i].x_location + 80, y_location - 40, HEAD_ZOMBIE_G_WIDTH, HEAD_ZOMBIE_G_HEIGHT);
                if (dead_zombies[i].is_cold)
                    win.draw_png(cold_of[dead_zombies[i].head], HEAD_ZOMBIE_WIDTH * scol, HEAD_ZOMBIE_HEIGHT * srow, HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT, dead_zombies[i].x_location + 80, y_location - 40, HEAD_ZOMBIE_G_WIDTH, HEAD_ZOMBIE_G_HEIGHT);

                if (check_status(game_state, IS_PAUSED) == false)
                    if (++dead_zombies[i].frame >= ZOMBIE_DIE_FRAME * all_img[dead_zombies[i].body].n_sheet)
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
/*
If the zombie reached plant_type, apply its bites on the plant_type:
    + bite++
    + if plant_type's health == 0: reset zombie moving and delete that plant_type
*/
bool apply_zombie_bite_on_plant(Zombie &zombie, vector<Plants *> &plants, int &p_ind)
{
    if (has_zombie_reached_element(zombie, plants[p_ind]->get_row(), plants[p_ind]->get_col()))
    {
        play_sound_effect(ZOMBIE_EATING_MUSIC_DIRECTORY);
        zombie.bite_time = BITE_CLK_COUNT;
        if (zombie.cold_time)
            zombie.bite_time += BITE_CLK_COUNT;
        plants[p_ind]->set_attacked_time(MAX_TIME_BLINK);
        if (plants[p_ind]->decrease_health())
        {
            zombie.is_moving = true;
            zombie.bite_time = BITE_CLK_COUNT;
            cells[plants[p_ind]->get_row()][plants[p_ind]->get_col()].is_planted = false;
            delete plants[p_ind];
            plants.erase(plants.begin() + p_ind);
            p_ind--;
        }
        return true;
    }
    return false;
}

/* Note to optimize after
For all zombie and all plant_type: apply zombie's bite
*/
void handle_zombie_plant_encounter(vector<Zombie> &zombies, vector<Plants *> &plants)
{
    for (auto &zombie : zombies)
        if (zombie.bite_time == 0)
            for (int i = 0; i < (int)plants.size(); i++)
                if (apply_zombie_bite_on_plant(zombie, plants, i))
                {
                    break;
                }
}