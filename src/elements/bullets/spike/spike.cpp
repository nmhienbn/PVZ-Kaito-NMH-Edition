#include "spike.hpp"
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Map/Map.hpp"
#include "elements/Level/Level.hpp" // roof_tile_dy
#include "music/music.hpp"
#include <cmath>

extern Map cells;
extern Window win;

/*Construct new cactus spike*/
Spike::Spike(int _row, int _x, int _y_bias) : Bullet(SPIKE, _row, _x)
{
    width = 30;  // ProjectileCactus.png 30x16 (scale_audit §8.3)
    x_pos = (float)_x;
    height = 16;
    directory_num = SPIKE_BULLET_DIRECTORY;
    y_location = cells[row][0].y1 + 20 + _y_bias + roof_tile_dy(get_block_col(_x));
    bind_roof_slope();
}

void Spike::move()
{
    if (!has_exploded() && x_location <= WINDOW_WIDTH)
    {
        x_pos += PEA_DX;
        x_location = (int)lroundf(x_pos);
        if (splats_on_roof(28)) // pea/spike threshold (repvz Projectile.cpp:341)
            make_explode();
    }
}

void Spike::display()
{
    // Grow about the logical-rect center; width/height stay collision-only.
    const int dw = world_px(width), dh = world_px(height);
    win.draw_png(directory_num, x_location + (width - dw) / 2,
                 y_location + (height - dh) / 2, dw, dh);
}

void Spike::crash_effect()
{
    play_sound_effect(PEA_CRASH_MUSIC_DIRECTORY);
}

void Spike::make_explode()
{
    // No spike pop art: freeze in place briefly, then is_disappeared reaps it.
    if (!has_exploded())
        explode = BULLET_EXPLODE_TIME;
}
