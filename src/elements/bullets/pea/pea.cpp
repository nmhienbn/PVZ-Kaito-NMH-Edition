#include "pea.hpp"
#include <algorithm>
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
        pea_width = 26;
        pea_height = 25;
        directory_num = PEA_DIRECTORY;
    }
    else if (type == 2)
    {
        pea_width = 36;
        pea_height = 25;
        directory_num = SNOWZ_PEA_DIRECTORY;
    }
    explode = INF;
}

/*
x_location > other.x_location
*/
bool Pea::operator<(const Pea &other) const
{
    return x_location > other.x_location;
}

/*
@return 'true' if pea reach the zombie
*/
bool Pea::has_reached_zombie(const Zombie &zombie)
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
void handle_pea_zombie_encounter(vector<Pea> &peas, vector<Zombie *> &zombies, vector<ZombiePart> &zombie_parts)
{
    // sort zombies -> just find the first zombie to attack
    vector<Zombie *> tmp = zombies;
    stable_sort(tmp.begin(), tmp.end(), [](const Zombie *a, const Zombie *b) -> bool
                { return a->x_location == b->x_location ? a->cold_time < b->cold_time : a->x_location < b->x_location; });
    // sort peas -> farthest pea will hit first
    sort(peas.begin(), peas.end());
    for (int i = 0; i < (int)peas.size(); i++)
    {
        // Remove disappeared pea
        if (peas[i].is_disappeared())
        {
            peas.erase(peas.begin() + i);
            i--;
            continue;
        }
        if (peas[i].has_exploded())
            continue;
        // Apply pea hit zombie & delete died zombies
        for (int j = 0; j < (int)tmp.size(); j++)
            if (peas[i].apply_hitting_zombie(*tmp[j], zombie_parts))
            {
                tmp.erase(tmp.begin() + j);
                break;
            }
        for (int j = 0; j < (int)zombies.size(); j++)
            if (zombies[j]->get_health() == 0)
            {
                delete zombies[j];
                zombies.erase(zombies.begin() + j);
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
*/
bool Pea::apply_hitting_zombie(Zombie &zombie, vector<ZombiePart> &zombie_parts)
{
    if (has_reached_zombie(zombie))
    {
        // Sound effects
        if (type == 2)
        {
            play_sound_effect(SNOW_PEA_SPARKLES_DIRECTORY);
        }
        if (zombie.type == BUCKET_TYPE || zombie.type == DOOR_TYPE)
        {
            play_sound_effect(SHIELD_HIT_MUSIC_DIRECTORY);
        }
        else
        {
            play_sound_effect(PEA_CRASH_MUSIC_DIRECTORY);
        }
        // Snowz peas effects
        if (type == 2 && (zombie.type != DOOR_TYPE))
        {
            // If zombie is not cold, its next step and bite will be delay
            if (!zombie.cold_time)
            {
                zombie.bite_time *= 2;
                zombie.next_step_time *= 2;
            }
            zombie.cold_time = MAX_COLD_TIME;
        }
        // Pea explode
        if (directory_num == PEA_DIRECTORY ||
            directory_num == SNOWZ_PEA_DIRECTORY)
        {
            directory_num++; // Make the pea explode
            explode = PEA_EXPLODE_TIME;
        }
        // Attack zombie
        if (zombie.decrease_health(zombie_parts))
        {
            play_sound_effect(ZOMBIE_FALLING_MUSIC_DIRECTORY);
            zombie.add_zombie_die(zombie_parts);
        }
        return true;
    }
    return false;
}

/*
check if a pea has exploded or not.
initialized pea will explode after PEA_EXPLODE_TIME.
*/
bool Pea::has_exploded()
{
    return explode <= PEA_EXPLODE_TIME;
}

/*
Check if a pea disappeared
A pea will disappear if it exploded or moved out the screen
*/
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
    win.draw_png_scale(directory_num, x_location - more_px / 2, y_location - more_px / 2,
                       pea_width + more_px, pea_height + more_px);
}

/*
Display the shadow under the pea
*/
void Pea::display_shadow()
{
    win.draw_png(PEA_SHADOW_DIRECTORY, x_location, cells[row][0].y2 - 13, 21, 9);
}
