#pragma once
#include "support.hpp"
#include <SDL.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <string>
using namespace std;

bool init_music();
void play_music(int num_path, int repeat_times = -1);
void play_sound_effect(int num_path);
void close_music();
void delete_all_sound_effect();
// Loading-screen warm-up: decode sound effects [begin, end) into the chunk
// cache now instead of on first playback.
void warm_sound_effects(int begin, int end);
// Loading-screen warm-up: pull a music file's bytes through the page cache
// (Mix_LoadMUS streams lazily at play time; this pays the disk I/O upfront).
void prefetch_music_file(int num_path);

#define GROAN_MUSIC_DIRECTORY rand(GROAN1_MUSIC_DIRECTORY, GROAN5_MUSIC_DIRECTORY)
#define ZOMBIE_EATING_MUSIC_DIRECTORY rand(ZOMBIE_EATING1_MUSIC_DIRECTORY, ZOMBIE_EATING2_MUSIC_DIRECTORY)
#define SHIELD_HIT_MUSIC_DIRECTORY rand(SHIELD_HIT1_MUSIC_DIRECTORY, SHIELD_HIT2_MUSIC_DIRECTORY)
#define PEA_CRASH_MUSIC_DIRECTORY rand(PEA_CRASH_1_MUSIC_DIRECTORY, PEA_CRASH_3_MUSIC_DIRECTORY)
#define PLANT_PLANT_MUSIC_DIRECTORY rand(PLANT_PLANT_1_MUSIC_DIRECTORY, PLANT_PLANT_2_MUSIC_DIRECTORY)
#define ZOMBIE_FALLING_MUSIC_DIRECTORY rand(ZOMBIE_FALLING_1_MUSIC_DIRECTORY, ZOMBIE_FALLING_2_MUSIC_DIRECTORY)
#define BOWLING_IMPACT_MUSIC_DIRECTORY rand(BOWLING_IMPACT_1_MUSIC_DIRECTORY, BOWLING_IMPACT_2_MUSIC_DIRECTORY)
#define NEWSPAPER_RARRGH_SND_MUSIC_DIRECTORY rand(NEWSPAPER_RARRGH_MUSIC_DIRECTORY, NEWSPAPER_RARRGH2_MUSIC_DIRECTORY)
#define CHOMP_MUSIC_DIRECTORY rand(CHOMP_MUSIC1_DIRECTORY, CHOMP_MUSIC2_DIRECTORY)
#define JACK_SURPRISE_SND_MUSIC_DIRECTORY rand(JACK_SURPRISE_MUSIC_DIRECTORY, JACK_SURPRISE2_MUSIC_DIRECTORY)
#define BUNGEE_SCREAM_SND_DIRECTORY rand(BUNGEE_SCREAM_MUSIC_DIRECTORY, BUNGEE_SCREAM3_MUSIC_DIRECTORY)
#define IMP_SND_DIRECTORY rand(IMP_MUSIC_DIRECTORY, IMP2_MUSIC_DIRECTORY)
#define SQUASH_HMM_SND_MUSIC_DIRECTORY rand(SQUASH_HMM_MUSIC_DIRECTORY, SQUASH_HMM2_MUSIC_DIRECTORY)
#define IGNITE_SND_MUSIC_DIRECTORY rand(IGNITE_MUSIC_DIRECTORY, IGNITE2_MUSIC_DIRECTORY)
// Roof-plant (area 5) shorthands.
#define THROW_SND_MUSIC_DIRECTORY rand(THROW_MUSIC_DIRECTORY, THROW2_MUSIC_DIRECTORY)
#define MELON_IMPACT_SND_MUSIC_DIRECTORY rand(MELON_IMPACT_MUSIC_DIRECTORY, MELON_IMPACT2_MUSIC_DIRECTORY)
#define KERNELPULT_SND_MUSIC_DIRECTORY rand(KERNELPULT_MUSIC_DIRECTORY, KERNELPULT2_MUSIC_DIRECTORY)
#define PLASTIC_HIT_SND_MUSIC_DIRECTORY rand(PLASTIC_HIT_MUSIC_DIRECTORY, PLASTIC_HIT2_MUSIC_DIRECTORY)

