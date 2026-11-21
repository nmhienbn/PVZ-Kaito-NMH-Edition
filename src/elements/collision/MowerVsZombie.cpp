#include "MowerVsZombie.hpp"
#include "draw/ImageType.hpp"
#include "elements/Level/Level.hpp"
#include "game_param.hpp"
#include "reanim/reanim_zombie.hpp"

extern int game_state;
extern Level level;

/*Blast-style reach for the mower (repvz LawnMower::Update's kill loop,
LawnMower.cpp:189-216): EffectedByDamage(127) reaches a TUNNELING digger
(DAMAGAGES_UNDERGROUND bit set), but BoulderZombie / the boss only borrow
is_underground() as a hit-immunity flag (boss_zombie.hpp) -- they are not
tunneling, so the blade never touchs them (the original also flat-out skips
ZOMBIE_BOSS, LawnMower.cpp:213). Mind-controlled walkers stay safe too
(mMindControlled filters every damage range without ONLY_MINDCONTROLLED).*/
static bool mower_can_touch(const Zombie &z)
{
    if (z.type == ZOMBOSS_TYPE)
        return false; // repvz hard skip (boss boards carry no mowers anyway)
    if (z.is_charmed)
        return false; // repvz EffectedByDamage: hypnotized are not targets
    if (z.is_underground() && z.type != DIGGER_TYPE)
        return false;
    return true;
}

/*The parked mower only rolls when the zombie that crosses its line can
actually trip it (repvz LawnMower::Update, LawnMower.cpp:194-216): a bungee
dangles above the blade and never sets it off, and a zombie whose health pool
is already spent (the clone has no bleed-out headless walkers, so health <= 0
covers the original's !mHasHead dead-but-standing case) is a corpse in
transit, not a trigger. The trigger line doubles as the mower attack edge:
overlap measured in px against it -- balloon floats high, so it must sink a
full 20 px into the blade before the mower reacts (original's
overlap > (balloon ? 20 : 0)).*/
static bool can_zombie_trip_mower(const Zombie &z)
{
    if (z.type == BUNGEE_TYPE) // hangs over the mower, never trips it
        return false;
    if (z.get_health() <= 0)   // spent pool: dying walker, no trigger
        return false;
    if (z.type == BALLOON_TYPE)
        return (X_UPPER_LEFT - 30) - (z.x_location + ZOMBIE_EXACT_LOCATION) > 20;
    return z.x_location + ZOMBIE_EXACT_LOCATION < X_UPPER_LEFT - 30;
}

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
        if (!mower_can_touch(*zombie))
            continue;
        if (can_zombie_trip_mower(*zombie))
        {
            active_mower(mowers, zombie->row);
        }
    }
    // mower action
    for (int i = 0; i < (int)mowers.size(); i++)
        if (mowers[i] != nullptr && mowers[i]->status == MOWER_ACTIVE)
        {
            // mower move (repvz: 3.33 px/cs lawn, 2.5 px/cs pool, x5/3 here)
            mowers[i]->x += level.row_has_water(mowers[i]->row) ? MOWER_POOL_DX : MOWER_LAWN_DX;

            // mower gone (repvz mPosX > WIDE_BOARD_WIDTH = 800; clone +220)
            if (mowers[i]->x >= 1020)
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
Apply mower hit the zombies. (make zombie die immediately --
repvz MowZombie -> MowDown is an instant trample, no hp roll)
*/
bool apply_mower_hitting_zombie(vector<Zombie *> &zombies, const int &z_ind,
                                Mower &mower, vector<ZombiePart> &zombie_parts)
{
    if (is_mower_hit_zombie(mower, *zombies[z_ind]))
    {
        // zombie die
        zombies[z_ind]->add_zombie_die(zombie_parts);
        // armor drop
        if (reanimz::ready())
        {
            if (zombies[z_ind]->type == CONE_TYPE)
                zombie_parts.push_back(ZombiePart(reanimz::DROP_CONE_IMG, 1,
                                                  zombies[z_ind]->row, zombies[z_ind]->x_location,
                                                  ZOMBIE_WIDTH, ZOMBIE_HEIGHT));
            if (zombies[z_ind]->type == BUCKET_TYPE)
                zombie_parts.push_back(ZombiePart(reanimz::DROP_BUCKET_IMG, 1,
                                                  zombies[z_ind]->row, zombies[z_ind]->x_location,
                                                  ZOMBIE_WIDTH, ZOMBIE_HEIGHT));
            if (zombies[z_ind]->type == DOOR_TYPE)
                zombie_parts.push_back(ZombiePart(reanimz::DROP_DOOR_IMG, 1,
                                                  zombies[z_ind]->row, zombies[z_ind]->x_location,
                                                  ZOMBIE_WIDTH, ZOMBIE_HEIGHT));
        }
        else
        {
            if (zombies[z_ind]->type == CONE_TYPE)
                zombie_parts.push_back(ZombiePart(CONE_DROP_DIRECTORY, HEAD_ZOMBIE_FRAME,
                                                  zombies[z_ind]->row, zombies[z_ind]->x_location + 80,
                                                  HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT));
            if (zombies[z_ind]->type == BUCKET_TYPE)
                zombie_parts.push_back(ZombiePart(BUCKET_DROP_DIRECTORY, HEAD_ZOMBIE_FRAME,
                                                  zombies[z_ind]->row, zombies[z_ind]->x_location + 80,
                                                  HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT));
        }
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
    if (!mower_can_touch(zombie))
        return false;
    if (zombie.row == mower.row &&
        zombie.x_location <= mower.x)
        return true;
    return false;
}
