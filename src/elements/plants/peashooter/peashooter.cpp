#include "peashooter.hpp"
#define PEASHOOTER_FRAME 2
#define PEASHOOTER_WIDTH 85
#define PEASHOOTER_HEIGHT 81
#define PEASHOOTER_G_WIDTH 85
#define PEASHOOTER_G_HEIGHT 81

extern int game_state;
extern Map cells;
extern window win;

/*
Peashooter constructor
*/
Peashooter::Peashooter(const int &_row, const int &_col)
{
    type = PEASHOOTER_TYPE;
    row = _row;
    col = _col;
    health = PLANT_HEALTH_LIMIT[PEASHOOTER_TYPE];
    sec_for_prepare = 1;
    directory_num = PEASHOOTER_SHEET_DIRECTORY;
    frame = 0;
    attacked_time = 0;
}

/*
Peashooter destructor
*/
Peashooter::~Peashooter()
{
}

/*
Create new pea from a peashooter.
*/
void Peashooter::fire_pea(vector<Zombie *> &zombies, vector<Pea> &peas)
{
    if (directory_num == PEASHOOTER_ATTACK_DIRECTORY && frame == 32 * PEASHOOTER_FRAME)
    {
        play_sound_effect(FIRE_PEA_MUSIC_DIRECTORY);
        Pea temp(1, row, cells[row][col].x2 - 10);
        peas.push_back(temp);
    }
    determine_appearance(zombies);
}

/*
Change peashooter's appearance if a peashooter need to attack or not.
(Peashooter is attack only if there are some zombies in the row.)
Updated: Zombie position > peashooter position
*/
void Peashooter::determine_appearance(vector<Zombie *> &zombies)
{
    // Attack
    for (const auto &zombie : zombies)
        if (row == zombie->row &&
            is_in(cells[0][col].x2 - 140, zombie->x_location, ZOMBIE_INIT_X - ZOMBIE_EXACT_LOCATION - 20))
        {
            if (directory_num == PEASHOOTER_SHEET_DIRECTORY)
            {
                if (frame == 0)
                {
                    directory_num = PEASHOOTER_ATTACK_DIRECTORY;
                }
            }
            return;
        }
    // No attack
    if (directory_num == PEASHOOTER_ATTACK_DIRECTORY)
    {
        if (frame == 0)
        {
            directory_num = PEASHOOTER_SHEET_DIRECTORY;
        }
    }
}

/*
Display peashooters in row
*/
void Peashooter::display(const int &_row)
{
    if (row == _row)
    {
        // current frame
        int sframe = frame / PEASHOOTER_FRAME;
        // current column in source image
        int scol = sframe % all_img[directory_num].c_sheet;
        // current row in source image
        int srow = sframe / all_img[directory_num].c_sheet;
        // Peashooter
        win.draw_png(directory_num, PEASHOOTER_WIDTH * scol, PEASHOOTER_HEIGHT * srow,
                     PEASHOOTER_WIDTH, PEASHOOTER_HEIGHT,
                     cells[row][col].x1 + 5, cells[row][col].y1 + 5,
                     PEASHOOTER_G_WIDTH, PEASHOOTER_G_HEIGHT);
        // Blink
        if (attacked_time)
        {
            win.draw_png(blink_of[directory_num], PEASHOOTER_WIDTH * scol, PEASHOOTER_HEIGHT * srow,
                         PEASHOOTER_WIDTH, PEASHOOTER_HEIGHT,
                         cells[row][col].x1 + 5, cells[row][col].y1 + 5,
                         PEASHOOTER_G_WIDTH, PEASHOOTER_G_HEIGHT);
            if (check_status(game_state, IS_PAUSED) == false)
                attacked_time--;
        }
        // Next frame
        if (check_status(game_state, IS_PAUSED) == false)
            if (++frame >= PEASHOOTER_FRAME * all_img[directory_num].n_sheet)
            {
                frame = 0;
            }
    }
}
