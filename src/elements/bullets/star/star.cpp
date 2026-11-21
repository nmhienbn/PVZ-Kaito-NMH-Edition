#include "star.hpp"
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Map/Map.hpp"
#include "music/music.hpp"
#include <cmath>

extern Map cells;
extern Window win;

/*Construct a star with a constant velocity (px/tick; floats, so the diagonal
rays can fly at the original's 1.73/1.0 px/cs-derived speeds).*/
Star::Star(int _row, int _x, int _y, float _vx, float _vy) : Bullet(STAR, _row, _x)
{
    width = 38;  // Projectile_star.png 38x38 (scale_audit §8.3)
    height = 38;
    vx = _vx;
    vy = _vy;
    x_pos = _x;
    y_pos = _y;
    dir = vx < 0 ? -1 : 1;
    directory_num = STAR_BULLET_DIRECTORY;
    y_location = _y;
    bind_roof_slope();
}

void Star::move()
{
    if (has_exploded())
        return;
    x_pos += vx;
    y_pos += vy;
    x_location = (int)lroundf(x_pos);
    y_location = (int)lroundf(y_pos);
    int new_row = get_block_row(x_location + width / 2, y_location + height / 2);
    if (new_row >= 0)
        row = new_row;
    // The star's shadow rises/dives with the shot (repvz UpdateNormalMotion
    // mShadowY += mVelY), so its roof gap thins only by the slope itself.
    if (splats_on_roof(23)) // star threshold (repvz Projectile.cpp:354)
        make_explode();
    // Stars just tumble off the board.
    if (x_location > WINDOW_WIDTH - 20 || x_location < X_UPPER_LEFT - 30 ||
        y_location > WINDOW_HEIGHT || y_location < Y_UPPER_LEFT - 40)
        make_explode();
}

void Star::display()
{
    const int dw = world_px(width), dh = world_px(height);
    win.draw_png(directory_num, x_location + (width - dw) / 2,
                 y_location + (height - dh) / 2, dw, dh);
}

void Star::crash_effect()
{
    play_sound_effect(PEA_CRASH_MUSIC_DIRECTORY);
}

void Star::make_explode()
{
    // No star pop art: hold the last position briefly, then disappear.
    if (!has_exploded())
        explode = BULLET_EXPLODE_TIME;
}
