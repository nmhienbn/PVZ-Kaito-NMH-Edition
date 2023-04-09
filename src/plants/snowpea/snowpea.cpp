#include "snowpea.hpp"
#define SNOWPEA_FRAME 2
#define SNOWPEA_WIDTH 185
#define SNOWPEA_HEIGHT 177
#define SNOWPEA_G_WIDTH 85
#define SNOWPEA_G_HEIGHT 85

extern bool is_paused;
extern Map cells;
extern window win;

/*
For all snowpea: If there is a zombie in that snowpea's row:
    Generate a new snowz pea at that snowpea's position (if it's sprite to fire).
    All snowz peas are generated not at a time.
*/
void fire_snowz_peas(vector<Snowpea> &snowpeas, vector<Zombie> &zombies, vector<Pea> &peas)
{
    for (Snowpea &snowpea : snowpeas)
    {
        int row = snowpea.row;
        int col = snowpea.col;
        if (snowpea.directory_num == SNOWPEA_ATTACK_DIRECTORY && snowpea.frame == 32 * SNOWPEA_FRAME)
        {
            play_sound_effect(FIRE_PEA_MUSIC_DIRECTORY);
            Pea temp(2, row, cells[row][col].x2 - 10);
            peas.push_back(temp);
        }
        if (are_there_zombies_in_snowpea_row(snowpea, zombies))
        {
            if (snowpea.directory_num == SNOWPEA_SHEET_DIRECTORY)
            {
                if (snowpea.frame == 0)
                {
                    snowpea.directory_num = SNOWPEA_ATTACK_DIRECTORY;
                }
            }
        }
        else
        {
            if (snowpea.directory_num == SNOWPEA_ATTACK_DIRECTORY)
            {
                if (snowpea.frame == 0)
                {
                    snowpea.directory_num = SNOWPEA_SHEET_DIRECTORY;
                }
            }
        }
    }
}

/*
Check if a snowpea need to attack or not.
Snowpea is attack only if there are some zombies in the row.
Updated: Zombie position > snowpea position
*/
bool are_there_zombies_in_snowpea_row(Snowpea &snowpea, vector<Zombie> &zombies)
{
    for (Zombie zombie : zombies)
        if (snowpea.row == zombie.row &&
            is_in(cells[0][snowpea.col].x2 - 140, zombie.x_location, ZOMBIE_INIT_X - 70))
            return true;
    return false;
}

/*
Display snow peas
*/
void display_snowpeas(vector<Snowpea> &snowpeas)
{
    for (auto &snowpea : snowpeas)
    {
        int col = snowpea.col;
        int row = snowpea.row;
        int frame = snowpea.frame / SNOWPEA_FRAME;
        int scol = frame % C_SHEET[snowpea.directory_num];
        int srow = frame / C_SHEET[snowpea.directory_num];
        win.draw_png(snowpea.directory_num, SNOWPEA_WIDTH * scol, SNOWPEA_HEIGHT * srow,
                     SNOWPEA_WIDTH, SNOWPEA_HEIGHT,
                     cells[row][col].x1, cells[row][col].y1 + 5,
                     SNOWPEA_G_WIDTH, SNOWPEA_G_HEIGHT);
        if (snowpea.is_attacked)
        {
            win.draw_png(blink_of[snowpea.directory_num], SNOWPEA_WIDTH * scol, SNOWPEA_HEIGHT * srow,
                         SNOWPEA_WIDTH, SNOWPEA_HEIGHT,
                         cells[row][col].x1, cells[row][col].y1 + 5,
                         SNOWPEA_G_WIDTH, SNOWPEA_G_HEIGHT);
            snowpea.is_attacked--;
        }
        if (is_paused == false)
            if (++snowpea.frame >= SNOWPEA_FRAME * N_SHEET[snowpea.directory_num])
            {
                snowpea.frame = 0;
            }
    }
}
