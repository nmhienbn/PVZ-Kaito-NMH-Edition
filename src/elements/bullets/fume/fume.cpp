#include "fume.hpp"
#include "../bullet.hpp"
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Map/Map.hpp"
#include "elements/Level/Level.hpp" // roof_tile_dy
#include "elements/zombies/zombie_struct.hpp"
#include "music/music.hpp"

extern Map cells;
extern Window win;

// The cloud soaks ~4 tiles ahead of the muzzle (AttackPlantVsZombie uses the
// ONE_ROW_AHEAD_FOUR rect for triggering).
#define FUME_RANGE_PX (4 * BLOCK_WIDTH)
// The original fume is not a travelling projectile at all: the whole attack
// rect (mX+60 .. mX+400) takes the 20-damage soak on the launch tick (repvz
// Plant::Fire, Plant.cpp:4546-4551). The clone cloud visual paces the soak;
// fly it at pea flight pace (HEAD: PEA_DX = 10 px/tick) so the far end lands
// in the same ~32 ticks a pea needs, not slower.
#define FUME_SPEED 10

/*Construct the spore cloud at the fume-shroom's nozzle*/
Fume::Fume(int _row, int _x) : Bullet(FUME, _row, _x)
{
    width = 90;
    height = 70;
    spawn_x = _x;
    directory_num = FUME_BULLET_DIRECTORY;
    y_location = cells[row][0].y1 + 25 + roof_tile_dy(get_block_col(_x));
}

void Fume::move()
{
    if (x_location <= WINDOW_WIDTH && !has_exploded())
    {
        x_location += FUME_SPEED;
        if (x_location - spawn_x > FUME_RANGE_PX)
            make_explode(); // the cloud disperses
    }
}

void Fume::display()
{
    const int dw = world_px(width), dh = world_px(height);
    win.draw_png(directory_num, x_location + (width - dw) / 2,
                 y_location + (height - dh) / 2, dw, dh);
}

void Fume::crash_effect()
{
    play_sound_effect(FUME_MUSIC_DIRECTORY);
}

void Fume::make_explode()
{
    explode = BULLET_EXPLODE_TIME;
}

/*
Piercing hit: each zombie soaked by the cloud takes one hit per cloud
instance; shields do not protect (fume goes through doors and newspapers).
@return true only the first time this cloud reaches the zombie.
*/
bool Fume::try_hit(Zombie &zombie)
{
    if (row != zombie.row || zombie.is_charmed)
        return false;
    if (x_location + width / 2 < zombie.x_location + 40 ||
        x_location > zombie.x_location + ZOMBIE_EXACT_LOCATION)
        return false;
    if (!hit_already.insert(&zombie).second)
        return false;
    return true;
}
