#include "polevault_zombie.hpp"
#include "../roof/roof_shared.hpp" // plant_index_at
#include "draw/rsdl.hpp"
#include "music/music.hpp"
#include "reanim/reanim_zombie.hpp"

// Vault length: 55 ticks (~0.9 s) -- in line with the original jump beat.
const int VAULT_TICKS = 55;
// Running speed: PHASE_POLEVAULTER_PRE_VAULT rolls 0.66-0.68 px/cs in the
// original (repvz PickRandomSpeed, Zombie.cpp:1201-1205; mid-band 0.67 here)
// -> 36.5 px/s under HEAD_SPEED_SCALE. After the vault the zombie walks at
// its spawn roll.
const float POLE_RUN_PX = CS_VEL(0.67f) * HEAD_SPEED_SCALE;
// Original body 500 = 25 clone pool: the arm pops at the original's 2/3-body
// mark (mBodyHealth < 2*mBodyMaxHealth/3, repvz Zombie.cpp UpdateDamageStates)
// = 16.67. Mirrors zombies.json "polevault": [16.5, 25].
const float POLE_ARM_HEALTH = 16.5f;

PoleVaultingZombie::~PoleVaultingZombie()
{
}

void PoleVaultingZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = *ZOMBIE_HEALTH_LIMIT[POLEVAULT_TYPE].rbegin();
    directory_num = POLEVAULT_RUN_DIRECTORY;
}

void PoleVaultingZombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - dir_height);
    state = WALKING;
    directory_num = POLEVAULT_WALK_DIRECTORY;
}

bool PoleVaultingZombie::on_reach_plant(const int &col)
{
    if (state == VAULTING)
        return true; // mid-air: the plant stays untouched until landing
    if (state != RUNNING)
        return false; // pole already lost -> eat like any zombie
    // A Tall-nut is too tall to vault: the run-up bonks against it and the
    // zombie keeps the pole but walks on at normal pace, chewing the wall
    // (repvz Zombie.cpp:1892-1903: FOLEY_BONK, then PHASE_POLEVAULTER_POST_VAULT).
    int p = plant_index_at(row, col);
    if (p >= 0 && game_characters.plants[p]->get_type() == TALLNUT_TYPE)
    {
        state = WALKING;
        directory_num = POLEVAULT_WALK_DIRECTORY;
        frame = 0;
        reanim_time = 0.0f;
        reanim_rate = 0.2f;
        play_sound_effect(BONK_MUSIC_DIRECTORY);
        return false;
    }
    state = VAULTING;
    vault_ticks = VAULT_TICKS;
    vault_start_x = x_location;
    vault_travel = (int)(reanimz::poleVaultTravelPx() + 0.5f);
    frame = 0;
    reanim_time = 0.0f;
    directory_num = POLEVAULT_VAULT_DIRECTORY;
    // Play the whole jump segment once over the vault window.
    float len = reanimz::segmentLength(POLEVAULT_VAULT_DIRECTORY);
    reanim_rate = len > 0 ? len / VAULT_TICKS : 0.2f;
    play_sound_effect(POLEVAULT_MUSIC_DIRECTORY);
    return true;
}

bool PoleVaultingZombie::on_move_tick()
{
    if (state != VAULTING)
        return false;
    vault_ticks--;
    // Flight distance is baked into anim_jump; slide the logical position at
    // a constant pace and land it exactly vault_travel past the trigger.
    x_location = vault_start_x - (VAULT_TICKS - vault_ticks) * vault_travel / VAULT_TICKS;
    if (vault_ticks <= 0)
    {
        x_location = vault_start_x - vault_travel;
        state = WALKING;
        directory_num = POLEVAULT_WALK_DIRECTORY;
        frame = 0;
        reanim_time = 0.0f;
        reanim_rate = 0.2f;
    }
    return true;
}

bool PoleVaultingZombie::can_eat_plant() const
{
    return state != VAULTING;
}

/*repvz: pre-vault the pole tip reaches ahead (mZombieAttackRect (-29, 0, 70),
Zombie.cpp:335), mid-vault GetZombieAttackRect swaps in (-40, 0, 100) (:8406),
post-vault it drops to (50, 0, 20) (:1797).*/
void PoleVaultingZombie::attack_x_span(int &left, int &width) const
{
    if (state == VAULTING)
        left = -40, width = 100;
    else if (state == RUNNING)
        left = -29, width = 70;
    else
        left = 50, width = 20;
}

float PoleVaultingZombie::speed_px_per_tick() const
{
    return state == RUNNING ? POLE_RUN_PX : Zombie::speed_px_per_tick();
}

/*Degrade at the 2/3-body mark (333/500 dmg): the outer arm drops, no segment
change (the pole skin hides the arm tracks through armor_dir).*/
void PoleVaultingZombie::determine_appearance(vector<ZombiePart> &zombie_parts)
{
    if (!armless && health <= POLE_ARM_HEALTH * health_scale)
    {
        armless = true;
        armor_dir = reanimz::POLE_ARMLESS_STATE;
        if (reanimz::ready())
            zombie_parts.push_back(ZombiePart(reanimz::DROP_ARM_IMG, 1, row, x_location,
                                              ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                              reanimz::SKIN_POLEVAULT));
        else
            zombie_parts.push_back(ZombiePart(ZOMBIE_ARM_DIRECTORY, 1, row, x_location,
                                              ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0));
    }
}

void PoleVaultingZombie::add_zombie_die(vector<ZombiePart> &zombie_parts)
{
    if (reanimz::ready())
    {
        zombie_parts.push_back(ZombiePart(reanimz::DEATH_PART_IMG, 1, row, x_location,
                                          ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                          reanimz::SKIN_POLEVAULT));
        return;
    }
    Zombie::add_zombie_die(zombie_parts);
}
