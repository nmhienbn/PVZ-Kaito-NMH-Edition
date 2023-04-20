#include "pea.hpp"
#define PEA_WIDTH 25
#define PEA_HEIGHT 25
#define PEA_EXPLODE_TIME 15

extern Map cells;
extern window win;

Pea::Pea()
{
}

/*Construct new pea*/
Pea::Pea(int _type, int _row, int _x)
{
    type = _type;
    row = _row;
    x_location = _x;
    if (type == 1)
    {
        directory_num = PEA_DIRECTORY;
    }
    else if (type == 2)
    {
        directory_num = SNOWZ_PEA_DIRECTORY;
    }
    explode = INF;
}

/*
@return 'true' if pea reach the zombie
*/
bool Pea::has_reached_zombie(Zombie &zombie)
{
    if (zombie.row == row &&
        x_location > zombie.x_location + 70)
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
        if (peas[i].is_disappeared())
        {
            peas.erase(peas.begin() + i);
            i--;
            continue;
        }
        if (peas[i].has_exploded())
            continue;
        {
        }
        for (int j = 0; j < (int)zombies.size(); j++)
            if (peas[i].apply_hitting_zombie(zombies, dead_zombies, j))
            {
                j--;
                break;
            }
    }
}

/*
If a pea collide with a zombie: apply it to hit the zombie:
    Pea -> crashed pea.
    Snow pea will slow zombies at once.
    Zombie decrease health & blink.
    Add zombies' death.
    Has some probabilities not hit zombie =))
*/
bool Pea::apply_hitting_zombie(vector<Zombie> &zombies, vector<DeadZombie> &dead_zombies, const int &z_ind)
{
    if (has_reached_zombie(zombies[z_ind]))
    {
        // Sound effects
        if (type == 2)
        {
            play_sound_effect(SNOW_PEA_SPARKLES_DIRECTORY);
        }
        if (zombies[z_ind].type == BUCKET_TYPE || zombies[z_ind].type == DOOR_TYPE)
        {
            play_sound_effect(SHIELD_HIT_MUSIC_DIRECTORY);
        }
        else
        {
            play_sound_effect(PEA_CRASH_MUSIC_DIRECTORY);
        }
        // Snowz peas effects
        if (type == 2 && (zombies[z_ind].type != DOOR_TYPE))
        {
            // If zombie is not cold, its next step and bite will be delay
            if (!zombies[z_ind].cold_time)
            {
                zombies[z_ind].bite_time *= 2;
                zombies[z_ind].next_step_time *= 2;
            }
            zombies[z_ind].cold_time = MAX_COLD_TIME;
        }
        // Pea explode
        if (directory_num == PEA_DIRECTORY ||
            directory_num == SNOWZ_PEA_DIRECTORY)
        {
            directory_num++; // Make the pea explode
            explode = PEA_EXPLODE_TIME;
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

bool Pea::has_exploded()
{
    return explode <= PEA_EXPLODE_TIME;
}

bool Pea::is_disappeared()
{
    int right_bound = WINDOW_WIDTH;
    return --explode == 0 || x_location > right_bound;
}

/*
Return true if pea can move
    + If pea go out the playground: false
    + If pea reach any zombie: false
    + Else: true
*/
void Pea::move()
{
    int right_bound = WINDOW_WIDTH;
    if (x_location <= right_bound)
    {
        if (!has_exploded())
        {
            x_location += PEA_DX;
        }
    }
}

/*
Move the pea: For all pea:
    + If pea can move, its location += their speed.
*/
void move_peas(vector<Pea> &peas)
{
    for (auto &pea : peas)
    {
        pea.move();
    }
}

/*
Display the peas
*/
void Pea::display()
{
    int y_location = cells[row][0].y1 + 20;
    int more_px = 0;
    if (directory_num == PEA_EXPLODE_DIRECTORY ||
        directory_num == SNOWZ_PEA_EXPLODE_DIRECTORY)
        more_px += 25;
    win.draw_png_scale(directory_num, x_location, y_location, PEA_WIDTH + more_px, PEA_HEIGHT + more_px);
}

void Pea::display_shadow()
{
    win.draw_png(PEA_SHADOW_DIRECTORY, x_location, cells[row][0].y2 - 13, 21, 9);
}