enum music_type
{
    NULL_MUSIC_DIRECTORY = -1,

    Brainiac_Maniac_DIRECTORY,
    Grasswalk_DIRECTORY,
    Loonboon_DIRECTORY,
    Moongrains_DIRECTORY,
    WateryGravesSlow_DIRECTORY,
    UltimateBattle_DIRECTORY,
    RigorMormist_DIRECTORY,
    Cerebrawl_DIRECTORY,
    GrazeTheRoof_DIRECTORY,
    OPENING_MUSIC_DIRECTORY,

    ChooseYourSeeds_DIRECTORY,
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
    FINAL_WAVE_MUSIC_DIRECTORY,
    SIREN_MUSIC_DIRECTORY,
    ZOMBIE_EATING1_MUSIC_DIRECTORY,
    ZOMBIE_EATING2_MUSIC_DIRECTORY,
    ZOMBIE_FALLING_1_MUSIC_DIRECTORY,
    ZOMBIE_FALLING_2_MUSIC_DIRECTORY,
    SHIELD_HIT1_MUSIC_DIRECTORY, // buckethead
    SHIELD_HIT2_MUSIC_DIRECTORY, // buckethead
    CHERRYBOMB_MUSIC_DIRECTORY,
    POTATOMINE_MUSIC_DIRECTORY,
    NEW_PLANT_MUSIC_DIRECTORY,
    PAUSE_MUSIC_DIRECTORY,
    BUZZER_MUSIC_DIRECTORY, // player have not enough sun
    MOWER_MUSIC_DIRECTORY,
    LOADINGBAR_FLOWER_DIRECTORY,
    LOADINGBAR_ZOMBIE_DIRECTORY,
    BOWLING_MUSIC_DIRECTORY, // nut launch (rolling sound)
    BOWLING_IMPACT_1_MUSIC_DIRECTORY,
    BOWLING_IMPACT_2_MUSIC_DIRECTORY,
    POLEVAULT_MUSIC_DIRECTORY, // pole vault "boing"
    NEWSPAPER_RIP_MUSIC_DIRECTORY,
    NEWSPAPER_RARRGH_MUSIC_DIRECTORY,
    NEWSPAPER_RARRGH2_MUSIC_DIRECTORY,
    CHOMP_MUSIC1_DIRECTORY, // chomper bite variants
    CHOMP_MUSIC2_DIRECTORY,
    BIGCHOMP_MUSIC_DIRECTORY,   // chomper swallows a zombie whole
    CHOMPSOFT_MUSIC_DIRECTORY,  // chomper starts chewing
    PUFF_MUSIC_DIRECTORY,       // puff-shroom spits a spore
    FUME_MUSIC_DIRECTORY,       // fume-shroom fumes
    DOOM_MUSIC_DIRECTORY,       // doom-shroom blast
    ICE_MUSIC_DIRECTORY,        // ice-shroom freeze
    GRAVESTONE_RUMBLE_MUSIC_DIRECTORY, // zombies rise from the graves
    DIRT_RISE_MUSIC_DIRECTORY,         // whack-a-zombie riser claws out of a stone
    GRAVEBUSTER_MUSIC_DIRECTORY,       // grave buster chomps a grave
    PLANTGROW_MUSIC_DIRECTORY,         // sun-shroom grows up
    DANCER_MUSIC_DIRECTORY,            // dancing zombie summons the backups
    POOL_CLEANER_MUSIC_DIRECTORY,      // pool cleaner rolls along a water lane

