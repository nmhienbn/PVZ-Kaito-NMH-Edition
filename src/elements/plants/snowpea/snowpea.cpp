#include "snowpea.hpp"
#define SNOWPEA_FRAME 4
#define SNOWPEA_WIDTH 115
#define SNOWPEA_HEIGHT 98
#define SNOWPEA_G_WIDTH 115
#define SNOWPEA_G_HEIGHT 98

extern int game_state;
extern Map cells;
extern window win;

/*
Snow Pea constructor
*/
Snowpea::Snowpea(const int &_row, const int &_col)
{
    type = SNOWPEA_TYPE;
    row = _row;
    col = _col;
    health = PLANT_HEALTH_LIMIT[SNOWPEA_TYPE];
    sec_for_prepare = 1;
    directory_num = SNOWPEA_SHEET_DIRECTORY;
    frame = 0;
    attacked_time = 0;
}

/*
Snow Pea destructor
*/
Snowpea::~Snowpea()
{
}

/*
Create new pea from a snow pea.
*/
void Snowpea::fire_pea(vector<Zombie *> &zombies, vector<Pea> &peas)
{
    if (directory_num == SNOWPEA_ATTACK_DIRECTORY && frame == 17 * SNOWPEA_FRAME)
    {
        play_sound_effect(FIRE_PEA_MUSIC_DIRECTORY);
        Pea temp(2, row, cells[row][col].x2 - 10);
        peas.push_back(temp);
    }
    determine_appearance(zombies);
}

/*
Change snow pea's appearance if a snow pea need to attack or not.
(Snow Pea is attack only if there are some zombies in the row.)
Updated: Zombie position > snow pea position
*/
void Snowpea::determine_appearance(vector<Zombie *> &zombies)
{
    // Attack
    for (const auto &zombie : zombies)
        if (row == zombie->row &&
            is_in(cells[0][col].x2 - 140, zombie->x_location, ZOMBIE_INIT_X - ZOMBIE_EXACT_LOCATION - 20))
        {
            if (directory_num == SNOWPEA_SHEET_DIRECTORY)
            {
                if (frame == 0)
                {
                    directory_num = SNOWPEA_ATTACK_DIRECTORY;
                }
            }
            return;
        }
    // No attack
    if (directory_num == SNOWPEA_ATTACK_DIRECTORY)
    {
        if (frame == 0)
        {
            directory_num = SNOWPEA_SHEET_DIRECTORY;
        }
    }
}

/*
Display snow pea in row
*/
void Snowpea::display(const int &_row)
{
    if (row == _row)
    {
        // current frame
        int sframe = frame / SNOWPEA_FRAME;
        // current column in source image
        int scol = sframe % all_img[directory_num].c_sheet;
        // current row in source image
        int srow = sframe / all_img[directory_num].c_sheet;
        // Snow Pea
        win.draw_png(directory_num, SNOWPEA_WIDTH * scol, SNOWPEA_HEIGHT * srow,
                     SNOWPEA_WIDTH, SNOWPEA_HEIGHT,
                     cells[row][col].x1 - 10, cells[row][col].y1 - 5,
                     SNOWPEA_G_WIDTH, SNOWPEA_G_HEIGHT);
        // Blink
        if (attacked_time)
        {
            win.draw_png(blink_of[directory_num], SNOWPEA_WIDTH * scol, SNOWPEA_HEIGHT * srow,
                         SNOWPEA_WIDTH, SNOWPEA_HEIGHT,
                         cells[row][col].x1 - 10, cells[row][col].y1 - 5,
                         SNOWPEA_G_WIDTH, SNOWPEA_G_HEIGHT);
            if (check_status(game_state, IS_PAUSED) == false)
                attacked_time--;
        }
        // Next frame
        if (check_status(game_state, IS_PAUSED) == false)
            if (++frame >= SNOWPEA_FRAME * all_img[directory_num].n_sheet)
            {
                frame = 0;
            }
    }
}
