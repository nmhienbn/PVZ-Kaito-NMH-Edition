#pragma once
#include <SDL.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <string>
using namespace std;

bool init_music();
void play_music(int num_path, int repeat_times = -1);
void play_sound_effect(int num_path);
void close_music();

#define GROAN_MUSIC_DIRECTORY rand(GROAN1_MUSIC_DIRECTORY, GROAN5_MUSIC_DIRECTORY)
#define ZOMBIE_EATING_MUSIC_DIRECTORY rand(ZOMBIE_EATING1_MUSIC_DIRECTORY, ZOMBIE_EATING2_MUSIC_DIRECTORY)
#define SHIELD_HIT_MUSIC_DIRECTORY rand(SHIELD_HIT1_MUSIC_DIRECTORY, SHIELD_HIT2_MUSIC_DIRECTORY)
#define PEA_CRASH_MUSIC_DIRECTORY rand(PEA_CRASH_1_MUSIC_DIRECTORY, PEA_CRASH_3_MUSIC_DIRECTORY)
#define PLANT_PLANT_MUSIC_DIRECTORY rand(PLANT_PLANT_1_MUSIC_DIRECTORY, PLANT_PLANT_2_MUSIC_DIRECTORY)
#define ZOMBIE_FALLING_MUSIC_DIRECTORY rand(ZOMBIE_FALLING_1_MUSIC_DIRECTORY, ZOMBIE_FALLING_2_MUSIC_DIRECTORY)

enum music_type
{
    BACKGROUND_MUSIC_DIRECTORY,
    OPENING_MUSIC_DIRECTORY,
    BUTTON_CLICK_MUSIC_DIRECTORY,
    EVIL_LAUGH_MUSIC_DIRECTORY,
    R_S_P_MUSIC_DIRECTORY,
    WIN_MUSIC_DIRECTORY,
    LOSE_MUSIC_DIRECTORY,
    SUN_CLICK_MUSIC_DIRECTORY,
    SHOVEL_MUSIC_DIRECTORY,
    SEED_LIFT_MUSIC_DIRECTORY,
    PLANT_PLANT_1_MUSIC_DIRECTORY,
    PLANT_PLANT_2_MUSIC_DIRECTORY,
    GROAN1_MUSIC_DIRECTORY,
    GROAN2_MUSIC_DIRECTORY,
    GROAN3_MUSIC_DIRECTORY,
    GROAN4_MUSIC_DIRECTORY,
    GROAN5_MUSIC_DIRECTORY,
    FIRE_PEA_MUSIC_DIRECTORY,
    PEA_CRASH_1_MUSIC_DIRECTORY,
    PEA_CRASH_2_MUSIC_DIRECTORY,
    PEA_CRASH_3_MUSIC_DIRECTORY,
    SNOW_PEA_SPARKLES_DIRECTORY,
    ZOMBIE_COMING_MUSIC_DIRECTORY,
    HUGE_WAVE_MUSIC_DIRECTORY,
    ZOMBIE_EATING1_MUSIC_DIRECTORY,
    ZOMBIE_EATING2_MUSIC_DIRECTORY,
    ZOMBIE_FALLING_1_MUSIC_DIRECTORY,
    ZOMBIE_FALLING_2_MUSIC_DIRECTORY,
    SHIELD_HIT1_MUSIC_DIRECTORY, // buckethead
    SHIELD_HIT2_MUSIC_DIRECTORY, // buckethead
    CHERRYBOMB_MUSIC_DIRECTORY,
    NEW_PLANT_MUSIC_DIRECTORY,
    PAUSE_MUSIC_DIRECTORY,
    BUZZER_MUSIC_DIRECTORY, // player have not enough sun

    COUNT_MUSIC_DIRECTORY
};

const string music_directory[] = {
    "./audio/Brainiac_Maniac.mp3",   // BACKGROUND_MUSIC_DIRECTORY
    "./audio/opening.mp3",           // OPENING_MUSIC_DIRECTORY
    "./audio/button_click.mp3",      // BUTTON_CLICK_MUSIC_DIRECTORY
    "./audio/evil_laugh.mp3",        // EVIL_LAUGH_MUSIC_DIRECTORY
    "./audio/readysetplant.mp3",     // R_S_P_MUSIC_DIRECTORY
    "./audio/victory.mp3",           // WIN_MUSIC_DIRECTORY
    "./audio/zombie_won.mp3",        // LOSE_MUSIC_DIRECTORY
    "./audio/sun_click.mp3",         // SUN_CLICK_MUSIC_DIRECTORY
    "./audio/shovel.mp3",            // SHOVEL_MUSIC_DIRECTORY
    "./audio/seedlift.mp3",          // SEED_LIFT_MUSIC_DIRECTORY
    "./audio/plant1.mp3",            // PLANT_PLANT_1_MUSIC_DIRECTORY
    "./audio/plant2.mp3",            // PLANT_PLANT_2_MUSIC_DIRECTORY
    "./audio/groan1.mp3",            // GROAN1_MUSIC_DIRECTORY
    "./audio/groan2.mp3",            // GROAN2_MUSIC_DIRECTORY
    "./audio/groan3.mp3",            // GROAN3_MUSIC_DIRECTORY
    "./audio/groan4.mp3",            // GROAN4_MUSIC_DIRECTORY
    "./audio/groan5.mp3",            // GROAN5_MUSIC_DIRECTORY
    "./audio/firepea.mp3",           // FIRE_PEA_MUSIC_DIRECTORY
    "./audio/splat1.mp3",            // PEA_CRASH_1_MUSIC_DIRECTORY
    "./audio/splat2.mp3",            // PEA_CRASH_2_MUSIC_DIRECTORY
    "./audio/splat3.mp3",            // PEA_CRASH_3_MUSIC_DIRECTORY
    "./audio/snow_pea_sparkles.mp3", // SNOW_PEA_SPARKLES_DIRECTORY
    "./audio/zombie_coming.mp3",     // ZOMBIE_COMING_MUSIC_DIRECTORY
    "./audio/hugewave.mp3",          // HUGE_WAVE_MUSIC_DIRECTORY
    "./audio/zombie_eating1.mp3",    // ZOMBIE_EATING1_MUSIC_DIRECTORY
    "./audio/zombie_eating2.mp3",    // ZOMBIE_EATING2_MUSIC_DIRECTORY
    "./audio/zombie_falling_1.mp3",  // ZOMBIE_EATING1_MUSIC_DIRECTORY
    "./audio/zombie_falling_2.mp3",  // ZOMBIE_EATING2_MUSIC_DIRECTORY
    "./audio/shieldhit1.mp3",        // SHIELD_HIT1_MUSIC_DIRECTORY
    "./audio/shieldhit2.mp3",        // SHIELD_HIT2_MUSIC_DIRECTORY
    "./audio/cherrybomb.mp3",        // CHERRYBOMB_MUSIC_DIRECTORY
    "./audio/new_plant.mp3",         // NEW_PLANT_MUSIC_DIRECTORY
    "./audio/pause.mp3",             // PAUSE_MUSIC_DIRECTORY
    "./audio/buzzer.mp3",            // PAUSE_MUSIC_DIRECTORY

};