    // Area 5 (roof) + Zomboss boss fight sound effects.
    POGO_ZOMBIE_MUSIC_DIRECTORY,         // pogo stick boing loop
    BUNGEE_SCREAM_MUSIC_DIRECTORY,       // bungee zombie yodel variants
    BUNGEE_SCREAM2_MUSIC_DIRECTORY,
    BUNGEE_SCREAM3_MUSIC_DIRECTORY,
    LADDER_ZOMBIE_MUSIC_DIRECTORY,       // ladder placed on a defensive plant
    GARGANTUAR_THUMP_MUSIC_DIRECTORY,    // gargantuar smashes a plant
    GARGANTUDEATH_MUSIC_DIRECTORY,       // gargantuar dies
    IMP_MUSIC_DIRECTORY,                 // imp thrown (variants)
    IMP2_MUSIC_DIRECTORY,
    BASKETBALL_MUSIC_DIRECTORY,          // catapult zombie lobs a basketball
    AWOOGA_MUSIC_DIRECTORY,              // zomboss boulder warning horn
    BOSS_EXPLOSION_MUSIC_DIRECTORY,      // zombot blows up
    BOSS_BOULDER_MUSIC_DIRECTORY,        // zomboss rolls a fire/ice boulder
    HYDRAULIC_MUSIC_DIRECTORY,           // zombot head lowers (vulnerable)
    HYDRAULIC_SHORT_MUSIC_DIRECTORY,
    RVTHROW_MUSIC_DIRECTORY,             // zomboss throws the RV
    ZOMBIES_ON_YOUR_LAWN_MUSIC_DIRECTORY,

    // Area 3 (pool) plant sound effects.
    JALAPENO_MUSIC_DIRECTORY,          // jalapeno burns the whole row
    SQUASH_HMM_MUSIC_DIRECTORY,        // squash spots a target
    ZOMBIE_SPLASH_MUSIC_DIRECTORY,     // tangle kelp drags a zombie under

    // Area 4 (fog) plant + balloon sound effects.
    BLOVER_MUSIC_DIRECTORY,        // blover wind blast
    PLANTERN_MUSIC_DIRECTORY,      // plantern lights up
    MAGNETSHROOM_MUSIC_DIRECTORY,  // magnet-shroom steals a metal item
    BALLOON_POP_MUSIC_DIRECTORY,   // a balloon zombie's balloon pops

    // Area 4 (fog) zombie + storm sound effects (zombie agent): the balloon
    // pop above belongs to the fog-plants agent; both use the same entry.
    JACKINTHEBOX_MUSIC_DIRECTORY,      // jack-in-the-box crank music loop
    JACK_SURPRISE_MUSIC_DIRECTORY,     // the jack pops out of the box
    JACK_SURPRISE2_MUSIC_DIRECTORY,
    BALLOON_INFLATE_MUSIC_DIRECTORY,   // balloon zombie floats in
    DIGGER_MUSIC_DIRECTORY,            // digger digs in / breaks the surface
    RAIN_MUSIC_DIRECTORY,              // storm level rain loop
    THUNDER_MUSIC_DIRECTORY,           // storm level lightning crack

    // Area 3 (pool) zombie sound effects.
    ZOMBIE_ENTERING_WATER_MUSIC_DIRECTORY,   // ducky tube / snorkel / dolphin hit the pool
    ZAMBONI_MUSIC_DIRECTORY,                 // zomboni engine hum (replayed as a loop)
    DOLPHIN_APPEARS_MUSIC_DIRECTORY,         // dolphin rider warning call
    DOLPHIN_BEFORE_JUMPING_MUSIC_DIRECTORY,  // dolphin rider clicks before the jump

