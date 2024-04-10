#include "BulletVsZombie.hpp"

/*
@return 'true' if pea reach the zombie
*/
bool has_bullet_reached_zombie(const Bullet &bullet, const Zombie &zombie)
{
    if (bullet.row == zombie.row &&
        bullet.x_location > zombie.x_location + 70)
        return true;
    return false;
}

/*
For all pea and all zombie to check their collision.
One pea hit only one zombies at a time.
*/
void handle_bullet_encounter_zombie(vector<Bullet *> &bullets, vector<Zombie *> &zombies, vector<ZombiePart> &zombie_parts)
{
    // sort zombies -> just find the first zombie to attack
    vector<Zombie *> tmp = zombies;
    stable_sort(tmp.begin(), tmp.end(), [](const Zombie *a, const Zombie *b) -> bool
                { return a->x_location == b->x_location ? a->cold_time < b->cold_time : a->x_location < b->x_location; });

    // sort bullets -> farthest pea will hit first
    sort(bullets.begin(), bullets.end());
    for (int i = 0; i < (int)bullets.size(); i++)
    {
        // Remove disappeared pea
        if (bullets[i]->is_disappeared())
        {
            bullets.erase(bullets.begin() + i);
            i--;
            continue;
        }
        if (bullets[i]->has_exploded())
            continue;
        // Apply pea hit zombie & delete died zombies
        for (int j = 0; j < (int)tmp.size(); j++)
            if (apply_bullet_hit_zombie(*bullets[i], *tmp[j], zombie_parts))
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

void snow_pea_effect(Bullet &bullet, Zombie &zombie)
{
    if (bullet.type == SNOWZPEA && (zombie.type != DOOR_TYPE))
    {
        // If zombie is not cold, its next step and bite will be delay
        if (!zombie.cold_time)
        {
            zombie.bite_time *= 2;
            zombie.next_step_time *= 2;
        }
        zombie.cold_time = MAX_COLD_TIME;
    }
}

/*
If a pea collide with a zombie: apply it to hit the zombie:
    Pea -> crashed pea.
    Snow pea will slow zombies at once.
    Zombie decrease health & blink.
    Add zombies' death.
*/
bool apply_bullet_hit_zombie(Bullet &bullet, Zombie &zombie, vector<ZombiePart> &zombie_parts)
{
    if (has_bullet_reached_zombie(bullet, zombie))
    {
        // Sound effects
        if (zombie.type == BUCKET_TYPE || zombie.type == DOOR_TYPE)
            play_sound_effect(SHIELD_HIT_MUSIC_DIRECTORY);
        else
            bullet.crash_effect();

        // Snowz peas effects
        snow_pea_effect(bullet, zombie);

        // Bullet explode
        bullet.make_explode();

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