#include "snowzpea.hpp"
#include "../pea/pea.hpp" // crosses_torchwood
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Map/Map.hpp"
#include "elements/Level/Level.hpp" // roof_tile_dy
#include "music/music.hpp"
#include <algorithm>
#include <cmath>

extern Map cells;
extern Window win;

SnowzPea::SnowzPea(int _row, int _x) : Bullet(SNOWZPEA, _row, _x)
{
    width = 36;
    x_pos = (float)_x;
    height = 25;
    directory_num = SNOWZ_PEA_DIRECTORY;
    slow_down_rate = 2;
    y_location = cells[row][0].y1 + 20 + roof_tile_dy(get_block_col(_x));
    bind_roof_slope();
}

void SnowzPea::move()
{
    int right_bound = WINDOW_WIDTH;
    if (x_location <= right_bound)
    {
        if (!has_exploded())
        {
            int prev_x = x_location;
            x_pos += PEA_DX;
            x_location = (int)lroundf(x_pos);
            // Torchwood melts the chill off: the shot flies on as a plain
            // pea (no snow splash at impact anymore).
            if (type == SNOWZPEA && crosses_torchwood(row, prev_x, x_location))
            {
                type = PEA; // melted: plain pea again
                directory_num = PEA_DIRECTORY;
            }
            if (splats_on_roof(28)) // pea/spike threshold (repvz Projectile.cpp:341)
                make_explode();
        }
    }
}

void SnowzPea::display()
{
    int more_px = 0;
    if (directory_num == SNOWZ_PEA_EXPLODE_DIRECTORY)
        more_px += 25;
    const int wtot = world_px(width + more_px);
    const int grow = wtot - width;
    win.draw_png_height_scaled(directory_num, x_location - grow / 2, y_location - grow / 2,
                               wtot, angle);
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
    else if (directory_num == PEA_DIRECTORY)
    {
        // Melted through a torchwood: a plain pea again, pop with the pea splat.
        directory_num = PEA_EXPLODE_DIRECTORY;
        explode = BULLET_EXPLODE_TIME;
        angle = rand(0, 360);
    }
}