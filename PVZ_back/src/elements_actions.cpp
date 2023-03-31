#include "elements_actions.h"
static bool zombie_has_coming = false;

/*
Handles all the changes to the game:
    + Update all zombies' moving status.
    + Check all bullets' moving status.
    + Create new wave of zombies. (if level has finised and that's time to create new wave)
    + Make zombies' bite to plants and update the next time they can bite them.
    + Fire all the bullets.
    + Generate suns from the sky (not same time). No sun at night.
    + Generate suns from sunflowers (not same time).
    + Remove the suns that appear for a long time.
    + Update remaining time of each plant seed.
*/
void handle_changes(Icons &icons, Elements &elements, Map &cells, Level &level, int clk)
{ // Update all zombies' moving status
    update_moving_status_for_zombies(elements, cells);

    // Check all bullets' moving status
    handle_pea_zombie_encounter(elements, cells);

    // Check all cherrybombs' status
    handle_cherrybomb_zombie_encounter(elements, cells);

    // Create new wave of zombies. (if level has finised and that's time to create new wave)
    if (level.waves_finished == false && clk % ZOMBIE_CREATE_CLK_COUNT == 0)
        create_new_zombies(elements, level);
    else if (clk % ZOMBIE_CREATE_CLK_COUNT == ANNOUNCER_CLK_COUNT)
        level.announce_directory = NULL_DIRECTORY;

    // Zombie bite plant
    handle_zombie_plant_encounter(elements.zombies, elements.peashooters, cells, PEASHOOTER_BITE_LIMIT);
    handle_zombie_plant_encounter(elements.zombies, elements.sunflowers, cells, SUNFLOWER_BITE_LIMIT);
    handle_zombie_plant_encounter(elements.zombies, elements.walnuts, cells, WALNUT_BITE_LIMIT);
    handle_zombie_plant_encounter(elements.zombies, elements.snowpeas, cells, SNOWPEA_BITE_LIMIT);
    handle_zombie_plant_encounter(elements.zombies, elements.cherrybombs, cells, CHERRYBOMB_BITE_LIMIT);

    // Update next time for each zombie to bite plant
    update_zombie_next_bite(elements.zombies);

    // Fire pea, snowpea
    fire_peas(elements, cells);
    fire_snowz_peas(elements, cells);

    // Generate suns from the sky. No sun at night.
    if (level.is_night == false && clk % SUN_GEN_SKY_CLK_COUNT == 0)
        gen_random_sun_from_sky(elements);

    // Generate suns from sunflowers.
    gen_sun_from_all_sunflowers(elements, cells);

    // Remove the suns that appear for a long time
    remove_suns(elements);

    // Update remaining time of each plant seed
    update_remaining_time(icons);
}

/*
- Create a new zombie on random row and at ZOMBIE_INIT_X
- If the level has not finished:
    + Generate zombies for current wave and current second.
    + Then move to the next second and maybe next wave.
    + Reduce the empty time between waves of zombies when all current zombies died (while zombies.empty() loop).
*/
void create_new_zombies(Elements &elements, Level &level)
{
    Zombie temp;
    if (level.waves_finished == false)
    {
        // This loop is to reduce the empty time between waves of zombies when all current zombies died.
        while (elements.zombies.empty())
        {
            // Generate zombies for current wave and current second.
            // For all type of zombies.
            for (int typ = NORMAL_TYPE; typ < COUNT_ZOMBIE_TYPE; typ++)
            {
                // Number of zombies for this second of waves.
                int zombie_cnt = level.zombie_distr_for_wave[typ][level.cur_wave][level.cur_sec];
                // First second of first wave: sound effect "The zombies are coming".
                if (level.cur_wave == 0 && level.cur_sec == 0)
                    zombie_has_coming = false;
                if (zombie_has_coming == false)
                {
                    if (zombie_cnt > 0)
                    {
                        play_sound_effect(ZOMBIE_COMING_MUSIC_DIRECTORY);
                        zombie_has_coming = true;
                    }
                }
                // Sometimes: sound effect "Zombie groan".
                else if (zombie_cnt > 0)
                {
                    play_sound_effect(GROAN_MUSIC_DIRECTORY);
                }
                // Random row of new zombie.
                for (int i = 0; i < zombie_cnt; i++)
                {
                    temp = Zombie(typ);
                    if (level.level_num == 1)
                        temp.row = 2;
                    else if (level.level_num == 2)
                        temp.row = rand(1, 3);
                    elements.zombies.push_back(temp);
                }
            }
            // Move to the next second and maybe next wave.
            // First second of each wave: sound effect "Huge wave".
            if (level.cur_sec + 1 < level.wave_duration[level.cur_wave])
            {
                level.cur_sec++;
            }
            else if (level.cur_wave + 1 < level.wave_count)
            {
                level.cur_sec = 0;
                level.cur_wave++;
                if (level.cur_wave == level.wave_count - 1)
                {
                    play_sound_effect(HUGE_WAVE_MUSIC_DIRECTORY);
                    level.announce_directory = FINAL_WAVE_DIRECTORY;
                    // Make flag zombies
                    temp = Zombie(NORMAL_TYPE, FLAG_ZOMBIE_WALK_DIRECTORY);
                    if (level.level_num == 1)
                        temp.row = 2;
                    else if (level.level_num == 2)
                        temp.row = rand(1, 3);
                    elements.zombies.push_back(temp);
                }
                else if (level.is_huge_wave())
                {
                    play_sound_effect(HUGE_WAVE_MUSIC_DIRECTORY);
                    level.announce_directory = HUGE_WAVE_DIRECTORY;
                    // Make flag zombies
                    temp = Zombie(NORMAL_TYPE, FLAG_ZOMBIE_WALK_DIRECTORY);
                    if (level.level_num == 1)
                        temp.row = 2;
                    else if (level.level_num == 2)
                        temp.row = rand(1, 3);
                    elements.zombies.push_back(temp);
                }
            }
            else
            {
                level.waves_finished = true;
                break;
            }
        }
    }
}

/*
Handle all movement is happening: zombies, suns, peas.
@param clk: the clock of game
*/
void handle_movements(Elements &elements, Map &cells, int clk)
{
    if (clk % ZOMBIE_CLK_COUNT == 0)
        move_zombies(elements.zombies, elements, cells);
    if (clk % SUN_CLK_COUNT == 0)
        move_suns(elements.suns, cells);
    if (clk % PEA_CLK_COUNT == 0)
        move_peas(elements.peas, elements, cells);
}

/*
Update icons remainning time
*/
void update_remaining_time(Icons &icons)
{
    if (icons.peashooter_remaining_time)
        icons.peashooter_remaining_time--;
    if (icons.sunflower_remaining_time)
        icons.sunflower_remaining_time--;
    if (icons.walnut_remaining_time)
        icons.walnut_remaining_time--;
    if (icons.snowpea_remaining_time)
        icons.snowpea_remaining_time--;
    if (icons.cherrybomb_remaining_time)
        icons.cherrybomb_remaining_time--;
}