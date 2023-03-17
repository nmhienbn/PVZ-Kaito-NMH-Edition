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
        handle_zombie_walnut_encounter(elements, map);
        handle_zombie_peashooter_encounter(elements, map);
        handle_zombie_sunflower_encounter(elements, map);
    }
    fire_peas(elements, map);

    if (clk % SUN_GEN_SKY_CLK_COUNT == 0)
        gen_random_sun_from_sky(elements);

    gen_sun_from_all_sunflowers(elements, map);
    remove_suns(elements);
    update_remaining_time(icons);
}

/**
Generate suns from sunflowers not at the same time.
Old version: gen all sun at a time.
**/
void gen_sun_from_all_sunflowers(Elements &elements, Map &map)
{
    for (int i = 0; i < elements.sunflowers.size(); i++)
        if (--elements.sunflowers[i].sec_for_another_sun == 0)
        {
            gen_sun_from_a_sunflower(elements, elements.sunflowers[i], map);
        }
}

/* I have changed this for better performance
Update all zombies' moving status.
*/
void update_moving_status_for_zombies(Elements &elements, Map &map)
{
    for (int i = 0; i < elements.zombies.size(); i++)
    {
        has_zombie_reached_any_elements(elements, elements.zombies[i], map);
    }
}

/*Updated
For all pea and all zombie to check their collision.
Old version: one pea hit many zombies at a time.
Updated: one pea hit only one zombies at a time.
*/
void handle_pea_zombie_encounter(Elements &elements, Map &map)
{
    for (int i = 0; i < elements.peas.size(); i++)
        for (int j = 0; j < elements.zombies.size(); j++)
            if (apply_pea_hitting_zombie(elements, i, j))
                break;
}

/*
If a pea collide with a zombie: apply it to hit the zombie
Updated: zombie blink
*/
bool apply_pea_hitting_zombie(Elements &elements, int p_ind, int z_ind)
{
    if (has_pea_reached_zombie(elements.peas[p_ind], elements.zombies[z_ind]))
    {
        elements.zombies[z_ind].health--;
        elements.zombies[z_ind].is_attacked = MAX_TIME_BLINK;
        elements.peas.erase(elements.peas.begin() + p_ind);
        // determine_zombie_appearanc(elements.zombies[z_ind]);
        if (elements.zombies[z_ind].health == 0)
        {
            DeadZombie tmp;
            tmp.row = elements.zombies[z_ind].row;
            tmp.x_location = elements.zombies[z_ind].x_location - DEAD_ZOMBIE_WIDTH + ZOMBIE_G_WIDTH / 2;
            tmp.type_head = rand() % 3;
            elements.dead_zombies.push_back(tmp);
            elements.zombies.erase(elements.zombies.begin() + z_ind);
        }
        return true;
    }
    return false;
}

/* Note to optimize after
For all zombie and all wallnut: apply zombie's bite
*/
void handle_zombie_walnut_encounter(Elements &elements, Map &map)
{
    for (int i = 0; i < elements.walnuts.size(); i++)
        for (int j = 0; j < elements.zombies.size(); j++)
            apply_zombie_bite_on_walnut(elements, j, i, map);
}

/* Note to optimize after
For all zombie and all peashooter: apply zombie's bite
*/
void handle_zombie_peashooter_encounter(Elements &elements, Map &map)
{
    for (int i = 0; i < elements.peashooters.size(); i++)
        for (int j = 0; j < elements.zombies.size(); j++)
            apply_zombie_bite_on_peashooter(elements, j, i, map);
}

/* Note to optimize after
For all zombie and all sunflower: apply zombie's bite
*/
void handle_zombie_sunflower_encounter(Elements &elements, Map &map)
{
    for (int i = 0; i < elements.sunflowers.size(); i++)
        for (int j = 0; j < elements.zombies.size(); j++)
            apply_zombie_bite_on_sunflower(elements, j, i, map);
}