    // Area 5 (roof) plant sound effects (plants agent).
    THROW_MUSIC_DIRECTORY,            // cabbage/kernel-pult launches its lob
    THROW2_MUSIC_DIRECTORY,
    KERNELPULT_MUSIC_DIRECTORY,       // kernel-pult corn launch variants
    KERNELPULT2_MUSIC_DIRECTORY,
    BUTTER_MUSIC_DIRECTORY,           // butter splats down on a zombie's head
    MELON_IMPACT_MUSIC_DIRECTORY,     // melon splash impact variants
    MELON_IMPACT2_MUSIC_DIRECTORY,
    COFFEE_MUSIC_DIRECTORY,           // coffee bean planted on a mushroom
    WAKEUP_MUSIC_DIRECTORY,           // mushroom wakes up

    // Extra variants / effects whose files exist in resources/audio.
    SQUASH_HMM2_MUSIC_DIRECTORY,   // squash spots a target (variant 2)
    GULP_MUSIC_DIRECTORY,          // chomper gulps after the swallow
    FLOOP_MUSIC_DIRECTORY,         // digging / shovel pickup "floop"
    IGNITE_MUSIC_DIRECTORY,        // torchwood ignites a pea (variants)
    IGNITE2_MUSIC_DIRECTORY,

    // Whack-a-Zombie (2-5) mallet sounds (original FOLEY_SWING / FOLEY_BONK /
    // FOLEY_PLASTIC_HIT; bucket helm reuses SHIELD_HIT above).
    SWING_MUSIC_DIRECTORY,         // mallet whoosh on every whack click
    BONK_MUSIC_DIRECTORY,          // mallet lands on a bare zombie
    PLASTIC_HIT_MUSIC_DIRECTORY,   // mallet clangs off a cone helm (variants)
    PLASTIC_HIT2_MUSIC_DIRECTORY,

    // Money pickups + zombie vocal (2Pak retail audio).
    COIN_MUSIC_DIRECTORY,          // coin/diamond pickup chime
    SCREAM_MUSIC_DIRECTORY,        // zombie scream

    // Selector screen (2Pak retail audio): tombstone slide on the ADVENTURE
    // click; pairs with EVIL_LAUGH_MUSIC_DIRECTORY for the zombie-hand rise.
    GRAVEBUTTON_MUSIC_DIRECTORY,

    COUNT_MUSIC_DIRECTORY
};

