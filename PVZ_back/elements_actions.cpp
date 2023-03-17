#include "elements_actions.h"

/*Updated
Handles all the changes to the game:
    + Update all zombies' moving status.
    + Check all bullets' moving status.
    + Create new wave of zombies. (if level has finised and that's time to create new wave)
    + Make zombies' bite to plants if necessary.
    + Moving all the bullets.
    + Generate suns from the sky.
    + Generate suns from sunflowers.
Updated: Suns from sunflowers not at the same time.
*/
void handle_changes(Icons &icons, Elements &elements, Map &map, Level &level, int clk)
{
    update_moving_status_for_zombies(elements, map);

    handle_pea_zombie_encounter(elements, map);
    if (level.waves_finished == false && clk % ZOMBIE_CREATE_CLK_COUNT == 0)
        create_new_zombies(elements, level);
    if (clk % BITE_CLK_COUNT == 0)
    {
        handle_zombie_plant_encounter(elements.zombies, elements.walnuts, map, WALNUT_BITE_LIMIT);
        handle_zombie_plant_encounter(elements.zombies, elements.peashooters, map, PEASHOOTER_BITE_LIMIT);
        handle_zombie_plant_encounter(elements.zombies, elements.sunflowers, map, SUNFLOWER_BITE_LIMIT);
    }
    fire_peas(elements, map);

    if (clk % SUN_GEN_SKY_CLK_COUNT == 0)
        gen_random_sun_from_sky(elements);

    gen_sun_from_all_sunflowers(elements, map);
    remove_suns(elements);
    update_remaining_time(icons);
}

/*
- Create a new zombie on random row and at ZOMBIE_INIT_X
- If the level has not finished:
    + Generate zombies for current wave and current second.
    + Then move to the next second and maybe next wave.
*/
void create_new_zombies(Elements &elements, Level &level)
{
    srand(time(NULL));
    Zombie temp;
    temp.x_location = ZOMBIE_INIT_X + rand() % 100;
    temp.health = ZOMBIE_NORMAL_HEALTH_LIMIT;
    temp.is_moving = true;

    if (level.waves_finished == false)
    {
        // Generate zombies for current wave and current second.
        int zombie_cnt = level.zombie_distr_for_wave[level.cur_wave][level.cur_sec];
        temp.row = rand() % 5;
        for (int i = 0; i < zombie_cnt; i++)
        {
            elements.zombies.push_back(temp);
            temp.row = (temp.row + 1 < 4) ? temp.row + 1 : 0;
        }
        // Move to the next second and maybe next wave
        if (level.cur_sec + 1 < level.wave_duration[level.cur_wave])
            level.cur_sec++;
        else if (level.cur_wave + 1 < level.wave_count)
        {
            level.cur_sec = 0;
            level.cur_wave++;
        }
        else
            level.waves_finished = true;
    }
}

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Collision Detection
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

/*
Handle all movement is happening: zombies, suns, peas.
@param clk: the clock of game
*/
void handle_movements(Elements &elements, Map &map, int clk)
{
    if (clk % ZOMBIE_CLK_COUNT == 0)
        move_zombies(elements.zombies, elements, map);
    if (clk % SUN_CLK_COUNT == 0)
        move_suns(elements.suns, map);
    if (clk % PEA_CLK_COUNT == 0)
        move_peas(elements.peas, elements, map);
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
}