/*
If the zombie reached sunflower, apply its bites on the sunflower:
    + bite++
    + if sunflower's bite == LIMIT: reset zombie moving and delete that sunflower
*/
void apply_zombie_bite_on_sunflower(Elements &elements, int z_ind, int s_ind, Map &map)
{
    if (has_zombie_reached_element(elements.zombies[z_ind], elements.sunflowers[s_ind].row, elements.sunflowers[s_ind].col, map))
    {
        elements.sunflowers[s_ind].bite++;
        elements.sunflowers[s_ind].is_attacked = MAX_TIME_BLINK;
        if (elements.sunflowers[s_ind].bite == SUNFLOWER_BITE_LIMIT)
        {
            elements.zombies[z_ind].is_moving = true;
            map[elements.sunflowers[s_ind].row][elements.sunflowers[s_ind].col].is_planted = 0;
            elements.sunflowers.erase(elements.sunflowers.begin() + s_ind);
        }
    }
}

/*
If the zombie reached peashooter, apply its bites on the peashooter:
    + bite++
    + if peashooter's bite == LIMIT: reset zombie moving and delete that peashooter
*/
void apply_zombie_bite_on_peashooter(Elements &elements, int z_ind, int p_ind, Map &map)
{
    if (has_zombie_reached_element(elements.zombies[z_ind], elements.peashooters[p_ind].row, elements.peashooters[p_ind].col, map))
    {
        elements.peashooters[p_ind].bite++;
        if (elements.peashooters[p_ind].bite == PEASHOOTER_BITE_LIMIT)
        {
            elements.zombies[z_ind].is_moving = true;
            map[elements.peashooters[p_ind].row][elements.peashooters[p_ind].col].is_planted = 0;
            elements.peashooters.erase(elements.peashooters.begin() + p_ind);
        }
    }
}

/*
If the zombie reached wallnut, apply its bites on the wallnut:
    + bite++
    + determine wallnut's apperance
    + if wallnut's bite == LIMIT: reset zombie moving and delete that wallnut
*/
void apply_zombie_bite_on_walnut(Elements &elements, int z_ind, int w_ind, Map &map)
{
    if (has_zombie_reached_element(elements.zombies[z_ind], elements.walnuts[w_ind].row, elements.walnuts[w_ind].col, map))
    {
        elements.walnuts[w_ind].bite++;
        elements.walnuts[w_ind].is_attacked = MAX_TIME_BLINK;
        determine_walnut_appearance(elements.walnuts[w_ind]);
        if (elements.walnuts[w_ind].bite == WALNUT_BITE_LIMIT)
        {
            elements.zombies[z_ind].is_moving = true;
            map[elements.walnuts[w_ind].row][elements.walnuts[w_ind].col].is_planted = 0;
            elements.walnuts.erase(elements.walnuts.begin() + w_ind);
        }
    }
}

/*
bite <= (1/4) * WALNUT_BITE_LIMIT: 1
bite <= (2/4) * WALNUT_BITE_LIMIT: 2
bite <= (3/4) * WALNUT_BITE_LIMIT: 3
bite <= (4/4) * WALNUT_BITE_LIMIT: 4
*/
void determine_walnut_appearance(Walnut &walnut)
{
    if (walnut.bite <= WALNUT_BITE_LIMIT / 4)
    {
        walnut.directory_num = WALNUT_1_DIRECTORY;
        walnut.blink_directory_num = WALNUT_1_BLINK_DIRECTORY;
    }
    else if (walnut.bite <= WALNUT_BITE_LIMIT / 2)
    {
        walnut.directory_num = WALNUT_2_DIRECTORY;
        walnut.blink_directory_num = WALNUT_2_BLINK_DIRECTORY;
    }
    else if (walnut.bite <= WALNUT_BITE_LIMIT * 3 / 4)
    {
        walnut.directory_num = WALNUT_3_DIRECTORY;
        walnut.blink_directory_num = WALNUT_3_BLINK_DIRECTORY;
    }
    else
    {
        walnut.directory_num = WALNUT_4_DIRECTORY;
        walnut.blink_directory_num = WALNUT_4_BLINK_DIRECTORY;
    }
}