const string music_directory[] = {
    "./resources/audio/Brainiac_Maniac.mp3",  // Brainiac_Maniac_DIRECTORY
    "./resources/audio/Grasswalk.mp3",        // Grasswalk_DIRECTORY
    "./resources/audio/Loonboon.mp3",         // Loonboon_DIRECTORY
    "./resources/audio/Moongrains.mp3",       // Moongrains_DIRECTORY
    "./resources/audio/WateryGravesSlow.mp3", // WateryGravesSlow_DIRECTORY
    "./resources/audio/UltimateBattle.mp3",   // UltimateBattle_DIRECTORY
    "./resources/audio/RigorMormist.mp3",     // RigorMormist_DIRECTORY
    "./resources/audio/Cerebrawl.mp3",        // Cerebrawl_DIRECTORY
    "./resources/audio/GrazeTheRoof.mp3",     // GrazeTheRoof_DIRECTORY
    "./resources/audio/opening.mp3",          // OPENING_MUSIC_DIRECTORY

    "./resources/audio/ChooseYourSeeds.mp3",   // ChooseYourSeeds_DIRECTORY
    "./resources/audio/button_click.mp3",      // BUTTON_CLICK_MUSIC_DIRECTORY
    "./resources/audio/evil_laugh.mp3",        // EVIL_LAUGH_MUSIC_DIRECTORY
    "./resources/audio/readysetplant.mp3",     // R_S_P_MUSIC_DIRECTORY
    "./resources/audio/victory.mp3",           // WIN_MUSIC_DIRECTORY
    "./resources/audio/zombie_won.mp3",        // LOSE_MUSIC_DIRECTORY
    "./resources/audio/sun_click.mp3",         // SUN_CLICK_MUSIC_DIRECTORY
    "./resources/audio/shovel.mp3",            // SHOVEL_MUSIC_DIRECTORY
    "./resources/audio/seedlift.mp3",          // SEED_LIFT_MUSIC_DIRECTORY
    "./resources/audio/plant1.mp3",            // PLANT_PLANT_1_MUSIC_DIRECTORY
    "./resources/audio/plant2.mp3",            // PLANT_PLANT_2_MUSIC_DIRECTORY
    "./resources/audio/groan1.mp3",            // GROAN1_MUSIC_DIRECTORY
    "./resources/audio/groan2.mp3",            // GROAN2_MUSIC_DIRECTORY
    "./resources/audio/groan3.mp3",            // GROAN3_MUSIC_DIRECTORY
    "./resources/audio/groan4.mp3",            // GROAN4_MUSIC_DIRECTORY
    "./resources/audio/groan5.mp3",            // GROAN5_MUSIC_DIRECTORY
    "./resources/audio/firepea.mp3",           // FIRE_PEA_MUSIC_DIRECTORY
    "./resources/audio/splat1.mp3",            // PEA_CRASH_1_MUSIC_DIRECTORY
    "./resources/audio/splat2.mp3",            // PEA_CRASH_2_MUSIC_DIRECTORY
    "./resources/audio/splat3.mp3",            // PEA_CRASH_3_MUSIC_DIRECTORY
    "./resources/audio/snow_pea_sparkles.mp3", // SNOW_PEA_SPARKLES_DIRECTORY
    "./resources/audio/zombie_coming.mp3",     // ZOMBIE_COMING_MUSIC_DIRECTORY
    "./resources/audio/hugewave.mp3",          // HUGE_WAVE_MUSIC_DIRECTORY
    "./resources/audio/finalwave.mp3",         // FINAL_WAVE_MUSIC_DIRECTORY
    "./resources/audio/siren.mp3",             // SIREN_MUSIC_DIRECTORY
    "./resources/audio/zombie_eating1.mp3",    // ZOMBIE_EATING1_MUSIC_DIRECTORY
    "./resources/audio/zombie_eating2.mp3",    // ZOMBIE_EATING2_MUSIC_DIRECTORY
    "./resources/audio/zombie_falling_1.mp3",  // ZOMBIE_FALLING_1_MUSIC_DIRECTORY
    "./resources/audio/zombie_falling_2.mp3",  // ZOMBIE_FALLING_2_MUSIC_DIRECTORY
    "./resources/audio/shieldhit1.mp3",        // SHIELD_HIT1_MUSIC_DIRECTORY
    "./resources/audio/shieldhit2.mp3",        // SHIELD_HIT2_MUSIC_DIRECTORY
    "./resources/audio/cherrybomb.mp3",        // CHERRYBOMB_MUSIC_DIRECTORY
    "./resources/audio/potato_mine.mp3",       // POTATOMINE_MUSIC_DIRECTORY
    "./resources/audio/new_plant.mp3",         // NEW_PLANT_MUSIC_DIRECTORY
    "./resources/audio/pause.mp3",             // PAUSE_MUSIC_DIRECTORY
    "./resources/audio/buzzer.mp3",            // BUZZER_MUSIC_DIRECTORY
    "./resources/audio/lawnmower.mp3",         // MOWER_MUSIC_DIRECTORY
    "./resources/audio/loadingbar_flower.mp3", // LOADINGBAR_FLOWER_DIRECTORY
    "./resources/audio/loadingbar_zombie.mp3", // LOADINGBAR_ZOMBIE_DIRECTORY
    "./resources/audio/bowling.ogg",           // BOWLING_MUSIC_DIRECTORY
    "./resources/audio/bowlingimpact.ogg",     // BOWLING_IMPACT_1_MUSIC_DIRECTORY
    "./resources/audio/bowlingimpact2.ogg",     // BOWLING_IMPACT_2_MUSIC_DIRECTORY
    "./resources/audio/polevault.ogg",          // POLEVAULT_MUSIC_DIRECTORY
    "./resources/audio/newspaper_rip.ogg",      // NEWSPAPER_RIP_MUSIC_DIRECTORY
    "./resources/audio/newspaper_rarrgh.ogg",   // NEWSPAPER_RARRGH_MUSIC_DIRECTORY
    "./resources/audio/newspaper_rarrgh2.ogg",  // NEWSPAPER_RARRGH2_MUSIC_DIRECTORY
    "./resources/audio/chomp.ogg",              // CHOMP_MUSIC1_DIRECTORY
    "./resources/audio/chomp2.ogg",             // CHOMP_MUSIC2_DIRECTORY
    "./resources/audio/bigchomp.ogg",           // BIGCHOMP_MUSIC_DIRECTORY
    "./resources/audio/chompsoft.ogg",          // CHOMPSOFT_MUSIC_DIRECTORY
    "./resources/audio/puff.ogg"                // PUFF_MUSIC_DIRECTORY
    ,
    "./resources/audio/fume.ogg",               // FUME_MUSIC_DIRECTORY
    "./resources/audio/doomshroom.ogg",         // DOOM_MUSIC_DIRECTORY
    "./resources/audio/frozen.ogg",             // ICE_MUSIC_DIRECTORY
    "./resources/audio/gravestone_rumble.ogg",  // GRAVESTONE_RUMBLE_MUSIC_DIRECTORY
    "./resources/audio/dirt_rise.ogg",          // DIRT_RISE_MUSIC_DIRECTORY
    "./resources/audio/gravebusterchomp.ogg",   // GRAVEBUSTER_MUSIC_DIRECTORY
    "./resources/audio/plantgrow.ogg",          // PLANTGROW_MUSIC_DIRECTORY
    "./resources/audio/dancer.ogg"              // DANCER_MUSIC_DIRECTORY
    ,
    "./resources/audio/pool_cleaner.ogg"        // POOL_CLEANER_MUSIC_DIRECTORY
    ,
    // Area 5 (roof) + Zomboss boss fight sound effects.
    "./resources/audio/pogo_zombie.ogg",        // POGO_ZOMBIE_MUSIC_DIRECTORY
    "./resources/audio/bungee_scream.ogg",      // BUNGEE_SCREAM_MUSIC_DIRECTORY
    "./resources/audio/bungee_scream2.ogg",     // BUNGEE_SCREAM2_MUSIC_DIRECTORY
    "./resources/audio/bungee_scream3.ogg",     // BUNGEE_SCREAM3_MUSIC_DIRECTORY
    "./resources/audio/ladder_zombie.ogg",      // LADDER_ZOMBIE_MUSIC_DIRECTORY
    "./resources/audio/gargantuar_thump.ogg",   // GARGANTUAR_THUMP_MUSIC_DIRECTORY
    "./resources/audio/gargantudeath.ogg",      // GARGANTUDEATH_MUSIC_DIRECTORY
    "./resources/audio/imp.ogg",                // IMP_MUSIC_DIRECTORY
    "./resources/audio/imp2.ogg",               // IMP2_MUSIC_DIRECTORY
    "./resources/audio/basketball.ogg",         // BASKETBALL_MUSIC_DIRECTORY
    "./resources/audio/awooga.ogg",             // AWOOGA_MUSIC_DIRECTORY
    "./resources/audio/bossexplosion.ogg",      // BOSS_EXPLOSION_MUSIC_DIRECTORY
    "./resources/audio/bossboulderattack.ogg",  // BOSS_BOULDER_MUSIC_DIRECTORY
    "./resources/audio/hydraulic.ogg",          // HYDRAULIC_MUSIC_DIRECTORY
    "./resources/audio/hydraulic_short.ogg",    // HYDRAULIC_SHORT_MUSIC_DIRECTORY
    "./resources/audio/RVthrow.ogg",            // RVTHROW_MUSIC_DIRECTORY
    "./resources/audio/ZombiesOnYourLawn.ogg"   // ZOMBIES_ON_YOUR_LAWN_MUSIC_DIRECTORY
    ,
    "./resources/audio/jalapeno.ogg",           // JALAPENO_MUSIC_DIRECTORY
    "./resources/audio/squash_hmm.ogg",         // SQUASH_HMM_MUSIC_DIRECTORY
    "./resources/audio/zombiesplash.ogg",       // ZOMBIE_SPLASH_MUSIC_DIRECTORY
    "./resources/audio/blover.ogg",             // BLOVER_MUSIC_DIRECTORY
    "./resources/audio/plantern.ogg",           // PLANTERN_MUSIC_DIRECTORY
    "./resources/audio/magnetshroom.ogg",       // MAGNETSHROOM_MUSIC_DIRECTORY
    "./resources/audio/balloon_pop.ogg"         // BALLOON_POP_MUSIC_DIRECTORY
    ,
    "./resources/audio/jackinthebox.ogg",        // JACKINTHEBOX_MUSIC_DIRECTORY
    "./resources/audio/jack_surprise.ogg",       // JACK_SURPRISE_MUSIC_DIRECTORY
    "./resources/audio/jack_surprise2.ogg",      // JACK_SURPRISE2_MUSIC_DIRECTORY
    "./resources/audio/ballooninflate.ogg",      // BALLOON_INFLATE_MUSIC_DIRECTORY
    "./resources/audio/digger_zombie.ogg",       // DIGGER_MUSIC_DIRECTORY
    "./resources/audio/rain.ogg",                // RAIN_MUSIC_DIRECTORY
    "./resources/audio/thunder.ogg"              // THUNDER_MUSIC_DIRECTORY
    ,
    "./resources/audio/zombie_entering_water.ogg",   // ZOMBIE_ENTERING_WATER_MUSIC_DIRECTORY
    "./resources/audio/zamboni.ogg",                 // ZAMBONI_MUSIC_DIRECTORY
    "./resources/audio/dolphin_appears.ogg",         // DOLPHIN_APPEARS_MUSIC_DIRECTORY
    "./resources/audio/dolphin_before_jumping.ogg"   // DOLPHIN_BEFORE_JUMPING_MUSIC_DIRECTORY
    ,
    // Area 5 (roof) plant sound effects.
    "./resources/audio/throw.ogg",          // THROW_MUSIC_DIRECTORY
    "./resources/audio/throw2.ogg",         // THROW2_MUSIC_DIRECTORY
    "./resources/audio/kernelpult.ogg",     // KERNELPULT_MUSIC_DIRECTORY
    "./resources/audio/kernelpult2.ogg",    // KERNELPULT2_MUSIC_DIRECTORY
    "./resources/audio/butter.ogg",         // BUTTER_MUSIC_DIRECTORY
    "./resources/audio/melonimpact.ogg",    // MELON_IMPACT_MUSIC_DIRECTORY
    "./resources/audio/melonimpact2.ogg",   // MELON_IMPACT2_MUSIC_DIRECTORY
    "./resources/audio/coffee.ogg",         // COFFEE_MUSIC_DIRECTORY
    "./resources/audio/wakeup.ogg"          // WAKEUP_MUSIC_DIRECTORY
    ,
    "./resources/audio/squash_hmm2.ogg",    // SQUASH_HMM2_MUSIC_DIRECTORY
    "./resources/audio/gulp.ogg",           // GULP_MUSIC_DIRECTORY
    "./resources/audio/floop.ogg",          // FLOOP_MUSIC_DIRECTORY
    "./resources/audio/ignite.ogg",         // IGNITE_MUSIC_DIRECTORY
    "./resources/audio/ignite2.ogg",        // IGNITE2_MUSIC_DIRECTORY

    "./resources/audio/swing.ogg",          // SWING_MUSIC_DIRECTORY
    "./resources/audio/bonk.ogg",           // BONK_MUSIC_DIRECTORY
    "./resources/audio/plastichit.ogg",     // PLASTIC_HIT_MUSIC_DIRECTORY
    "./resources/audio/plastichit2.ogg"     // PLASTIC_HIT2_MUSIC_DIRECTORY
    ,
    "./resources/audio/coin.ogg",           // COIN_MUSIC_DIRECTORY
    "./resources/audio/scream.ogg"          // SCREAM_MUSIC_DIRECTORY
    ,
    "./resources/audio/gravebutton.ogg"     // GRAVEBUTTON_MUSIC_DIRECTORY

};

