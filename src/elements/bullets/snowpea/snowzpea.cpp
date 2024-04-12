#include "snowzpea.hpp"

extern Map cells;
extern Window win;

SnowzPea::SnowzPea(int _row, int _x) : Bullet(SNOWZPEA, _row, _x)
{
    width = 36;
    height = 25;
    directory_num = SNOWZ_PEA_DIRECTORY;
    slow_down_rate = 2;
    y_location = cells[row][0].y1 + 20;
}

void SnowzPea::move()
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

void SnowzPea::display()
{
    int more_px = 0;
    if (directory_num == SNOWZ_PEA_EXPLODE_DIRECTORY)
        more_px += 25;
    win.draw_png_height_scaled(directory_num, x_location - more_px / 2, y_location - more_px / 2,
                               width + more_px, angle);
}

void SnowzPea::crash_effect()
{
    play_sound_effect(SNOW_PEA_SPARKLES_DIRECTORY);
}

void SnowzPea::make_explode()
{
    if (directory_num == SNOWZ_PEA_DIRECTORY)
    {
        directory_num++; // Make the pea explode
        explode = BULLET_EXPLODE_TIME;
        angle = rand(0, 360);
    }
}