/*
health > ZOMBIE_NORMAL_HEALTH_LIMIT / 2: healthy
health <= ZOMBIE_NORMAL_HEALTH_LIMIT / 2: injured
*/
void determine_zombie_appearanc(Zombie &zombie)
{
    if (zombie.health > (ZOMBIE_NORMAL_HEALTH_LIMIT >> 1))
        zombie.directory_num = ZOMBIE_HEALTHY_DIRECTORY;
    else
        zombie.directory_num = ZOMBIE_INJURED_DIRECTORY;
}

/*
Random final row (0-4) and column (0-8) for sun from the sky
Then add it into vector<Sun>
*/
void gen_random_sun_from_sky(Elements &elements)
{
    srand(time(NULL));
    Sun temp;
    temp.final_row = rand() % 5;
    temp.final_col = rand() % 9;
    temp.y_location = INIT_SUN_Y;
    temp.wait_seconds = 0;
    elements.suns.push_back(temp);
}

/*
Generate sun from the sunflowers
Then add it into vector<Sun>
*/
void gen_sun_from_a_sunflower(Elements &elements, Sunflower sunflower, Map &map)
{
    sunflower.sec_for_another_sun = SUN_GEN_SUNFLOWER_CLK_COUNT;
    Sun temp;
    temp.final_col = sunflower.col;
    temp.final_row = sunflower.row;
    temp.y_location = map[sunflower.row][sunflower.col].y1 - 20;
    temp.wait_seconds = 0;
    elements.suns.push_back(temp);
}

