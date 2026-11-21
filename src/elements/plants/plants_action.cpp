#include "elements/Map/Map.hpp"
#include "elements/collision/AttackPlantVsZombie.hpp"
#include "elements/collision/InstancePlantVsZombie.hpp"
#include "elements/elements.hpp"
#include "plants.hpp"
#include "support.hpp"
#include "support_plant.hpp"

extern Elements game_characters;
extern Map cells;
extern Window win;

/*
Peashooter's action: fire pea
*/
void Peashooter::action()
{
    fire_pea(game_characters.bullets);
    bool check_zombie_in_attack_range = check_in_attack_range(*this, game_characters.zombies);
    determine_appearance(check_zombie_in_attack_range);
}

/*
Sunflower's action: gen sun
*/
void Sunflower::action()
{
    gen_sun_from_a_sunflower(game_characters.suns);
}

/*
Wall-nut's action: nothing
*/
void WallNut::action()
{
}

/*
Snow Pea's action: fire snow pea
*/
void SnowPea::action()
{
    fire_pea(game_characters.bullets);
    determine_appearance(check_in_attack_range(*this, game_characters.zombies));
}

/*
Repeater's action: fire two peas back to back
*/
void Repeater::action()
{
    fire_pea(game_characters.bullets);
    determine_appearance(check_in_attack_range(*this, game_characters.zombies));
}

/*
Puff-shroom's action: fire spore puffs (only at night; asleep by day)
*/
void PuffShroom::action()
{
    if (is_night())
    {
        fire_puff(game_characters.bullets);
        determine_appearance(check_in_attack_range(*this, game_characters.zombies));
    }
}

/*
Sun-shroom's action: grow over time, then make small (15) / regular (25) suns.
*/
void SunShroom::action()
{
    if (!is_night())
        return;
    grow_step();
    gen_sun(game_characters.suns);
}

/*
Fume-shroom's action: squirt a piercing spore cloud at night.
*/
void FumeShroom::action()
{
    if (is_night())
    {
        fire_fume(game_characters.bullets);
        determine_appearance(check_in_attack_range(*this, game_characters.zombies));
    }
}

/*
Grave Buster's action: defined in gravebuster.cpp (land, chew, clear grave).
*/

/*
Hypno-shroom's action: none; the trance hook fires when it is bitten.
*/

/*
Scaredy-shroom's action: full-row spores, but ducks when a zombie comes close.
*/
void ScaredyShroom::action()
{
    if (is_night())
    {
        bool scared = is_scared_now(game_characters.zombies);
        if (!scared && !is_down())
            fire_puff(game_characters.bullets);
        determine_appearance(check_in_attack_range(*this, game_characters.zombies), scared);
    }
}

/*
Ice-shroom's action: fuse, then freeze every zombie on the lawn for
ICESHROOM_FREEZE_MIN..MAX_TICKS (repvz RandRangeInt(400,600)cs -> 240..360
ticks) and melt away; the 20 s chill afterwards lands when the hold ends.
*/
void IceShroom::action()
{
    if (!is_night())
        return;
    if (!exploded())
    {
        if (fuse_step())
            for (int j = 0; j < (int)game_characters.zombies.size(); j++)
            {
                Zombie *z = game_characters.zombies[j];
                z->freeze_time = rand(ICESHROOM_FREEZE_MIN_TICKS, ICESHROOM_FREEZE_MAX_TICKS); // stand dead still, then stay chilled
                if (z->take_hit(1, game_characters.zombie_parts))
                {
                    z->add_zombie_die(game_characters.zombie_parts);
                    delete z;
                    game_characters.zombies.erase(game_characters.zombies.begin() + j);
                    j--;
                }
            }
    }
    else
        melt_step();
}

/*
Doom-shroom's action: blow a 3x5 crater into the lawn. The crater heals
after ~180 s (original GridItem counter 18000cs -> 10800 ticks at 60 tps).
*/
void DoomShroom::action()
{
    if (!is_night())
        return;
    if (fuse_blow())
    {
        apply_instance_plant_hit_zombie(*this, game_characters.zombies, game_characters.zombie_parts);
        cells[row][col].has_crater = true;
        cells[row][col].crater_clk = CRATER_HEAL_TICKS;
    }
    aftermath();
}

/*
Cherry Bomb's action: blow up zombies
*/
void CherryBomb::action()
{
    if (is_blow())
    {
        play_sound_effect(CHERRYBOMB_MUSIC_DIRECTORY);
        apply_instance_plant_hit_zombie(*this, game_characters.zombies, game_characters.zombie_parts);
    }
    disappear();
}

void PotatoMine::action()
{
    if (state == ARMED && check_in_attack_range(*this, game_characters.zombies))
    {
        // Explode instantly on contact -- the PvZ1 reanim has no attack
        // animation for Potato Mine (anim_mashed is empty in the data), so the
        // attack wind-up sheet is skipped and the blast happens right away.
        state = EXPLOSION;
        directory_num = POTATOMINE_EXPLOSION_DIRECTORY;
        frame = 0;
        play_sound_effect(POTATOMINE_MUSIC_DIRECTORY);
        apply_instance_plant_hit_zombie(*this, game_characters.zombies, game_characters.zombie_parts);
        // (old flow: state = ATTACK with POTATOMINE_ATTACK_DIRECTORY sheet,
        //  then is_blow() switched to EXPLOSION after the sheet finished)
    }
    disappear();
}

/*
Delete idx-th plant in vector<plants*>
A stacked tile (support plant + rider + pumpkin shell, see support_plant.hpp)
keeps its planted/blocked flags until the LAST plant on the tile is gone; the
block flag is then re-derived from the survivors (a flat spike-weed never
blocks, any other plant -- shell included -- does).
*/
void delete_plant(vector<Plants *> &plants, int &idx)
{
    int row = plants[idx]->get_row();
    int col = plants[idx]->get_col();
    cells[row][col].has_ladder = false; // ladder dies with its plant
    delete plants[idx];
    plants.erase(plants.begin() + idx);
    idx--;
    if (!cell_has_support(plants, row, col) && !cell_has_main(plants, row, col))
    {
        cells[row][col].is_planted = false;
        cells[row][col].is_block_zombie = false;
        return;
    }
    bool blocks = false;
    for (auto *p : plants)
        if (p->get_row() == row && p->get_col() == col && p->get_type() != SPIKEWEED_TYPE)
        {
            blocks = true;
            break;
        }
    cells[row][col].is_block_zombie = blocks;
}

/*
Move plants to next status.
Do all plants' actions.
*/
void update_plants_status(vector<Plants *> &plants)
{
    for (int i = 0; i < (int)game_characters.plants.size(); i++)
    {
        auto plant = game_characters.plants[i];
        plant->action();
        if (plant->is_died())
        {
            delete_plant(game_characters.plants, i);
        }
    }
}