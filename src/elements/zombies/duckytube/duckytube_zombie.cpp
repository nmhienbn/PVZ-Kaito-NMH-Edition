#include "duckytube_zombie.hpp"
#include "draw/rsdl.hpp"
#include "elements/Map/Map.hpp"
#include "music/music.hpp"
#include "reanim/one_shot_fx.hpp"
#include "reanim/reanim_zombie.hpp"

extern Map cells;

DuckyTubeZombie::~DuckyTubeZombie()
{
}

void DuckyTubeZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = *ZOMBIE_HEALTH_LIMIT[type].rbegin();
    directory_num = DUCKYTUBE_WALK_DIRECTORY;
}

void DuckyTubeZombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - dir_height);
    directory_num = DUCKYTUBE_WALK_DIRECTORY;
}

bool DuckyTubeZombie::on_move_tick()
{
    // First tick on the lawn: the tube splashes into the pool lane.
    if (!entered_water)
    {
        entered_water = true;
        play_sound_effect(ZOMBIE_ENTERING_WATER_MUSIC_DIRECTORY);
        reanimfx::spawn(reanimfx::FX_SPLASH, x_location + 85,
                        cells[row][0].y1 + 55, row);
    }
    return false;
}

/*Degrade: the tube never breaks; only the arm pops off at critical health
(same beat as the basic zombie, but the ducky state slots stay the walk/eat
ones so the reanim keeps the tube visible).*/
void DuckyTubeZombie::determine_appearance(vector<ZombiePart> &zombie_parts)
{
    if (!armless && health <= 5 * health_scale)
    {
        armless = true;
        if (reanimz::ready())
            zombie_parts.push_back(ZombiePart(reanimz::DROP_ARM_IMG, 1, row, x_location,
                                              ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0));
        else
            zombie_parts.push_back(ZombiePart(ZOMBIE_ARM_DIRECTORY, 1, row, x_location,
                                              ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0));
    }
}

void DuckyTubeZombie::add_zombie_die(vector<ZombiePart> &zombie_parts)
{
    // Plain skin death; the corpse lies in the water without the snorkel-era
    // head pop (skin-specific drop parts are only authored for skins 1..4).
    Zombie::add_zombie_die(zombie_parts);
}
