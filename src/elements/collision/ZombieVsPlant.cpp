#include "ZombieVsPlant.hpp"
#include "draw/rsdl.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/plants/coffeebean/coffeebean.hpp"
#include "elements/plants/garlic/garlic.hpp"
#include "elements/plants/support_plant.hpp"
#include "elements/zombies/jackbox/jackbox_zombie.hpp"
#include "music/music.hpp"

extern Map cells;
extern Level level;

/*
Update time for all zombies' next bite.
*/
void update_zombie_next_bite(vector<Zombie *> &zombies)
{
    for (auto &zombie : zombies)
        if (zombie->bite_time)
            zombie->bite_time--;
}

/*
Hypno-shroom trance: the first (unfrozen) bite on an AWAKE hypno-shroom turns
the zombie around instead of hurting it; the shroom is eaten right away. A
hypno-shroom dozing in daylight (no coffee bean) is just food -- the bite
chews it like any plant (repvz Zombie.cpp:4836: the charm requires
!aPlant->mIsAsleep).
@return true when the plant got consumed by the trance.
*/
static bool try_hypno_charm(Zombie &zombie, vector<Plants *> &plants, int &p_ind)
{
    Plants *shroom = plants[p_ind];
    if (shroom->get_type() != HYPNOSHROOM_TYPE || zombie.is_charmed || zombie.freeze_time)
        return false;
    if (!level.is_night() && !shroom->coffee_awake)
        return false; // asleep in daylight: plain bite, no trance
    zombie.is_charmed = true;
    zombie.is_moving = true;
    zombie.charm_flash = CHARM_FLASH_TICKS; // purple one-shot swirl burst
    zombie.bite_time = BITE_CLK_COUNT;
    zombie.change_zombie_eating_status(); // back to walking frames
    delete_plant(plants, p_ind);
    return true;
}

/*
If the zombie reached plant_type, apply its bites on the plant_type:
    + bite++
    + if plant_type's health == 0: reset zombie moving and delete that plant_type
*/
bool apply_zombie_bite_on_plant(Zombie &zombie, vector<Plants *> &plants, int &p_ind)
{
    // D1: headless dying zombies cannot eat (repvz CheckIfPreyCaught's
    // !mHasHead early-out): they walk over the tile and bleed out instead.
    if (zombie.head_dropped)
        return false;
    if (has_zombie_reached_block(zombie, plants[p_ind]->get_row(), plants[p_ind]->get_col()))
    {
        // Stacked tile (lily pad + rider, see support_plant.hpp): the zombie
        // chews through the RIDER first; the support plant waits its turn.
        if (is_support_plant(plants[p_ind]->get_type()) &&
            cell_has_main(plants, plants[p_ind]->get_row(), plants[p_ind]->get_col()))
            return false;
        // Laddered tile: zombies climb over the plant instead of chewing it.
        if (cells[plants[p_ind]->get_row()][plants[p_ind]->get_col()].is_block_zombie &&
            !cells[plants[p_ind]->get_row()][plants[p_ind]->get_col()].has_ladder)
        {
            // Zomboni-class vehicle: flattens the plant on contact instead of
            // stopping to chew (it never parks; on_reach_plant suppresses the
            // default stop, so only this touch registered once the bite clock
            // comes around).
            if (zombie.crushes_plants())
            {
                play_sound_effect(ZOMBIE_EATING_MUSIC_DIRECTORY);
                zombie.bite_time = BITE_CLK_COUNT;
                plants[p_ind]->set_attacked_time(MAX_TIME_BLINK);
                while (!plants[p_ind]->is_died())
                    plants[p_ind]->decrease_health();
                zombie.is_moving = true; // crush does not even slow the hull
                delete_plant(plants, p_ind);
                return true;
            }
            if (try_hypno_charm(zombie, plants, p_ind))
                return true;
            if (!zombie.can_eat_plant())
                return false;
            play_sound_effect(ZOMBIE_EATING_MUSIC_DIRECTORY);

            zombie.bite_time = BITE_CLK_COUNT;
            if (zombie.cold_time)
                zombie.bite_time += BITE_CLK_COUNT;
            plants[p_ind]->set_attacked_time(MAX_TIME_BLINK);
            plants[p_ind]->decrease_health();
            // Garlic: the surviving bite arms the yucky beat on the biter
            // (repvz Zombie.cpp:4851-4858) -- it chews a moment, gags in
            // place, then diverts lanes; the bite that finishes the bulb does
            // not divert. The beat itself lives in garlic.cpp
            // (update_garlic_yucky, ticked from update_zombies_status).
            if (plants[p_ind]->get_type() == GARLIC_TYPE && !plants[p_ind]->is_died())
            {
                garlic_bite(zombie);
                return true;
            }
            if (plants[p_ind]->is_died())
            {
                zombie.is_moving = true;
                zombie.bite_time = BITE_CLK_COUNT;
                delete_plant(plants, p_ind);
            }
            return true;
        }
    }
    return false;
}

