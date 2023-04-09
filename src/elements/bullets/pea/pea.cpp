#include "pea.hpp"
#define PEA_WIDTH 25
#define PEA_HEIGHT 25
#define PEA_EXPLODE_TIME 35

extern Map cells;
extern window win;

Pea::Pea()
{
}

/*Generate new pea*/
Pea::Pea(int _type, int _row, int _x)
{
    type = _type;
    row = _row;
    x_location = _x;
    if (type == 1)
    {
        directory_num = PEA_DIRECTORY;
        explode = PEA_EXPLODE_TIME;
    }
    else if (type == 2)
    {
        directory_num = SNOWZ_PEA_DIRECTORY;
        explode = PEA_EXPLODE_TIME;
    }
}

/*
@return 'true' if pea reach the zombie
*/
bool has_pea_reached_zombie(Pea &pea, Zombie &zombie)
{
    if (zombie.row == pea.row &&
        pea.x_location > zombie.x_location + 70)
        return true;
    return false;
}

/*
For all pea and all zombie to check their collision.
One pea hit only one zombies at a time.
*/
void handle_pea_zombie_encounter(vector<Pea> &peas, vector<Zombie> &zombies, vector<DeadZombie> &dead_zombies)
{
    for (int i = 0; i < (int)peas.size(); i++)
    {
        if (peas[i].directory_num == PEA_EXPLODE_DIRECTORY ||
            peas[i].directory_num == SNOWZ_PEA_EXPLODE_DIRECTORY)
        {
            if (--peas[i].explode == 0)
            {
                peas.erase(peas.begin() + i);
                i--;
            }
            continue;
        }
        for (int j = 0; j < (int)zombies.size(); j++)
            if (apply_pea_hitting_zombie(zombies, dead_zombies, peas[i], j))
            {
                j--;
                break;
            }
    }
}

/*
If a pea collide with a zombie: apply it to hit the zombie:
    Pea -> crashed pea.
    Snow pea will slow zombies.
    Zombie decrease health & blink.
    Add zombies' death.
    Has some probabilities not hit zombie =))
*/
bool apply_pea_hitting_zombie(vector<Zombie> &zombies, vector<DeadZombie> &dead_zombies, Pea &pea, const int &z_ind)
{
    if (has_pea_reached_zombie(pea, zombies[z_ind]))
    {
        // Sound effects
        if (pea.type == 2)
        {
            play_sound_effect(SNOW_PEA_SPARKLES_DIRECTORY);
        }
        if (zombies[z_ind].type == BUCKET_TYPE)
        {
            play_sound_effect(SHIELD_HIT_MUSIC_DIRECTORY);
        }
        else
        {
            play_sound_effect(PEA_CRASH_MUSIC_DIRECTORY);
        }
        // Snowz peas effects
        if (pea.type == 2)
        {
            zombies[z_ind].cold_time = MAX_COLD_TIME;
        }
        // Pea explode
        if (pea.directory_num == PEA_DIRECTORY ||
            pea.directory_num == SNOWZ_PEA_DIRECTORY)
        {
            pea.directory_num++; // Make the pea explode
        }
        // Attack zombie
        if (zombies[z_ind].decrease_health(dead_zombies))
        {
            play_sound_effect(ZOMBIE_FALLING_MUSIC_DIRECTORY);
            DeadZombie tmp(zombies[z_ind].row, zombies[z_ind].x_location, zombies[z_ind].cold_time > 0);
            dead_zombies.push_back(tmp);
            zombies.erase(zombies.begin() + z_ind);
        }
        return true;
    }
    return false;
}

/*
Return true if pea can move
    + If pea go out the playground: false
    + If pea reach any zombie: false
    + Else: true
*/
bool can_pea_move(Pea &pea, vector<Zombie> &zombies)
{
    int right_bound = WINDOW_WIDTH;
    if (pea.x_location > right_bound)
        return false;
    for (auto &zombie : zombies)
        if (has_pea_reached_zombie(pea, zombie))
            return false;
    return true;
}

/*
Move the pea: For all pea:
    + If pea can move, its location += their speed.
*/
void move_peas(vector<Pea> &peas, vector<Zombie> &zombies)
{
    for (int i = 0; i < (int)peas.size(); i++)
    {
        if (can_pea_move(peas[i], zombies))
        {
            peas[i].x_location += PEA_DX;
        }
        else
        {
            peas.erase(peas.begin() + i);
            i--;
        }
    }
}

/*
Display the peas
*/
void display_peas(vector<Pea> &peas)
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
