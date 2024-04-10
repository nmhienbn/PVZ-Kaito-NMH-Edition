#include "pea.hpp"
#include <algorithm>

extern Map cells;
extern window win;

/*Construct new pea*/
Pea::Pea(int _row, int _x) : Bullet(PEA, _row, _x)
{
    width = 26;
    height = 25;
    directory_num = PEA_DIRECTORY;
    y_location = cells[row][0].y1 + 20;
}

/*
Return true if pea can move
    + If pea go out the playground: false
    + If pea reach any zombie: false
    + Else: true
*/
void Pea::move()
{
    int right_bound = WINDOW_WIDTH;
    if (x_location <= right_bound)
    {
        if (!has_exploded())
        {
            x_location += PEA_DX;
        }
    }
}

/*
Display the peas
*/
void Pea::display()
{
    int more_px = 0;
    if (directory_num == PEA_EXPLODE_DIRECTORY)
        more_px += 25;
    win.draw_png_scale(directory_num, x_location - more_px / 2, y_location - more_px / 2,
                       width + more_px, height + more_px, angle);
}

void Pea::crash_effect()
{
    play_sound_effect(PEA_CRASH_MUSIC_DIRECTORY);
}

void Pea::make_explode()
{
    if (directory_num == PEA_DIRECTORY)
    {
        directory_num++; // Make the pea explode
        explode = BULLET_EXPLODE_TIME;
        angle = rand(0, 360);
    }
}