/*Updated
For all peashooter: If there is a zombie in that peashooter's row:
Generate a new pea at that peashooter's position.
Old version: all peas are generated at a time.
Updated: all peas are generated not at a time.
*/
void fire_peas(Elements &elements, Map &map)
{
    for (int i = 0; i < elements.peashooters.size(); i++)
    {
        int row = elements.peashooters[i].row;
        int col = elements.peashooters[i].col;
        if (are_there_zombies_in_peashooter_row(elements.peashooters[i], elements))
        {
            if (--elements.peashooters[i].sec_for_another_pea == 0)
            {
                elements.peashooters[i].sec_for_another_pea = FIRE_PEA_CLK_COUNT;
                Pea temp;
                temp.row = row;
                temp.x_location = map[row][col].x2 - 10;
                elements.peas.push_back(temp);
            }
        }
        else
        {
            elements.peashooters[i].sec_for_another_pea = 1;
        }
    }
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
    temp.x_location = ZOMBIE_INIT_X + 40 + rand() % 20;
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

/*
Check if a peashooter need to attack or not.
Peashooter is attack only if there are some zombies in the row.
*/
bool are_there_zombies_in_peashooter_row(Peashooter peashooter, Elements &elements)
{
    for (int i = 0; i < elements.zombies.size(); i++)
        if (peashooter.row == elements.zombies[i].row)
            return true;
    return false;
}

/*
Remove the suns that are exist a long time.
*/
void remove_suns(Elements &elements)
{
    for (int i = 0; i < elements.suns.size(); i++)
        if (elements.suns[i].wait_seconds >= 100)
            elements.suns.erase(elements.suns.begin() + i);
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
For all the sun:
    + If the sun has not reached the ground: move it.
    + Else increase sun-wait time.
*/
void move_suns(vector<Sun> &suns, Map &map)
{
    for (int i = 0; i < suns.size(); i++)
    {
        int row = suns[i].final_row;
        int col = suns[i].final_col;
        int lower_limit = map[row][col].y1 + 30;
        if (suns[i].y_location + SUN_DY < lower_limit)
            suns[i].y_location += SUN_DY;
        else
            suns[i].wait_seconds++;
    }
}

/*
@return 'true' if pea reach the zombie
*/
bool has_pea_reached_zombie(Pea pea, Zombie zombie)
{
    if (zombie.row == pea.row &&
        pea.x_location + PEA_DX > zombie.x_location + 30)
        return true;
    return false;
}

/*
Check if Zombie is in query tile or not.
@param zombie: query zombie
@param row: query row
@param col: query column
@return true if zombie is not in query tile.
*/
bool has_zombie_reached_element(Zombie zombie, int row, int col, Map &map)
{
    int right_limit = map[row][col].x2 - TILE_WIDTH / 2;
    int left_limit = map[row][col].x1 - TILE_WIDTH / 2;
    int zombie_new_location = zombie.x_location - ZOMBIE_DX;
    if (zombie.row == row &&
        left_limit < zombie_new_location && zombie_new_location < right_limit)
        return true;
    return false;
}

/*
Return true if pea can move
    + If pea go out the playground: false
    + If pea reach any zombie: false
    + Else: true
*/
bool can_pea_move(Pea &pea, Elements &elements, Map &map)
{
    int right_bound = map[pea.row][8].x2 - 20;
    if (pea.x_location + PEA_DX > right_bound)
        return false;
    for (int i = 0; i < elements.zombies.size(); i++)
        if (has_pea_reached_zombie(pea, elements.zombies[i]))
            return false;
    return true;
}

/*
Check if zombie can move or not:
    + If zombie go out background: false (because you have lose)
    + If zombie.is_moving is false: false (of course)
    + If zombie reached any element: has_zombie_reached_any_elements will decide it
*/
bool can_zombie_move(Zombie &zombie, Elements &elements, Map &map)
{
    int left_bound = map[0][0].x1 - 40;
    int zombie_new_location = zombie.x_location - ZOMBIE_DX;
    if (zombie_new_location < left_bound)
        return false;
    if (!zombie.is_moving)
        return false;
    if (has_zombie_reached_any_elements(elements, zombie, map))
        return false;
    return true;
}

/*
Check if zombie reach any plant or not.
Update zombies' moving status.
@return true if zombie reach any plant
*/
bool has_zombie_reached_any_elements(Elements &elements, Zombie &zombie, Map &map)
{
    for (int i = 0; i < elements.walnuts.size(); i++)
        if (has_zombie_reached_element(zombie, elements.walnuts[i].row, elements.walnuts[i].col, map))
        {
            zombie.is_moving = false;
            zombie.change_zombie_eating_status();
            return true;
        }
    for (int i = 0; i < elements.peashooters.size(); i++)
        if (has_zombie_reached_element(zombie, elements.peashooters[i].row, elements.peashooters[i].col, map))
        {
            zombie.is_moving = false;
            zombie.change_zombie_eating_status();
            return true;
        }
    for (int i = 0; i < elements.sunflowers.size(); i++)
        if (has_zombie_reached_element(zombie, elements.sunflowers[i].row, elements.sunflowers[i].col, map))
        {
            zombie.is_moving = false;
            zombie.change_zombie_eating_status();
            return true;
        }

    zombie.is_moving = true;
    zombie.change_zombie_eating_status();
    return false;
}

/*
Move the zombie: For all zombie:
    + If zombie can move, its location -= their speed.
*/
void move_zombies(vector<Zombie> &zombies, Elements &elements, Map &map)
{
    for (int i = 0; i < zombies.size(); i++)
    {
        if (can_zombie_move(zombies[i], elements, map))
            zombies[i].x_location -= ZOMBIE_DX;
    }
}

/*
Move the pea: For all pea:
    + If pea can move, its location += their speed.
*/
void move_peas(vector<Pea> &peas, Elements &elements, Map &map)
{
    for (int i = 0; i < peas.size(); i++)
    {
        if (can_pea_move(peas[i], elements, map))
            peas[i].x_location += PEA_DX;
    }
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