// Indexed by level.level_num (1-based; index 0 is an unused padding slot).
// Mirrors PvZ1 GOTY: minigame-style levels (1-5 bowling, 2-5 whack, 3-5 big
// trouble) play Loonboon; conveyor finals (10/20/30/45) play UltimateBattle;
// vasebreaker 4-5 (35) plays Cerebrawl; the 5-10 boss plays Brainiac_Maniac.
const int BACKGROUND_MUSIC_DIR[] = {
    Grasswalk_DIRECTORY, // 0: unused (level_num is 1-based)
    // Day frontyard (1-1 .. 1-10)
    Grasswalk_DIRECTORY, Grasswalk_DIRECTORY, Grasswalk_DIRECTORY, Grasswalk_DIRECTORY,
    Loonboon_DIRECTORY /*1-5 bowling*/, Grasswalk_DIRECTORY, Grasswalk_DIRECTORY,
    Grasswalk_DIRECTORY, Grasswalk_DIRECTORY, UltimateBattle_DIRECTORY,
    // Night frontyard (2-1 .. 2-10)
    Moongrains_DIRECTORY, Moongrains_DIRECTORY, Moongrains_DIRECTORY, Moongrains_DIRECTORY,
    Loonboon_DIRECTORY /*2-5 whack*/, Moongrains_DIRECTORY, Moongrains_DIRECTORY,
    Moongrains_DIRECTORY, Moongrains_DIRECTORY, UltimateBattle_DIRECTORY,
    // Day pool (3-1 .. 3-10)
    WateryGravesSlow_DIRECTORY, WateryGravesSlow_DIRECTORY, WateryGravesSlow_DIRECTORY,
    WateryGravesSlow_DIRECTORY, Loonboon_DIRECTORY /*3-5 big trouble conveyor*/,
    WateryGravesSlow_DIRECTORY, WateryGravesSlow_DIRECTORY, WateryGravesSlow_DIRECTORY,
    WateryGravesSlow_DIRECTORY, UltimateBattle_DIRECTORY /*3-10 conveyor*/,
    // Night pool / fog (4-1 .. 4-10)
    RigorMormist_DIRECTORY, RigorMormist_DIRECTORY, RigorMormist_DIRECTORY,
    RigorMormist_DIRECTORY, Cerebrawl_DIRECTORY /*4-5 vasebreaker*/,
    RigorMormist_DIRECTORY, RigorMormist_DIRECTORY, RigorMormist_DIRECTORY,
    RigorMormist_DIRECTORY, RigorMormist_DIRECTORY /*4-10 storm*/,
    // Roof (5-1 .. 5-10)
    GrazeTheRoof_DIRECTORY, GrazeTheRoof_DIRECTORY, GrazeTheRoof_DIRECTORY,
    GrazeTheRoof_DIRECTORY, UltimateBattle_DIRECTORY /*5-5 conveyor*/,
    GrazeTheRoof_DIRECTORY, GrazeTheRoof_DIRECTORY, GrazeTheRoof_DIRECTORY,
    GrazeTheRoof_DIRECTORY, Brainiac_Maniac_DIRECTORY /*5-10 Zomboss*/

};