#include "puff.hpp"
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Map/Map.hpp"
#include "elements/Level/Level.hpp" // roof_tile_dy
#include "music/music.hpp"
#include <algorithm>
#include <cmath>

extern Map cells;
extern Window win;

/*Construct new spore puff; range_px < 0 keeps the puff-shroom's ~3-tile cap*/
Puff::Puff(int _row, int _x, int _range_px) : Bullet(PUFF, _row, _x)
{
    width = 32;  // PuffShroom_puff1.png canvas 32x27 (scale_audit §8.3)
    x_pos = (float)_x;
    height = 32;
    directory_num = PUFF_BULLET_DIRECTORY;
    spawn_x = _x;
    range_px = _range_px >= 0 ? _range_px : PUFF_RANGE_PX;
    y_location = cells[row][0].y1 + 38 + roof_tile_dy(get_block_col(_x)); // puff-shroom is short: muzzle low
    bind_roof_slope();
}

/*
Return true if puff can move
    + Same as the pea, but the spore pops at the range cap
      (repvz Projectile.cpp:244 MOTION_PUFF dies at 75 cs = ~250 px of
      travel; the puff-shroom attack rect mX+60..mX+290 ends just past it,
      scaredy-shroom shoots across the whole row).
*/
void Puff::move()
{
    int right_bound = WINDOW_WIDTH;
    if (x_location <= right_bound)
    {
        if (!has_exploded())
        {
            x_pos += PEA_DX;
            x_location = (int)lroundf(x_pos);
            if (x_location - spawn_x > range_px)
                make_explode();
            else if (splats_on_roof(0)) // puff pops at the shingle line
                make_explode();
        }
    }
}

/*
Display the spores
*/
void Puff::display()
{
    int more_px = 0;
    if (directory_num == PUFF_EXPLODE_DIRECTORY)
        more_px += 18;
    const int wtot = world_px(width + more_px);
    const int grow = wtot - width;
    win.draw_png_height_scaled(directory_num, x_location - grow / 2, y_location - grow / 2,
                               wtot, angle);
}

void Puff::crash_effect()
{
    play_sound_effect(PEA_CRASH_MUSIC_DIRECTORY);
}

void Puff::make_explode()
{
    if (directory_num == PUFF_BULLET_DIRECTORY)
    {
        directory_num++; // Make the puff pop
        explode = BULLET_EXPLODE_TIME;
        angle = rand(0, 360);
    }
}
