#include "MowerVsZombie.hpp"

extern int game_state;

/*
Handle mower vs zombie collision
*/
void handle_mower_zombie_encounter(vector<Mower *> &mowers,
                                   vector<Zombie *> &zombies,
                                   vector<ZombiePart> &zombie_parts)
{
    if (check_status(game_state, IS_PAUSED) == true)
        return;
    // active mower if zombie touch
    for (auto &zombie : zombies)
    {
        if (zombie->x_location + ZOMBIE_EXACT_LOCATION < X_UPPER_LEFT - 30)
        {
            active_mower(mowers, zombie->row);
        }
    }
    // mower action
    for (int i = 0; i < (int)mowers.size(); i++)
        if (mowers[i] != nullptr && mowers[i]->status == MOWER_ACTIVE)
        {
            // mower move
            mowers[i]->x += MOWER_DX;

            // mower gone
            if (++mowers[i]->x >= WINDOW_WIDTH)
            {
                mowers[i] = nullptr;
                continue;
            }

            // mower hit zombie
            for (int j = 0; j < (int)zombies.size();)
            {
                if (!apply_mower_hitting_zombie(zombies, j, *mowers[i], zombie_parts))
                {
                    j++;
                }
            }
        }
}

/*
Apply mower hit the zombies. (make zombie die immediately)
*/
bool apply_mower_hitting_zombie(vector<Zombie *> &zombies, const int &z_ind,
                                Mower &mower, vector<ZombiePart> &zombie_parts)
{
    if (is_mower_hit_zombie(mower, *zombies[z_ind]))
    {
        // zombie die
        zombies[z_ind]->add_zombie_die(zombie_parts);
        // armor drop
        if (zombies[z_ind]->type == CONE_TYPE)
            zombie_parts.push_back(ZombiePart(CONE_DROP_DIRECTORY, HEAD_ZOMBIE_FRAME,
                                              zombies[z_ind]->row, zombies[z_ind]->x_location + 80,
                                              HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT));
        if (zombies[z_ind]->type == BUCKET_TYPE)
            zombie_parts.push_back(ZombiePart(BUCKET_DROP_DIRECTORY, HEAD_ZOMBIE_FRAME,
                                              zombies[z_ind]->row, zombies[z_ind]->x_location + 80,
                                              HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT));
        // delete zombie
        delete zombies[z_ind];
        zombies.erase(zombies.begin() + z_ind);
        return true;
    }
    return false;
}

/*
@return 'true' if mower hit the zombie
*/
bool is_mower_hit_zombie(Mower &mower, Zombie &zombie)
{
    if (zombie.row == mower.row &&
        zombie.x_location <= mower.x)
        return true;
    return false;
}