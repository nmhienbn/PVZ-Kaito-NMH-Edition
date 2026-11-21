#include "bullet.hpp"
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "music/music.hpp"
#include <algorithm>

extern Map cells;
extern Window win;
extern Level level;

/*Construct new pea*/
Bullet::Bullet(int _type, int _row, int _x)
{
    if (_type >= BULLET_TYPE_NUM)
        throw "Invalid bullet type";
    type = _type;
    row = _row;
    x_location = _x;
    explode = INF;
    slow_down_rate = 1;
    dmg = 1;
    angle = 0;
}

Bullet::~Bullet()
{
}

/*
x_location > other.x_location
*/
bool Bullet::operator<(const Bullet &other) const
{
    return x_location > other.x_location;
}

/*
check if a pea has exploded or not.
initialized pea will explode after BULLET_EXPLODE_TIME.
*/
bool Bullet::has_exploded()
{
    return explode <= BULLET_EXPLODE_TIME;
}

/*
Check if a pea disappeared
A pea will disappear if it exploded or moved out the screen
*/
bool Bullet::is_disappeared()
{
    int right_bound = WINDOW_WIDTH;
    return --explode == 0 || x_location > right_bound;
}

/*
Display the shadow under the pea.
Original Projectile::DrawShadow (Projectile.cpp:1078) draws the blob at
mShadowY, the ground line the shot is over: on a roof mShadowY rides the
shingle ramp (UpdateMotion adds the GetPosYBasedOnRow slope delta every tick,
Projectile.cpp:745-761), on the flat it sits a fixed gap under the shot.
Puffs cast no shadow at all (repvz returns early) and the fume cloud is no
projectile to begin with (Plant::Fire -> DoRowAreaDamage).
*/
void Bullet::display_shadow()
{
    if (type == PUFF || type == FUME)
        return;
    int shadow_y;
    if (roof_splats)
        // Bound straight shot (pea/snowpea/spike/star): the spawn shadow gap
        // plus whatever the roof has climbed under the shot since the muzzle.
        shadow_y = y_location + slope_gap0 + roof_slope_offset(x_location) - slope_cont0;
    else
        // Flat ground or an unbound lobbed arc: the row floor, following the
        // roof ramp under the shot when there is one.
        shadow_y = cells[row][0].y2 - 13 + roof_slope_offset(x_location);
    // Center the blob under the shot: draw() anchors x_location at the
    // sprite's left edge, the shadow art stretches with the world knob.
    const int sw = world_px(21);
    win.draw_png(BULLET_SHADOW_DIRECTORY, x_location + (width - sw) / 2, shadow_y, sw, world_px(9));
}

void move_bullets(std::vector<Bullet *> &bullets)
{
    for (auto &bullet : bullets)
    {
        bullet->move();
    }
}

/*Bind the roof-slope splat: capture the spawn shadow gap (repvz
ProjectileInitialize: mShadowY = GridToPixelY(col, row) + 67,
Projectile.cpp:53) and the continuous slope offset at the muzzle (repvz
Board::GetPosYBasedOnRow, Board.cpp:9226). Called once y_location is set, by
every straight shot. Fume/lobbed shots never bind: the fume-shroom bursts
row-area damage without a projectile (Plant::Fire -> DoRowAreaDamage,
Plant.cpp:4548) and lobbed motion never reaches CheckForHighGround
(UpdateMotion, Projectile.cpp:736).*/
void Bullet::bind_roof_slope()
{
    if (!level.is_roof())
        return;
    slope_gap0 = cells[row][0].y1 + roof_tile_dy(get_block_col(x_location)) + 67 - y_location;
    slope_cont0 = roof_slope_offset(x_location);
    roof_splats = true;
}

/*Repvz Projectile::UpdateMotion (Projectile.cpp:745-761): the shadow follows
the sloped roof line as the shot flies, shrinking the gap by 0.25 px per px
flown; CheckForHighGround (Projectile.cpp:331) splats the shot the tick the
gap dips under its threshold: pea/snowpea/spike 28, puff 0, star 23.*/
bool Bullet::splats_on_roof(const int &threshold)
{
    if (!roof_splats)
        return false;
    return slope_gap0 + roof_slope_offset(x_location) - slope_cont0 < threshold;
}