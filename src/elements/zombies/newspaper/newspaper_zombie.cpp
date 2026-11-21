#include "newspaper_zombie.hpp"
#include "draw/rsdl.hpp"
#include "music/music.hpp"
#include "reanim/reanim_zombie.hpp"

NewspaperZombie::~NewspaperZombie()
{
}

void NewspaperZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = *ZOMBIE_HEALTH_LIMIT[NEWSPAPER_TYPE].rbegin();
    directory_num = NEWSPAPER_WALK_DIRECTORY;
}

void NewspaperZombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - dir_height);
    directory_num = NEWSPAPER_WALK_NOPAPER_DIRECTORY;
}

bool NewspaperZombie::on_move_tick()
{
    if (rage_pause == 0)
        return false;
    if (--rage_pause == 0)
    {
        // Gasp over: sprint (or chew) madly from here on.
        directory_num = is_moving ? NEWSPAPER_WALK_NOPAPER_DIRECTORY
                                  : NEWSPAPER_EAT_NOPAPER_DIRECTORY;
        frame = 0;
        reanim_time = 0.0f;
    }
    return true; // frozen in place while gasping
}

bool NewspaperZombie::can_eat_plant() const
{
    return rage_pause == 0;
}

float NewspaperZombie::speed_px_per_tick() const
{
    return !has_paper && rage_pause == 0 ? NEWSPAPER_RAGE_PX : Zombie::speed_px_per_tick();
}

/*Paper degrade -> paper torn (gasp beat) -> arm off at low health. All
thresholds, not exact hits: a melon (4 dmg) can skip a tier's exact value.*/
void NewspaperZombie::determine_appearance(vector<ZombiePart> &zombie_parts)
{
    if (health <= NEWSPAPER_ARM_HEALTH * health_scale && armor_dir != reanimz::PAPER_ARMLESS_STATE)
    {
        armor_dir = reanimz::PAPER_ARMLESS_STATE;
        if (reanimz::ready())
            zombie_parts.push_back(ZombiePart(reanimz::DROP_ARM_IMG, 1, row, x_location,
                                              ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                              reanimz::SKIN_NEWSPAPER));
        else
            zombie_parts.push_back(ZombiePart(ZOMBIE_ARM_DIRECTORY, 1, row, x_location,
                                              ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0));
    }
    if (!has_paper)
        return;
    // Worst-first so a hit crossing both dmg tiers lands on the worst art.
    if (health <= NEWSPAPER_PAPER_DMG3_HEALTH * health_scale)
        armor_dir = reanimz::PAPER_DAMAGED3_STATE;
    else if (health <= NEWSPAPER_PAPER_DMG2_HEALTH * health_scale)
        armor_dir = reanimz::PAPER_DAMAGED2_STATE;
    if (health <= NEWSPAPER_PAPER_BREAK_HEALTH * health_scale)
    {
        has_paper = false;
        armor_dir = -1;
        rage_pause = NEWSPAPER_GASP_TICKS;
        is_moving = false;
        directory_num = NEWSPAPER_GASP_DIRECTORY;
        frame = 0;
        reanim_time = 0.0f;
        play_sound_effect(NEWSPAPER_RIP_MUSIC_DIRECTORY);
        play_sound_effect(NEWSPAPER_RARRGH_SND_MUSIC_DIRECTORY);
    }
}

void NewspaperZombie::add_zombie_die(vector<ZombiePart> &zombie_parts)
{
    if (reanimz::ready())
    {
        zombie_parts.push_back(ZombiePart(reanimz::DEATH_PART_IMG, 1, row, x_location,
                                          ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                          reanimz::SKIN_NEWSPAPER));
        return;
    }
    Zombie::add_zombie_die(zombie_parts);
}
