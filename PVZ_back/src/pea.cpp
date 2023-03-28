#include "pea.h"

/*
@return 'true' if pea reach the zombie
*/
bool has_pea_reached_zombie(Pea pea, Zombie zombie)
{
    if (zombie.row == pea.row &&
        pea.x_location > zombie.x_location + 70)
        return true;
    return false;
}

/*Updated
For all pea and all zombie to check their collision.
Old version: one pea hit many zombies at a time.
Updated: one pea hit only one zombies at a time.
*/
void handle_pea_zombie_encounter(Elements &elements, Map &cells)
{
    for (int i = 0; i < (int)elements.peas.size(); i++)
    {
        if (elements.peas[i].directory_num == PEA_EXPLODE_DIRECTORY ||
            elements.peas[i].directory_num == SNOWZ_PEA_EXPLODE_DIRECTORY)
        {
            if (--elements.peas[i].explode == 0)
            {
                elements.peas.erase(elements.peas.begin() + i);
            }
            continue;
        }
        for (int j = 0; j < (int)elements.zombies.size(); j++)
            if (apply_pea_hitting_zombie(elements, elements.peas[i], j))
            {
                break;
            }
    }
}

/*
If a pea collide with a zombie: apply it to hit the zombie
Updated:
    Zombie blink.
    Remove zombie's 2 appearances.
    Add zombies' death struct.
*/
bool apply_pea_hitting_zombie(Elements &elements, Pea &pea, int z_ind)
{
    if (has_pea_reached_zombie(pea, elements.zombies[z_ind]))
    {
        if (elements.zombies[z_ind].type == BUCKET_TYPE)
        {
            play_sound_effect(SHIELD_HIT_MUSIC_DIRECTORY);
        }
        else
        {
            play_sound_effect(PEA_CRASH_MUSIC_DIRECTORY);
        }
        if (pea.type == 2)
        {
            elements.zombies[z_ind].cold_time = MAX_COLD_TIME;
        }
        if (pea.directory_num == PEA_DIRECTORY ||
            pea.directory_num == SNOWZ_PEA_DIRECTORY)
        {
            pea.directory_num++; // Make the pea explode
        }
        if (elements.zombies[z_ind].decrease_health())
        {
            DeadZombie tmp;
            tmp.row = elements.zombies[z_ind].row;
            tmp.x_location = elements.zombies[z_ind].x_location;
            if (elements.zombies[z_ind].cold_time)
                tmp.is_cold = true;
            elements.dead_zombies.push_back(tmp);
            elements.zombies.erase(elements.zombies.begin() + z_ind);
        }
        return true;
    }
    return false;
}

/*
Move the pea: For all pea:
    + If pea can move, its location += their speed.
*/
void move_peas(vector<Pea> &peas, Elements &elements, Map &cells)
{
    for (int i = 0; i < (int)peas.size(); i++)
    {
        if (can_pea_move(peas[i], elements, cells))
            peas[i].x_location += PEA_DX;
        else
        {
            peas.erase(peas.begin() + i);
        }
    }
}

/*
Return true if pea can move
    + If pea go out the playground: false
    + If pea reach any zombie: false
    + Else: true
*/
bool can_pea_move(Pea &pea, Elements &elements, Map &cells)
{
    int right_bound = WINDOW_WIDTH;
    if (pea.x_location > right_bound)
        return false;
    for (auto &zombie : elements.zombies)
        if (has_pea_reached_zombie(pea, zombie))
            return false;
    return true;
}

/*
Display the peas
*/
void display_peas(window &win, vector<Pea> &peas, Map &cells)
{
    for (auto &pea : peas)
    {
        int row = pea.row;
        int y_location = cells[row][0].y1 + 20;
        int more_px = 0;
        if (pea.directory_num == PEA_EXPLODE_DIRECTORY ||
            pea.directory_num == SNOWZ_PEA_EXPLODE_DIRECTORY)
            more_px += 25;
        win.draw_png_scale(pea.directory_num, pea.x_location, y_location, PEA_WIDTH + more_px, PEA_HEIGHT + more_px);
    }
}