/* Note to optimize after
For all zombie and all plant_type: apply zombie's bite
*/
void handle_zombie_plant_encounter(vector<Zombie *> &zombies, vector<Plants *> &plants)
{
    for (auto &zombie : zombies)
    {
        if (zombie->is_charmed || zombie->freeze_time || zombie->butter_time ||
            zombie->head_dropped)
            continue; // charmed walk past plants, frozen/buttered/headless do nothing
        if (zombie->bite_time == 0)
        {
            // Pumpkin shell stack: the zombie chews the SHELL before the plant
            // inside it (main first, support last).
            bool ate = false;
            for (int i = 0; i < (int)plants.size(); i++)
                if (plants[i]->get_type() == PUMPKIN_TYPE &&
                    apply_zombie_bite_on_plant(*zombie, plants, i))
                {
                    ate = true;
                    break;
                }
            if (ate)
                continue;
            for (int i = 0; i < (int)plants.size(); i++)
                if (apply_zombie_bite_on_plant(*zombie, plants, i))
                {
                    break;
                }
        }
    }
}

/*
Hypnotized zombies brawl with the regular ones: when a charmed zombie bumps
into a row-mate, both halt and trade bites (2.5 damage per swing
[= 50 raw, repvz EatZombie 4 dmg per 4 cs], each on its own BITE_CLK --
100 raw dmg/s like the original). Kills are finalized right here so no stray
corpse lingers.
*/
void handle_charmed_zombie_bites(vector<Zombie *> &zombies, vector<ZombiePart> &zombie_parts)
{
    for (size_t ci = 0; ci < zombies.size(); ci++)
    {
        Zombie *c = zombies[ci];
        if (!c->is_charmed || c->freeze_time)
            continue;
        // walk off the right edge: leave the lawn for good
        if (c->x_location > WINDOW_WIDTH)
        {
            delete c;
            zombies.erase(zombies.begin() + ci);
            ci--;
            continue;
        }
        // D1: a headless charmed zombie cannot bite either (repvz
        // CheckIfPreyCaught bails on !mHasHead); it just shambles on.
        if (c->head_dropped)
        {
            c->is_moving = true;
            continue;
        }
        Zombie *target = nullptr;
        size_t ti = 0;
        for (size_t j = 0; j < zombies.size(); j++)
        {
            Zombie *t = zombies[j];
            if (t == c || t->is_charmed)
                continue;
            if (t->row != c->row)
                continue;
            if (abs(t->x_location - c->x_location) < 40)
            {
                target = t;
                ti = j;
                break;
            }
        }
        c->is_moving = (target == nullptr);
        if (!target)
            continue;
        // charmed zombie takes a swing (repvz EatZombie: 4 dmg / 4 cs ->
        // 2.5 clone hp per 0.5 s swing)
        if (c->bite_time == 0)
        {
            play_sound_effect(ZOMBIE_EATING_MUSIC_DIRECTORY);
            c->bite_time = BITE_CLK_COUNT;
            if (target->take_hit(2.5f, zombie_parts))
            {
                play_sound_effect(ZOMBIE_FALLING_MUSIC_DIRECTORY);
                target->add_zombie_die(zombie_parts);
                delete target;
                zombies.erase(zombies.begin() + ti);
                if (ti < ci)
                    ci--;
                c->is_moving = true;
                continue;
            }
        }
        // the victim bites the charmed one back
        Zombie *c2 = (ci < zombies.size()) ? zombies[ci] : nullptr;
        if (c2 && c2 == c && target->freeze_time == 0 && target->bite_time == 0)
        {
            target->bite_time = BITE_CLK_COUNT;
            if (c->take_hit(2.5f, zombie_parts))
            {
                play_sound_effect(ZOMBIE_FALLING_MUSIC_DIRECTORY);
                c->add_zombie_die(zombie_parts);
                delete c;
                zombies.erase(zombies.begin() + ci);
                ci--;
            }
        }
    }
}

/*
    + Update all zombies' moving status.
    + Make zombies' bite to plants and update the next time they can bite them.
    + Hypnotized zombies fight the horde.
*/
void update_zombies_status(vector<Zombie *> &zombies, vector<Plants *> &plants, vector<ZombiePart> &zombie_parts)
{
    // Jackbox blast victims queued during last frame's move pass are erased
    // here, outside any zombie-vector iteration.
    drain_jack_explosion_kills(zombies);

    /*D1: dying bodies bleed out every logic tick (repvz Zombie::Update's
    isDying branch, Zombie.cpp:4661-4688): a headless zombie, or a
    zomboni/catapult whose body fell under 200 original HP. The head-drop
    tick itself is skipped once (bleed_pending), so a normal 10-pea kill lands
    ~0.7 s after the head comes off. Death finalizes here, outside any other
    zombie iteration, with the headless corpse render.*/
    for (size_t i = 0; i < zombies.size();)
    {
        Zombie *z = zombies[i];
        if (!z->is_dying_body())
        {
            ++i;
            continue;
        }
        if (z->bleed_pending)
        {
            z->bleed_pending = false;
            ++i;
            continue;
        }
        z->bleed_health(z->headless_bleed_per_tick());
        if (z->get_health() <= 0.0f)
        {
            play_sound_effect(ZOMBIE_FALLING_MUSIC_DIRECTORY);
            z->add_zombie_die(zombie_parts);
            delete z;
            zombies.erase(zombies.begin() + i);
            continue;
        }
        ++i;
    }

    // Update all zombies' moving status
    update_moving_status_for_zombies(zombies);

    // Garlic gag beat: hold the gagging zombies, divert rows on the turn
    // (must run after the moving-status pass, before the biting pass).
    update_garlic_yucky(zombies);

    // Zombie bite plant
    handle_zombie_plant_encounter(zombies, plants);

    // Hypnotized zombies vs the horde
    handle_charmed_zombie_bites(zombies, zombie_parts);

    // Update next time for each zombie to bite plant
    update_zombie_next_bite(zombies);
}
