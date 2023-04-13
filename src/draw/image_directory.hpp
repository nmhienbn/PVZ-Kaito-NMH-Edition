#pragma once
#include <string>
#include <map>
using namespace std;

#define ZOMBIE_SHEET_DIRECTORY rand(ZOMBIE_WALK1_DIRECTORY, ZOMBIE_WALK2_DIRECTORY)

/*
Contain all image used for SDL
*/
enum ImageType
{
    NULL_DIRECTORY = -1,

    PEASHOOTER_DIRECTORY,
    SUNFLOWER_DIRECTORY,
    WALNUT_DIRECTORY,
    SNOWPEA_DIRECTORY,
    CHERRYBOMB_DIRECTORY,

    PEASHOOTER_SHEET_DIRECTORY,
    PEASHOOTER_SHEET_BLINK_DIRECTORY,
    PEASHOOTER_ATTACK_DIRECTORY,
    PEASHOOTER_ATTACK_BLINK_DIRECTORY,

    SUNFLOWER_SHEET_DIRECTORY,
    SUNFLOWER_SHEET_BLINK_DIRECTORY,
    SUNFLOWER_HAPPY_DIRECTORY,
    SUNFLOWER_HAPPY_BLINK_DIRECTORY,

    WALNUT_1_DIRECTORY,
    WALNUT_2_DIRECTORY,
    WALNUT_3_DIRECTORY,
    WALNUT_4_DIRECTORY,
    WALNUT_1_BLINK_DIRECTORY,
    WALNUT_2_BLINK_DIRECTORY,
    WALNUT_3_BLINK_DIRECTORY,
    WALNUT_4_BLINK_DIRECTORY,

    SNOWPEA_SHEET_DIRECTORY,
    SNOWPEA_ATTACK_DIRECTORY,

    CHERRYBOMB_SHEET_DIRECTORY,
    CHERRYBOMB_SHEET_BLINK_DIRECTORY,

    PEASHOOTER_ICON_BRIGHT_DIRECTORY,
    PEASHOOTER_ICON_DIM_DIRECTORY,
    SUNFLOWER_ICON_BRIGHT_DIRECTORY,
    SUNFLOWER_ICON_DIM_DIRECTORY,
    WALNUT_ICON_BRIGHT_DIRECTORY,
    WALNUT_ICON_DIM_DIRECTORY,
    SNOWPEA_ICON_BRIGHT_DIRECTORY,
    SNOWPEA_ICON_DIM_DIRECTORY,
    CHERRYBOMB_ICON_BRIGHT_DIRECTORY,
    CHERRYBOMB_ICON_DIM_DIRECTORY,

    SUNFLOWER_TROPHY_DIRECTORY,
    NEW_PEASHOOTER_DIRECTORY,
    NEW_SUNFLOWER_DIRECTORY,
    NEW_WALNUT_DIRECTORY,
    NEW_SNOWPEA_DIRECTORY,
    NEW_CHERRYBOMB_DIRECTORY,

    PEA_DIRECTORY,
    PEA_EXPLODE_DIRECTORY,
    SNOWZ_PEA_DIRECTORY,
    SNOWZ_PEA_EXPLODE_DIRECTORY,

    STARTING_SCREEN_DIRECTORY,
    STARTING_SCREEN_CLICKED_DIRECTORY,
    CHOOSE_LEVELS_DIRECTORY,
    LEVEL_BLINK_DIRECTORY,
    BACKGROUND_LV1_DIRECTORY,
    BACKGROUND_LV2_DIRECTORY,
    BACKGROUND_DIRECTORY,
    BACKGROUND_NIGHT_DIRECTORY,
    LOSING_MESSAGE_DIRECTORY,
    WINNING_MESSAGE_DIRECTORY,

    SUN_DIRECTORY,
    SUN2_DIRECTORY,
    SUN_BAR_DIRECTORY,
    ICON_BAR_DIRECTORY,
    SHOVEL_BAR_DIRECTORY,
    SHOVEL_DIRECTORY,
    FLAG_METER,
    FLAG_METER_PROGRESS,
    FLAG_METER_PART,

    MENU_ICON_DIRECTORY,
    TURBO_ICON_DIRECTORY,
    TURBO_ICON_A_DIRECTORY,
    MENU_DIRECTORY,
    CONTINUE_DIRECTORY,
    RENAME_DIRECTORY,
    RESET_LEVEL_DIRECTORY,
    QUIT_DIRECTORY,
    OK_DIRECTORY,
    CANCEL_DIRECTORY,
    RENAME_MENU_DIRECTORY,
    NEW_USER_DIRECTORY,
    LEAVE_PROMPT_DIRECTORY,
    QUIT_PROMPT_DIRECTORY,
    RESET_PROMPT_DIRECTORY,
    RESTART_PROMPT_DIRECTORY,

    // NORMAL ZOMBIE---------------------------------------------------
    ZOMBIE_WALK1_DIRECTORY,
    ZOMBIE_WALK2_DIRECTORY,
    ZOMBIE_WALK3_DIRECTORY,

    ZOMBIE_WALK1_BLINK_DIRECTORY,
    ZOMBIE_WALK2_BLINK_DIRECTORY,
    ZOMBIE_WALK3_BLINK_DIRECTORY,

    ZOMBIE_WALK1_COLD_DIRECTORY,
    ZOMBIE_WALK2_COLD_DIRECTORY,
    ZOMBIE_WALK3_COLD_DIRECTORY,

    ZOMBIE_DIE_DIRECTORY,
    ZOMBIE_DIE_COLD_DIRECTORY,
    ZOMBIE_HEAD_DIRECTORY,
    ZOMBIE_HEAD_COLD_DIRECTORY,
    ZOMBIE_BURNT_DIRECTORY,

    ZOMBIE_EATING_DIRECTORY,
    ZOMBIE_EATING_BLINK_DIRECTORY,
    ZOMBIE_EATING_COLD_DIRECTORY,

    // FLAG ZOMBIE--------------------------------------------------------
    FLAG_ZOMBIE_WALK_DIRECTORY,
    FLAG_ZOMBIE_WALK_BLINK_DIRECTORY,
    FLAG_ZOMBIE_WALK_COLD_DIRECTORY,

    FLAG_ZOMBIE_EATING_DIRECTORY,
    FLAG_ZOMBIE_EATING_BLINK_DIRECTORY,
    FLAG_ZOMBIE_EATING_COLD_DIRECTORY,

    // CONEHEAD ZOMBIE--------------------------------------------------------
    CONE_DROP_DIRECTORY,

    CONE_ZOMBIE_WALK_1_DIRECTORY,
    CONE_ZOMBIE_WALK_2_DIRECTORY,
    CONE_ZOMBIE_WALK_3_DIRECTORY,

    CONE_ZOMBIE_WALK_1_BLINK_DIRECTORY,
    CONE_ZOMBIE_WALK_2_BLINK_DIRECTORY,
    CONE_ZOMBIE_WALK_3_BLINK_DIRECTORY,

    CONE_ZOMBIE_WALK_1_COLD_DIRECTORY,
    CONE_ZOMBIE_WALK_2_COLD_DIRECTORY,
    CONE_ZOMBIE_WALK_3_COLD_DIRECTORY,

    CONE_ZOMBIE_EATING_1_DIRECTORY,
    CONE_ZOMBIE_EATING_2_DIRECTORY,
    CONE_ZOMBIE_EATING_3_DIRECTORY,

    CONE_ZOMBIE_EATING_1_BLINK_DIRECTORY,
    CONE_ZOMBIE_EATING_2_BLINK_DIRECTORY,
    CONE_ZOMBIE_EATING_3_BLINK_DIRECTORY,

    CONE_ZOMBIE_EATING_1_COLD_DIRECTORY,
    CONE_ZOMBIE_EATING_2_COLD_DIRECTORY,
    CONE_ZOMBIE_EATING_3_COLD_DIRECTORY,

    // BUCKETHEAD ZOMBIE--------------------------------------------------------
    BUCKET_DROP_DIRECTORY,

    BUCKET_ZOMBIE_WALK_1_DIRECTORY,
    BUCKET_ZOMBIE_WALK_2_DIRECTORY,
    BUCKET_ZOMBIE_WALK_3_DIRECTORY,

    BUCKET_ZOMBIE_WALK_1_BLINK_DIRECTORY,
    BUCKET_ZOMBIE_WALK_2_BLINK_DIRECTORY,
    BUCKET_ZOMBIE_WALK_3_BLINK_DIRECTORY,

    BUCKET_ZOMBIE_WALK_1_COLD_DIRECTORY,
    BUCKET_ZOMBIE_WALK_2_COLD_DIRECTORY,
    BUCKET_ZOMBIE_WALK_3_COLD_DIRECTORY,

    BUCKET_ZOMBIE_EATING_1_DIRECTORY,
    BUCKET_ZOMBIE_EATING_2_DIRECTORY,
    BUCKET_ZOMBIE_EATING_3_DIRECTORY,

    BUCKET_ZOMBIE_EATING_1_BLINK_DIRECTORY,
    BUCKET_ZOMBIE_EATING_2_BLINK_DIRECTORY,
    BUCKET_ZOMBIE_EATING_3_BLINK_DIRECTORY,

    BUCKET_ZOMBIE_EATING_1_COLD_DIRECTORY,
    BUCKET_ZOMBIE_EATING_2_COLD_DIRECTORY,
    BUCKET_ZOMBIE_EATING_3_COLD_DIRECTORY,

    //-------------------------------------------------------
    START_READY_DIRECTORY,
    START_SET_DIRECTORY,
    START_PLANT_DIRECTORY,
    HUGE_WAVE_DIRECTORY,
    FINAL_WAVE_DIRECTORY,

    BLACK_SCREEN_DIRECTORY,
    WHITE_SCREEN_DIRECTORY,

    FRONTYARD_C1,
    FRONTYARD_C2,
    FRONTYARD_C3,
    FRONTYARD_C4,
    FRONTYARD_C5,
    FRONTYARD_C6,
    FRONTYARD_C7,
    FRONTYARD_C8,
    FRONTYARD_C9,
    FRONTYARD_R1,
    FRONTYARD_R2,
    FRONTYARD_R3,
    FRONTYARD_R4,
    FRONTYARD_R5,

    WIN_00_DIRECTORY,
    WIN_01_DIRECTORY,
    WIN_02_DIRECTORY,
    WIN_03_DIRECTORY,
    WIN_04_DIRECTORY,
    WIN_05_DIRECTORY,
    WIN_06_DIRECTORY,
    WIN_07_DIRECTORY,
    WIN_08_DIRECTORY,
    WIN_09_DIRECTORY,
    WIN_10_DIRECTORY,
    WIN_11_DIRECTORY,
    WIN_12_DIRECTORY,
    WIN_13_DIRECTORY,
    WIN_14_DIRECTORY,
    WIN_15_DIRECTORY,
    WIN_16_DIRECTORY,
    WIN_17_DIRECTORY,
    WIN_18_DIRECTORY,
    WIN_19_DIRECTORY,
    WIN_20_DIRECTORY,
    WIN_21_DIRECTORY,
    WIN_22_DIRECTORY,
    WIN_23_DIRECTORY,
    WIN_24_DIRECTORY,
    WIN_25_DIRECTORY,
    WIN_26_DIRECTORY,
    WIN_27_DIRECTORY,
    WIN_28_DIRECTORY,
    WIN_29_DIRECTORY,
    WIN_30_DIRECTORY,
    WIN_31_DIRECTORY,
    WIN_32_DIRECTORY,
    WIN_33_DIRECTORY,
    WIN_34_DIRECTORY,
    WIN_35_DIRECTORY,
    WIN_36_DIRECTORY,
    WIN_37_DIRECTORY,
    WIN_38_DIRECTORY,
    WIN_39_DIRECTORY,
    WIN_40_DIRECTORY,
    WIN_41_DIRECTORY,
    WIN_42_DIRECTORY,
    WIN_43_DIRECTORY,
    WIN_44_DIRECTORY,
    WIN_45_DIRECTORY,
    WIN_46_DIRECTORY,
    WIN_47_DIRECTORY,
    WIN_48_DIRECTORY,
    WIN_49_DIRECTORY,
    WIN_50_DIRECTORY,
    WIN_51_DIRECTORY,
    WIN_52_DIRECTORY,
    WIN_53_DIRECTORY,
    WIN_54_DIRECTORY,
    WIN_55_DIRECTORY,
    WIN_56_DIRECTORY,
    WIN_57_DIRECTORY,
    WIN_58_DIRECTORY,
    WIN_59_DIRECTORY,
    WIN_60_DIRECTORY,
    WIN_61_DIRECTORY,
    WIN_62_DIRECTORY,
    WIN_63_DIRECTORY,
    WIN_64_DIRECTORY,
    WIN_65_DIRECTORY,
    WIN_66_DIRECTORY,
    WIN_67_DIRECTORY,
    WIN_68_DIRECTORY,
    WIN_69_DIRECTORY,
    WIN_70_DIRECTORY,
    WIN_71_DIRECTORY,
    WIN_72_DIRECTORY,
    WIN_73_DIRECTORY,

    MOUSE_CURSOR_DIRECTORY,
    MOWER_DIRECTORY,

    COUNT_USED_DIRECTORY
};

/*
Contain all image directory corresponding
*/
const string image_directory[] = {

    "./image/Plant_sheet/peashooter/Peashooter.png", // PEASHOOTER_DIRECTORY
    "./image/Plant_sheet/sunflower/Sunflower.png",   // SUNFLOWER_DIRECTORY
    "./image/Plant_sheet/walnut/Walnut.png",         // WALNUT_DIRECTORY
    "./image/Plant_sheet/snowpea/SnowPea.png",       // SNOWPEA_DIRECTORY
    "./image/Plant_sheet/cherrybomb/Cherrybomb.png", // CHERRYBOMB_DIRECTORY

    // Plant sheet
    "./image/Plant_sheet/peashooter/peashooter_sheet.png",        // PEASHOOTER_SHEET_DIRECTORY
    "./image/Plant_sheet/peashooter/peashooter_sheet_blink.png",  // PEASHOOTER_SHEET_BLINK_DIRECTORY
    "./image/Plant_sheet/peashooter/peashooter_attack.png",       // PEASHOOTER_ATTACK_DIRECTORY
    "./image/Plant_sheet/peashooter/peashooter_attack_blink.png", // PEASHOOTER_ATTACK_BLINK_DIRECTORY

    "./image/Plant_sheet/sunflower/sunflower_sheet.png",       // SUNFLOWER_SHEET_DIRECTORY
    "./image/Plant_sheet/sunflower/sunflower_sheet_blink.png", // SUNFLOWER_SHEET_BLINK_DIRECTORY
    "./image/Plant_sheet/sunflower/sunflower_happy.png",       // SUNFLOWER_HAPPY_DIRECTORY
    "./image/Plant_sheet/sunflower/sunflower_happy_blink.png", // SUNFLOWER_HAPPY_BLINK_DIRECTORY

    "./image/Plant_sheet/walnut/Walnut-sheet-1.png",       // WALNUT_1_DIRECTORY
    "./image/Plant_sheet/walnut/Walnut-sheet-2.png",       // WALNUT_2_DIRECTORY
    "./image/Plant_sheet/walnut/Walnut-sheet-3.png",       // WALNUT_3_DIRECTORY
    "./image/Plant_sheet/walnut/Walnut-sheet-4.png",       // WALNUT_4_DIRECTORY
    "./image/Plant_sheet/walnut/Walnut-sheet-1_blink.png", // WALNUT_1_BLINK_DIRECTORY
    "./image/Plant_sheet/walnut/Walnut-sheet-2_blink.png", // WALNUT_2_BLINK_DIRECTORY
    "./image/Plant_sheet/walnut/Walnut-sheet-3_blink.png", // WALNUT_3_BLINK_DIRECTORY
    "./image/Plant_sheet/walnut/Walnut-sheet-4_blink.png", // WALNUT_4_BLINK_DIRECTORY

    "./image/Plant_sheet/snowpea/snowpea_sheet.png",  // SNOWPEA_SHEET_DIRECTORY
    "./image/Plant_sheet/snowpea/snowpea_attack.png", // SNOWPEA_ATTACK_DIRECTORY

    "./image/Plant_sheet/cherrybomb/cherrybomb_sheet.png",       // CHERRYBOMB_SHEET_DIRECTORY
    "./image/Plant_sheet/cherrybomb/cherrybomb_sheet_blink.png", // CHERRYBOMB_SHEET_BLINK_DIRECTORY

    "./image/Plant_seed/Peashooter_Seed_Packet.png",     // PEASHOOTER_ICON_BRIGHT_DIRECTORY
    "./image/Plant_seed/Peashooter_Seed_Packet_dim.png", // PEASHOOTER_ICON_DIM_DIRECTORY
    "./image/Plant_seed/Sunflower_Seed_Packet.png",      // SUNFLOWER_ICON_BRIGHT_DIRECTORY
    "./image/Plant_seed/Sunflower_Seed_Packet_dim.png",  // SUNFLOWER_ICON_DIM_DIRECTORY
    "./image/Plant_seed/Walnut_Seed_Packet.png",         // WALNUT_ICON_BRIGHT_DIRECTORY
    "./image/Plant_seed/Walnut_Seed_Packet_dim.png",     // WALNUT_ICON_DIM_DIRECTORY
    "./image/Plant_seed/Snowpea_Seed_Packet.png",        // SNOWPEA_ICON_BRIGHT_DIRECTORY
    "./image/Plant_seed/Snowpea_Seed_Packet_dim.png",    // SNOWPEA_ICON_DIM_DIRECTORY
    "./image/Plant_seed/Cherrybomb_Seed_Packet.png",     // CHERRYBOMB_ICON_BRIGHT_DIRECTORY
    "./image/Plant_seed/Cherrybomb_Seed_Packet_dim.png", // CHERRYBOMB_ICON_DIM_DIRECTORY

    "./image/New_plant/Golden_Sunflower_Trophy.png", // SUNFLOWER_TROPHY_DIRECTORY
    "./image/New_plant/new_peashooter.png",          // NEW_PEASHOOTER_DIRECTORY
    "./image/New_plant/new_sunflower.png",           // NEW_SUNFLOWER_DIRECTORY
    "./image/New_plant/new_walnut.png",              // NEW_WALNUT_DIRECTORY
    "./image/New_plant/new_snowpea.png",             // NEW_SNOWPEA_DIRECTORY
    "./image/New_plant/new_cherrybomb.png",          // NEW_CHERRYBOMB_DIRECTORY

    "./image/Bullets/Pea/pea.png",                     // PEA_DIRECTORY
    "./image/Bullets/Pea/pea_explode.png",             // PEA_EXPLODE_DIRECTORY
    "./image/Bullets/Snowz_pea/snowz_pea.png",         // SNOWZ_PEA_DIRECTORY
    "./image/Bullets/Snowz_pea/snowz_pea_explode.png", // SNOWZ_PEA_EXPLODE_DIRECTORY

    "./image/Background/Starting_Screen.png",         // STARTING_SCREEN_DIRECTORY
    "./image/Background/Starting_Screen_clicked.png", // STARTING_SCREEN_CLICKED_DIRECTORY
    "./image/Background/Choose_levels.png",           // CHOOSE_LEVELS_DIRECTORY
    "./image/Background/level_blink.png",             // LEVEL_BLINK_DIRECTORY
    "./image/Background/Frontyard_LV1.png",           // BACKGROUND_LV1_DIRECTORY
    "./image/Background/Frontyard_LV2.png",           // BACKGROUND_LV2_DIRECTORY
    "./image/Background/Frontyard.png",               // BACKGROUND_DIRECTORY
    "./image/Background/Frontyard_night.png",         // BACKGROUND_NIGHT_DIRECTORY
    "./image/Background/ZombiesWon.png",              // LOSING_MESSAGE_DIRECTORY
    "./image/Background/Winning_Pic.png",             // WINNING_MESSAGE_DIRECTORY

    "./image/Icons/sun.png",                    // SUN_DIRECTORY
    "./image/Icons/sun2.png",                   // SUN2_DIRECTORY
    "./image/Icons/sun_bar.png",                // SUN_BAR_DIRECTORY
    "./image/Icons/Item_Bar.png",               // ICON_BAR_DIRECTORY
    "./image/Icons/Shovel.png",                 // SHOVEL_BAR_DIRECTORY
    "./image/Icons/Shovel2.png",                // SHOVEL_DIRECTORY
    "./image/Icons/FlagMeter.png",              // FLAG_METER
    "./image/Icons/FlagMeterLevelProgress.png", // FLAG_METER_PROGRESS
    "./image/Icons/FlagMeterParts.png",         // FLAG_METER_PART

    "./image/Icons/menu_icon.png",              // MENU_ICON_DIRECTORY
    "./image/Icons/Turbo_Button.png",           // TURBO_ICON_DIRECTORY
    "./image/Icons/Turbo_Button_Activited.png", // TURBO_ICON_A_DIRECTORY
    "./image/Icons/menu.png",                   // MENU_DIRECTORY
    "./image/Icons/continue.png",               // CONTINUE_DIRECTORY
    "./image/Icons/rename.png",                 // RENAME_DIRECTORY
    "./image/Icons/reset_level.png",            // RESET_LEVEL_DIRECTORY
    "./image/Icons/quit.png",                   // QUIT_DIRECTORY
    "./image/Icons/ok.png",                     // OK_DIRECTORY
    "./image/Icons/cancel.png",                 // CANCEL_DIRECTORY
    "./image/Icons/rename_menu.png",            // RENAME_MENU_DIRECTORY
    "./image/Icons/new_user.png",               // NEW_USER_DIRECTORY
    "./image/Icons/leave_prompt.png",           // LEAVE_PROMPT_DIRECTORY
    "./image/Icons/quit_prompt.png",            // QUIT_PROMPT_DIRECTORY
    "./image/Icons/reset_prompt.png",           // RESET_PROMPT_DIRECTORY
    "./image/Icons/restart_prompt.png",         // RESTART_PROMPT_DIRECTORY

    // NORMAL ZOMBIE------------------------------------------------------------------------
    "./image/Zombie/Normal/ZombieWalk1.png", // ZOMBIE_WALK1_DIRECTORY
    "./image/Zombie/Normal/ZombieWalk2.png", // ZOMBIE_WALK2_DIRECTORY
    "./image/Zombie/Normal/ZombieWalk3.png", // ZOMBIE_WALK3_DIRECTORY

    "./image/Zombie/Normal/ZombieWalk1_blink.png", // ZOMBIE_WALK1_BLINK_DIRECTORY
    "./image/Zombie/Normal/ZombieWalk2_blink.png", // ZOMBIE_WALK2_BLINK_DIRECTORY
    "./image/Zombie/Normal/ZombieWalk3_blink.png", // ZOMBIE_WALK3_BLINK_DIRECTORY

    "./image/Zombie/Normal/ZombieWalk1_cold.png", // ZOMBIE_WALK1_COLD_DIRECTORY
    "./image/Zombie/Normal/ZombieWalk2_cold.png", // ZOMBIE_WALK2_COLD_DIRECTORY
    "./image/Zombie/Normal/ZombieWalk3_cold.png", // ZOMBIE_WALK3_COLD_DIRECTORY

    "./image/Zombie/Normal/Zombie_die.png",       // ZOMBIE_DIE_DIRECTORY
    "./image/Zombie/Normal/Zombie_die_cold.png",  // ZOMBIE_DIE_COLD_DIRECTORY
    "./image/Zombie/Normal/Zombie_head.png",      // ZOMBIE_HEAD_DIRECTORY
    "./image/Zombie/Normal/Zombie_head_cold.png", // ZOMBIE_HEAD_COLD_DIRECTORY
    "./image/Zombie/Zombie_burnt.png",            // ZOMBIE_BURNT_DIRECTORY

    "./image/Zombie/Normal/ZombieAttack.png",       // ZOMBIE_EATING_DIRECTORY
    "./image/Zombie/Normal/ZombieAttack_blink.png", // ZOMBIE_EATING_BLINK_DIRECTORY
    "./image/Zombie/Normal/ZombieAttack_cold.png",  // ZOMBIE_EATING_COLD_DIRECTORY

    // FLAG ZOMBIE--------------------------------------------------------
    "./image/Zombie/Flag/FlagWalk.png",       // FLAG_ZOMBIE_WALK_DIRECTORY
    "./image/Zombie/Flag/FlagWalk_blink.png", // FLAG_ZOMBIE_WALK_BLINK_DIRECTORY
    "./image/Zombie/Flag/FlagWalk_cold.png",  // FLAG_ZOMBIE_WALK_COLD_DIRECTORY

    "./image/Zombie/Flag/FlagAttack.png",       // FLAG_ZOMBIE_EATING_DIRECTORY
    "./image/Zombie/Flag/FlagAttack_blink.png", // FLAG_ZOMBIE_EATING_BLINK_DIRECTORY
    "./image/Zombie/Flag/FlagAttack_cold.png",  // FLAG_ZOMBIE_EATING_COLD_DIRECTORY

    // CONEHEAD ZOMBIE------------------------------------------------------------------------
    "./image/Zombie/Cone/cone_drop.png", // CONE_DROP_DIRECTORY

    "./image/Zombie/Cone/ConeZombieWalk_1.png", // CONE_ZOMBIE_WALK_1_DIRECTORY
    "./image/Zombie/Cone/ConeZombieWalk_2.png", // CONE_ZOMBIE_WALK_2_DIRECTORY
    "./image/Zombie/Cone/ConeZombieWalk_3.png", // CONE_ZOMBIE_WALK_3_DIRECTORY

    "./image/Zombie/Cone/ConeZombieWalk_1_blink.png", // CONE_ZOMBIE_WALK_1_BLINK_DIRECTORY
    "./image/Zombie/Cone/ConeZombieWalk_2_blink.png", // CONE_ZOMBIE_WALK_2_BLINK_DIRECTORY
    "./image/Zombie/Cone/ConeZombieWalk_3_blink.png", // CONE_ZOMBIE_WALK_3_BLINK_DIRECTORY

    "./image/Zombie/Cone/ConeZombieWalk_1_cold.png", // CONE_ZOMBIE_WALK_1_COLD_DIRECTORY
    "./image/Zombie/Cone/ConeZombieWalk_2_cold.png", // CONE_ZOMBIE_WALK_2_COLD_DIRECTORY
    "./image/Zombie/Cone/ConeZombieWalk_3_cold.png", // CONE_ZOMBIE_WALK_3_COLD_DIRECTORY

    "./image/Zombie/Cone/ConeZombieAttack_1.png", // CONE_ZOMBIE_EATING_1_DIRECTORY
    "./image/Zombie/Cone/ConeZombieAttack_2.png", // CONE_ZOMBIE_EATING_2_DIRECTORY
    "./image/Zombie/Cone/ConeZombieAttack_3.png", // CONE_ZOMBIE_EATING_3_DIRECTORY

    "./image/Zombie/Cone/ConeZombieAttack_1_blink.png", // CONE_ZOMBIE_EATING_1_BLINK_DIRECTORY
    "./image/Zombie/Cone/ConeZombieAttack_2_blink.png", // CONE_ZOMBIE_EATING_2_BLINK_DIRECTORY
    "./image/Zombie/Cone/ConeZombieAttack_3_blink.png", // CONE_ZOMBIE_EATING_3_BLINK_DIRECTORY

    "./image/Zombie/Cone/ConeZombieAttack_1_cold.png", // CONE_ZOMBIE_EATING_1_COLD_DIRECTORY
    "./image/Zombie/Cone/ConeZombieAttack_2_cold.png", // CONE_ZOMBIE_EATING_2_COLD_DIRECTORY
    "./image/Zombie/Cone/ConeZombieAttack_3_cold.png", // CONE_ZOMBIE_EATING_3_COLD_DIRECTORY

    // BUCKETHEAD ZOMBIE------------------------------------------------------------------------
    "./image/Zombie/Bucket/bucket_drop.png", // BUCKET_DROP_DIRECTORY

    "./image/Zombie/Bucket/BucketZombieWalk_1.png", // BUCKET_ZOMBIE_WALK_1_DIRECTORY
    "./image/Zombie/Bucket/BucketZombieWalk_2.png", // BUCKET_ZOMBIE_WALK_2_DIRECTORY
    "./image/Zombie/Bucket/BucketZombieWalk_3.png", // BUCKET_ZOMBIE_WALK_3_DIRECTORY

    "./image/Zombie/Bucket/BucketZombieWalk_1_blink.png", // BUCKET_ZOMBIE_WALK_1_BLINK_DIRECTORY
    "./image/Zombie/Bucket/BucketZombieWalk_2_blink.png", // BUCKET_ZOMBIE_WALK_2_BLINK_DIRECTORY
    "./image/Zombie/Bucket/BucketZombieWalk_3_blink.png", // BUCKET_ZOMBIE_WALK_3_BLINK_DIRECTORY

    "./image/Zombie/Bucket/BucketZombieWalk_1_cold.png", // BUCKET_ZOMBIE_WALK_1_COLD_DIRECTORY
    "./image/Zombie/Bucket/BucketZombieWalk_2_cold.png", // BUCKET_ZOMBIE_WALK_2_COLD_DIRECTORY
    "./image/Zombie/Bucket/BucketZombieWalk_3_cold.png", // BUCKET_ZOMBIE_WALK_3_COLD_DIRECTORY

    "./image/Zombie/Bucket/BucketZombieAttack_1.png", // BUCKET_ZOMBIE_EATING_1_DIRECTORY
    "./image/Zombie/Bucket/BucketZombieAttack_2.png", // BUCKET_ZOMBIE_EATING_2_DIRECTORY
    "./image/Zombie/Bucket/BucketZombieAttack_3.png", // BUCKET_ZOMBIE_EATING_3_DIRECTORY

    "./image/Zombie/Bucket/BucketZombieAttack_1_blink.png", // BUCKET_ZOMBIE_EATING_1_BLINK_DIRECTORY
    "./image/Zombie/Bucket/BucketZombieAttack_2_blink.png", // BUCKET_ZOMBIE_EATING_2_BLINK_DIRECTORY
    "./image/Zombie/Bucket/BucketZombieAttack_3_blink.png", // BUCKET_ZOMBIE_EATING_3_BLINK_DIRECTORY

    "./image/Zombie/Bucket/BucketZombieAttack_1_cold.png", // BUCKET_ZOMBIE_EATING_1_COLD_DIRECTORY
    "./image/Zombie/Bucket/BucketZombieAttack_2_cold.png", // BUCKET_ZOMBIE_EATING_2_COLD_DIRECTORY
    "./image/Zombie/Bucket/BucketZombieAttack_3_cold.png", // BUCKET_ZOMBIE_EATING_3_COLD_DIRECTORY

    //----------------------------------------------------------------------------------------------
    "./image/Announcement/StartReady.png", // START_READY_DIRECTORY
    "./image/Announcement/StartSet.png",   // START_SET_DIRECTORY
    "./image/Announcement/StartPlant.png", // START_PLANT_DIRECTORY
    "./image/Announcement/NextWave.png",   // HUGE_WAVE_DIRECTORY
    "./image/Announcement/FinalWave.png",  // FINAL_WAVE_DIRECTORY

    "./image/Black_Screen.png", // BLACK_SCREEN_DIRECTORY
    "./image/White_Screen.png", // WHITE_SCREEN_DIRECTORY

    "./image/Background/yard_blink/Frontyardc1.png", // FRONTYARD_C1
    "./image/Background/yard_blink/Frontyardc2.png", // FRONTYARD_C2
    "./image/Background/yard_blink/Frontyardc3.png", // FRONTYARD_C3
    "./image/Background/yard_blink/Frontyardc4.png", // FRONTYARD_C4
    "./image/Background/yard_blink/Frontyardc5.png", // FRONTYARD_C5
    "./image/Background/yard_blink/Frontyardc6.png", // FRONTYARD_C6
    "./image/Background/yard_blink/Frontyardc7.png", // FRONTYARD_C7
    "./image/Background/yard_blink/Frontyardc8.png", // FRONTYARD_C8
    "./image/Background/yard_blink/Frontyardc9.png", // FRONTYARD_C9
    "./image/Background/yard_blink/Frontyardr1.png", // FRONTYARD_R1
    "./image/Background/yard_blink/Frontyardr2.png", // FRONTYARD_R2
    "./image/Background/yard_blink/Frontyardr3.png", // FRONTYARD_R3
    "./image/Background/yard_blink/Frontyardr4.png", // FRONTYARD_R4
    "./image/Background/yard_blink/Frontyardr5.png", // FRONTYARD_R5

    "./image/Background/win/win_e_00.png", // WIN_00_DIRECTORY
    "./image/Background/win/win_e_01.png", // WIN_01_DIRECTORY
    "./image/Background/win/win_e_02.png", // WIN_02_DIRECTORY
    "./image/Background/win/win_e_03.png", // WIN_03_DIRECTORY
    "./image/Background/win/win_e_04.png", // WIN_04_DIRECTORY
    "./image/Background/win/win_e_05.png", // WIN_05_DIRECTORY
    "./image/Background/win/win_e_06.png", // WIN_06_DIRECTORY
    "./image/Background/win/win_e_07.png", // WIN_07_DIRECTORY
    "./image/Background/win/win_e_08.png", // WIN_08_DIRECTORY
    "./image/Background/win/win_e_09.png", // WIN_09_DIRECTORY
    "./image/Background/win/win_e_10.png", // WIN_10_DIRECTORY
    "./image/Background/win/win_e_11.png", // WIN_11_DIRECTORY
    "./image/Background/win/win_e_12.png", // WIN_12_DIRECTORY
    "./image/Background/win/win_e_13.png", // WIN_13_DIRECTORY
    "./image/Background/win/win_e_14.png", // WIN_14_DIRECTORY
    "./image/Background/win/win_e_15.png", // WIN_15_DIRECTORY
    "./image/Background/win/win_e_16.png", // WIN_16_DIRECTORY
    "./image/Background/win/win_e_17.png", // WIN_17_DIRECTORY
    "./image/Background/win/win_e_18.png", // WIN_18_DIRECTORY
    "./image/Background/win/win_e_19.png", // WIN_19_DIRECTORY
    "./image/Background/win/win_e_20.png", // WIN_20_DIRECTORY
    "./image/Background/win/win_e_21.png", // WIN_21_DIRECTORY
    "./image/Background/win/win_e_22.png", // WIN_22_DIRECTORY
    "./image/Background/win/win_e_23.png", // WIN_23_DIRECTORY
    "./image/Background/win/win_e_24.png", // WIN_24_DIRECTORY
    "./image/Background/win/win_e_25.png", // WIN_25_DIRECTORY
    "./image/Background/win/win_e_26.png", // WIN_26_DIRECTORY
    "./image/Background/win/win_e_27.png", // WIN_27_DIRECTORY
    "./image/Background/win/win_e_28.png", // WIN_28_DIRECTORY
    "./image/Background/win/win_e_29.png", // WIN_29_DIRECTORY
    "./image/Background/win/win_e_30.png", // WIN_30_DIRECTORY
    "./image/Background/win/win_e_31.png", // WIN_31_DIRECTORY
    "./image/Background/win/win_e_32.png", // WIN_32_DIRECTORY
    "./image/Background/win/win_e_33.png", // WIN_33_DIRECTORY
    "./image/Background/win/win_e_34.png", // WIN_34_DIRECTORY
    "./image/Background/win/win_e_35.png", // WIN_35_DIRECTORY
    "./image/Background/win/win_e_36.png", // WIN_36_DIRECTORY
    "./image/Background/win/win_e_37.png", // WIN_37_DIRECTORY
    "./image/Background/win/win_e_38.png", // WIN_38_DIRECTORY
    "./image/Background/win/win_e_39.png", // WIN_39_DIRECTORY
    "./image/Background/win/win_e_40.png", // WIN_40_DIRECTORY
    "./image/Background/win/win_e_41.png", // WIN_41_DIRECTORY
    "./image/Background/win/win_e_42.png", // WIN_42_DIRECTORY
    "./image/Background/win/win_e_43.png", // WIN_43_DIRECTORY
    "./image/Background/win/win_e_44.png", // WIN_44_DIRECTORY
    "./image/Background/win/win_e_45.png", // WIN_45_DIRECTORY
    "./image/Background/win/win_e_46.png", // WIN_46_DIRECTORY
    "./image/Background/win/win_e_47.png", // WIN_47_DIRECTORY
    "./image/Background/win/win_e_48.png", // WIN_48_DIRECTORY
    "./image/Background/win/win_e_49.png", // WIN_49_DIRECTORY
    "./image/Background/win/win_e_50.png", // WIN_50_DIRECTORY
    "./image/Background/win/win_e_51.png", // WIN_51_DIRECTORY
    "./image/Background/win/win_e_52.png", // WIN_52_DIRECTORY
    "./image/Background/win/win_e_53.png", // WIN_53_DIRECTORY
    "./image/Background/win/win_e_54.png", // WIN_54_DIRECTORY
    "./image/Background/win/win_e_55.png", // WIN_55_DIRECTORY
    "./image/Background/win/win_e_56.png", // WIN_56_DIRECTORY
    "./image/Background/win/win_e_57.png", // WIN_57_DIRECTORY
    "./image/Background/win/win_e_58.png", // WIN_58_DIRECTORY
    "./image/Background/win/win_e_59.png", // WIN_59_DIRECTORY
    "./image/Background/win/win_e_60.png", // WIN_60_DIRECTORY
    "./image/Background/win/win_e_61.png", // WIN_61_DIRECTORY
    "./image/Background/win/win_e_62.png", // WIN_62_DIRECTORY
    "./image/Background/win/win_e_63.png", // WIN_63_DIRECTORY
    "./image/Background/win/win_e_64.png", // WIN_64_DIRECTORY
    "./image/Background/win/win_e_65.png", // WIN_65_DIRECTORY
    "./image/Background/win/win_e_66.png", // WIN_66_DIRECTORY
    "./image/Background/win/win_e_67.png", // WIN_67_DIRECTORY
    "./image/Background/win/win_e_68.png", // WIN_68_DIRECTORY
    "./image/Background/win/win_e_69.png", // WIN_69_DIRECTORY
    "./image/Background/win/win_e_70.png", // WIN_70_DIRECTORY
    "./image/Background/win/win_e_71.png", // WIN_71_DIRECTORY
    "./image/Background/win/win_e_72.png", // WIN_72_DIRECTORY
    "./image/Background/win/win_e_73.png", // WIN_73_DIRECTORY

    "./image/Icons/starter_fire_pack.png", // MOUSE_CURSOR_DIRECTORY
    "./image/Icons/mower.png",             // MOWER_DIRECTORY

};

/*
The number of sheet in sprite of images
*/
const int N_SHEET[] = {
    0, // PEASHOOTER_DIRECTORY
    0, // SUNFLOWER_DIRECTORY
    0, // WALNUT_DIRECTORY
    0, // SNOWPEA_DIRECTORY
    0, // CHERRYBOMB_DIRECTORY

    // Plant sheet
    60, // PEASHOOTER_SHEET_DIRECTORY
    60, // PEASHOOTER_SHEET_BLINK_DIRECTORY
    60, // PEASHOOTER_ATTACK_DIRECTORY
    60, // PEASHOOTER_ATTACK_BLINK_DIRECTORY

    60, // SUNFLOWER_SHEET_DIRECTORY
    60, // SUNFLOWER_SHEET_BLINK_DIRECTORY
    15, // SUNFLOWER_HAPPY_DIRECTORY
    15, // SUNFLOWER_HAPPY_BLINK_DIRECTORY

    3, // WALNUT_1_DIRECTORY
    2, // WALNUT_2_DIRECTORY
    2, // WALNUT_3_DIRECTORY
    2, // WALNUT_4_DIRECTORY
    3, // WALNUT_1_BLINK_DIRECTORY
    2, // WALNUT_2_BLINK_DIRECTORY
    2, // WALNUT_3_BLINK_DIRECTORY
    2, // WALNUT_4_BLINK_DIRECTORY

    60, // SNOWPEA_SHEET_DIRECTORY
    60, // SNOWPEA_ATTACK_DIRECTORY

    24, // CHERRYBOMB_SHEET_DIRECTORY
    24, // CHERRYBOMB_SHEET_BLINK_DIRECTORY

    0, // PEASHOOTER_ICON_BRIGHT_DIRECTORY
    0, // PEASHOOTER_ICON_DIM_DIRECTORY
    0, // SUNFLOWER_ICON_BRIGHT_DIRECTORY
    0, // SUNFLOWER_ICON_DIM_DIRECTORY
    0, // WALNUT_ICON_BRIGHT_DIRECTORY
    0, // WALNUT_ICON_DIM_DIRECTORY
    0, // SNOWPEA_ICON_BRIGHT_DIRECTORY
    0, // SNOWPEA_ICON_DIM_DIRECTORY
    0, // CHERRYBOMB_ICON_BRIGHT_DIRECTORY
    0, // CHERRYBOMB_ICON_DIM_DIRECTORY

    0, // SUNFLOWER_TROPHY_DIRECTORY
    0, // NEW_PEASHOOTER_DIRECTORY
    0, // NEW_SUNFLOWER_DIRECTORY
    0, // NEW_WALNUT_DIRECTORY
    0, // NEW_SNOWPEA_DIRECTORY
    0, // NEW_CHERRYBOMB_DIRECTORY

    0, // PEA_DIRECTORY
    0, // PEA_EXPLODE_DIRECTORY
    0, // SNOWZ_PEA_DIRECTORY
    0, // SNOWZ_PEA_EXPLODE_DIRECTORY

    0, // STARTING_SCREEN_DIRECTORY
    0, // STARTING_SCREEN_CLICKED_DIRECTORY
    0, // CHOOSE_LEVELS_DIRECTORY
    0, // LEVEL_BLINK_DIRECTORY
    0, // BACKGROUND_LV1_DIRECTORY
    0, // BACKGROUND_LV2_DIRECTORY
    0, // BACKGROUND_DIRECTORY
    0, // BACKGROUND_NIGHT_DIRECTORY
    0, // LOSING_MESSAGE_DIRECTORY
    0, // WINNING_MESSAGE_DIRECTORY

    0, // SUN_DIRECTORY
    0, // SUN2_DIRECTORY
    0, // SUN_BAR_DIRECTORY
    0, // ICON_BAR_DIRECTORY
    0, // SHOVEL_BAR_DIRECTORY
    0, // SHOVEL_DIRECTORY
    0, // FLAG_METER
    0, // FLAG_METER_PROGRESS
    0, // FLAG_METER_PART

    0, // MENU_ICON_DIRECTORY
    0, // TURBO_ICON_DIRECTORY
    0, // TURBO_ICON_A_DIRECTORY
    0, // MENU_DIRECTORY
    0, // CONTINUE_DIRECTORY
    0, // RENAME_DIRECTORY
    0, // RESET_LEVEL_DIRECTORY
    0, // QUIT_DIRECTORY
    0, // OK_DIRECTORY
    0, // CANCEL_DIRECTORY
    0, // RENAME_MENU_DIRECTORY
    0, // NEW_USER_DIRECTORY
    0, // LEAVE_PROMPT_DIRECTORY
    0, // QUIT_PROMPT_DIRECTORY
    0, // RESET_PROMPT_DIRECTORY
    0, // RESTART_PROMPT_DIRECTORY

    // NORMAL ZOMBIE----------------------------------------------------
    22, // ZOMBIE_WALK1_DIRECTORY
    31, // ZOMBIE_WALK2_DIRECTORY
    50, // ZOMBIE_WALK3_DIRECTORY

    22, // ZOMBIE_WALK1_BLINK_DIRECTORY
    31, // ZOMBIE_WALK2_BLINK_DIRECTORY
    50, // ZOMBIE_WALK3_BLINK_DIRECTORY

    22, // ZOMBIE_WALK1_COLD_DIRECTORY
    31, // ZOMBIE_WALK2_COLD_DIRECTORY
    50, // ZOMBIE_WALK3_COLD_DIRECTORY

    10, // ZOMBIE_DIE_DIRECTORY
    10, // ZOMBIE_DIE_COLD_DIRECTORY
    12, // ZOMBIE_HEAD_DIRECTORY
    12, // ZOMBIE_HEAD_COLD_DIRECTORY
    20, // ZOMBIE_BURNT_DIRECTORY

    11, // ZOMBIE_EATING_DIRECTORY
    11, // ZOMBIE_EATING_BLINK_DIRECTORY
    11, // ZOMBIE_EATING_COLD_DIRECTORY

    // FLAG ZOMBIE--------------------------------------------------------
    12, // FLAG_ZOMBIE_WALK_DIRECTORY
    12, // FLAG_ZOMBIE_WALK_BLINK_DIRECTORY
    12, // FLAG_ZOMBIE_WALK_COLD_DIRECTORY

    11, // FLAG_ZOMBIE_EATING_DIRECTORY
    11, // FLAG_ZOMBIE_EATING_BLINK_DIRECTORY
    11, // FLAG_ZOMBIE_EATING_COLD_DIRECTORY

    // CONEHEAD ZOMBIE-------------------------------------------------
    12, // CONE_DROP_DIRECTORY

    21, // CONE_ZOMBIE_WALK_1_DIRECTORY
    21, // CONE_ZOMBIE_WALK_2_DIRECTORY
    21, // CONE_ZOMBIE_WALK_3_DIRECTORY

    21, // CONE_ZOMBIE_WALK_1_BLINK_DIRECTORY
    21, // CONE_ZOMBIE_WALK_2_BLINK_DIRECTORY
    21, // CONE_ZOMBIE_WALK_3_BLINK_DIRECTORY

    21, // CONE_ZOMBIE_WALK_1_COLD_DIRECTORY
    21, // CONE_ZOMBIE_WALK_2_COLD_DIRECTORY
    21, // CONE_ZOMBIE_WALK_3_COLD_DIRECTORY

    11, // CONE_ZOMBIE_EATING_1_DIRECTORY
    11, // CONE_ZOMBIE_EATING_2_DIRECTORY
    11, // CONE_ZOMBIE_EATING_3_DIRECTORY

    11, // CONE_ZOMBIE_EATING_1_BLINK_DIRECTORY
    11, // CONE_ZOMBIE_EATING_2_BLINK_DIRECTORY
    11, // CONE_ZOMBIE_EATING_3_BLINK_DIRECTORY

    11, // CONE_ZOMBIE_EATING_1_COLD_DIRECTORY
    11, // CONE_ZOMBIE_EATING_2_COLD_DIRECTORY
    11, // CONE_ZOMBIE_EATING_3_COLD_DIRECTORY

    // BUCKETHEAD ZOMBIE-------------------------------------------------
    12, // BUCKET_DROP_DIRECTORY

    15, // BUCKET_ZOMBIE_WALK_1_DIRECTORY
    15, // BUCKET_ZOMBIE_WALK_2_DIRECTORY
    15, // BUCKET_ZOMBIE_WALK_3_DIRECTORY

    15, // BUCKET_ZOMBIE_WALK_1_BLINK_DIRECTORY
    15, // BUCKET_ZOMBIE_WALK_2_BLINK_DIRECTORY
    15, // BUCKET_ZOMBIE_WALK_3_BLINK_DIRECTORY

    15, // BUCKET_ZOMBIE_WALK_1_COLD_DIRECTORY
    15, // BUCKET_ZOMBIE_WALK_2_COLD_DIRECTORY
    15, // BUCKET_ZOMBIE_WALK_3_COLD_DIRECTORY

    11, // BUCKET_ZOMBIE_EATING_1_DIRECTORY
    11, // BUCKET_ZOMBIE_EATING_2_DIRECTORY
    11, // BUCKET_ZOMBIE_EATING_3_DIRECTORY

    11, // BUCKET_ZOMBIE_EATING_1_BLINK_DIRECTORY
    11, // BUCKET_ZOMBIE_EATING_2_BLINK_DIRECTORY
    11, // BUCKET_ZOMBIE_EATING_3_BLINK_DIRECTORY

    11, // BUCKET_ZOMBIE_EATING_1_COLD_DIRECTORY
    11, // BUCKET_ZOMBIE_EATING_2_COLD_DIRECTORY
    11, // BUCKET_ZOMBIE_EATING_3_COLD_DIRECTORY

    //--------------------------------------------------------------------
    0, // START_READY_DIRECTORY
    0, // START_SET_DIRECTORY
    0, // START_PLANT_DIRECTORY
    0, // HUGE_WAVE_DIRECTORY
    0, // FINAL_WAVE_DIRECTORY

    0, // BLACK_SCREEN_DIRECTORY
    0, // WHITE_SCREEN_DIRECTORY

    0, // FRONTYARD_C1
    0, // FRONTYARD_C2
    0, // FRONTYARD_C3
    0, // FRONTYARD_C4
    0, // FRONTYARD_C5
    0, // FRONTYARD_C6
    0, // FRONTYARD_C7
    0, // FRONTYARD_C8
    0, // FRONTYARD_C9
    0, // FRONTYARD_R1
    0, // FRONTYARD_R2
    0, // FRONTYARD_R3
    0, // FRONTYARD_R4
    0, // FRONTYARD_R5

    0, // WIN_00_DIRECTORY
    0, // WIN_01_DIRECTORY
    0, // WIN_02_DIRECTORY
    0, // WIN_03_DIRECTORY
    0, // WIN_04_DIRECTORY
    0, // WIN_05_DIRECTORY
    0, // WIN_06_DIRECTORY
    0, // WIN_07_DIRECTORY
    0, // WIN_08_DIRECTORY
    0, // WIN_09_DIRECTORY
    0, // WIN_10_DIRECTORY
    0, // WIN_11_DIRECTORY
    0, // WIN_12_DIRECTORY
    0, // WIN_13_DIRECTORY
    0, // WIN_14_DIRECTORY
    0, // WIN_15_DIRECTORY
    0, // WIN_16_DIRECTORY
    0, // WIN_17_DIRECTORY
    0, // WIN_18_DIRECTORY
    0, // WIN_19_DIRECTORY
    0, // WIN_20_DIRECTORY
    0, // WIN_21_DIRECTORY
    0, // WIN_22_DIRECTORY
    0, // WIN_23_DIRECTORY
    0, // WIN_24_DIRECTORY
    0, // WIN_25_DIRECTORY
    0, // WIN_26_DIRECTORY
    0, // WIN_27_DIRECTORY
    0, // WIN_28_DIRECTORY
    0, // WIN_29_DIRECTORY
    0, // WIN_30_DIRECTORY
    0, // WIN_31_DIRECTORY
    0, // WIN_32_DIRECTORY
    0, // WIN_33_DIRECTORY
    0, // WIN_34_DIRECTORY
    0, // WIN_35_DIRECTORY
    0, // WIN_36_DIRECTORY
    0, // WIN_37_DIRECTORY
    0, // WIN_38_DIRECTORY
    0, // WIN_39_DIRECTORY
    0, // WIN_40_DIRECTORY
    0, // WIN_41_DIRECTORY
    0, // WIN_42_DIRECTORY
    0, // WIN_43_DIRECTORY
    0, // WIN_44_DIRECTORY
    0, // WIN_45_DIRECTORY
    0, // WIN_46_DIRECTORY
    0, // WIN_47_DIRECTORY
    0, // WIN_48_DIRECTORY
    0, // WIN_49_DIRECTORY
    0, // WIN_50_DIRECTORY
    0, // WIN_51_DIRECTORY
    0, // WIN_52_DIRECTORY
    0, // WIN_53_DIRECTORY
    0, // WIN_54_DIRECTORY
    0, // WIN_55_DIRECTORY
    0, // WIN_56_DIRECTORY
    0, // WIN_57_DIRECTORY
    0, // WIN_58_DIRECTORY
    0, // WIN_59_DIRECTORY
    0, // WIN_60_DIRECTORY
    0, // WIN_61_DIRECTORY
    0, // WIN_62_DIRECTORY
    0, // WIN_63_DIRECTORY
    0, // WIN_64_DIRECTORY
    0, // WIN_65_DIRECTORY
    0, // WIN_66_DIRECTORY
    0, // WIN_67_DIRECTORY
    0, // WIN_68_DIRECTORY
    0, // WIN_69_DIRECTORY
    0, // WIN_70_DIRECTORY
    0, // WIN_71_DIRECTORY
    0, // WIN_72_DIRECTORY
    0, // WIN_73_DIRECTORY

    1,  // MOUSE_CURSOR_DIRECTORY
    17, // MOWER_DIRECTORY
};

/*
The number of column in sprite of images
*/
const int C_SHEET[] = {
    0, // PEASHOOTER_DIRECTORY
    0, // SUNFLOWER_DIRECTORY
    0, // WALNUT_DIRECTORY
    0, // SNOWPEA_DIRECTORY
    0, // CHERRYBOMB_DIRECTORY

    // Plant sheet
    8, // PEASHOOTER_SHEET_DIRECTORY
    8, // PEASHOOTER_SHEET_BLINK_DIRECTORY
    8, // PEASHOOTER_ATTACK_DIRECTORY
    8, // PEASHOOTER_ATTACK_BLINK_DIRECTORY

    8, // SUNFLOWER_SHEET_DIRECTORY
    8, // SUNFLOWER_SHEET_BLINK_DIRECTORY
    8, // SUNFLOWER_HAPPY_DIRECTORY
    8, // SUNFLOWER_HAPPY_BLINK_DIRECTORY

    3, // WALNUT_1_DIRECTORY
    2, // WALNUT_2_DIRECTORY
    2, // WALNUT_3_DIRECTORY
    2, // WALNUT_4_DIRECTORY
    3, // WALNUT_1_BLINK_DIRECTORY
    2, // WALNUT_2_BLINK_DIRECTORY
    2, // WALNUT_3_BLINK_DIRECTORY
    2, // WALNUT_4_BLINK_DIRECTORY

    8, // SNOWPEA_SHEET_DIRECTORY
    8, // SNOWPEA_ATTACK_DIRECTORY

    8, // CHERRYBOMB_SHEET_DIRECTORY
    8, // CHERRYBOMB_SHEET_BLINK_DIRECTORY

    0, // PEASHOOTER_ICON_BRIGHT_DIRECTORY
    0, // PEASHOOTER_ICON_DIM_DIRECTORY
    0, // SUNFLOWER_ICON_BRIGHT_DIRECTORY
    0, // SUNFLOWER_ICON_DIM_DIRECTORY
    0, // WALNUT_ICON_BRIGHT_DIRECTORY
    0, // WALNUT_ICON_DIM_DIRECTORY
    0, // SNOWPEA_ICON_BRIGHT_DIRECTORY
    0, // SNOWPEA_ICON_DIM_DIRECTORY
    0, // CHERRYBOMB_ICON_BRIGHT_DIRECTORY
    0, // CHERRYBOMB_ICON_DIM_DIRECTORY

    0, // SUNFLOWER_TROPHY_DIRECTORY
    0, // NEW_PEASHOOTER_DIRECTORY
    0, // NEW_SUNFLOWER_DIRECTORY
    0, // NEW_WALNUT_DIRECTORY
    0, // NEW_SNOWPEA_DIRECTORY
    0, // NEW_CHERRYBOMB_DIRECTORY

    0, // PEA_DIRECTORY
    0, // PEA_EXPLODE_DIRECTORY
    0, // SNOWZ_PEA_DIRECTORY
    0, // SNOWZ_PEA_EXPLODE_DIRECTORY

    0, // STARTING_SCREEN_DIRECTORY
    0, // STARTING_SCREEN_CLICKED_DIRECTORY
    0, // CHOOSE_LEVELS_DIRECTORY
    0, // LEVEL_BLINK_DIRECTORY
    0, // BACKGROUND_LV1_DIRECTORY
    0, // BACKGROUND_LV2_DIRECTORY
    0, // BACKGROUND_DIRECTORY
    0, // BACKGROUND_NIGHT_DIRECTORY
    0, // LOSING_MESSAGE_DIRECTORY
    0, // WINNING_MESSAGE_DIRECTORY

    0, // SUN_DIRECTORY
    0, // SUN2_DIRECTORY
    0, // SUN_BAR_DIRECTORY
    0, // ICON_BAR_DIRECTORY
    0, // SHOVEL_BAR_DIRECTORY
    0, // SHOVEL_DIRECTORY
    0, // FLAG_METER
    0, // FLAG_METER_PROGRESS
    0, // FLAG_METER_PART

    0, // MENU_ICON_DIRECTORY
    0, // TURBO_ICON_DIRECTORY
    0, // TURBO_ICON_A_DIRECTORY
    0, // MENU_DIRECTORY
    0, // CONTINUE_DIRECTORY
    0, // RENAME_DIRECTORY
    0, // RESET_LEVEL_DIRECTORY
    0, // QUIT_DIRECTORY
    0, // OK_DIRECTORY
    0, // CANCEL_DIRECTORY
    0, // RENAME_MENU_DIRECTORY
    0, // NEW_USER_DIRECTORY
    0, // LEAVE_PROMPT_DIRECTORY
    0, // QUIT_PROMPT_DIRECTORY
    0, // RESET_PROMPT_DIRECTORY
    0, // RESTART_PROMPT_DIRECTORY

    // NORMAL ZOMBIE------------------------------------------------------------
    8, // ZOMBIE_WALK1_DIRECTORY
    8, // ZOMBIE_WALK2_DIRECTORY
    8, // ZOMBIE_WALK3_DIRECTORY

    8, // ZOMBIE_WALK1_BLINK_DIRECTORY
    8, // ZOMBIE_WALK2_BLINK_DIRECTORY
    8, // ZOMBIE_WALK3_BLINK_DIRECTORY

    8, // ZOMBIE_WALK1_COLD_DIRECTORY
    8, // ZOMBIE_WALK2_COLD_DIRECTORY
    8, // ZOMBIE_WALK3_COLD_DIRECTORY

    3,  // ZOMBIE_DIE_DIRECTORY
    3,  // ZOMBIE_DIE_COLD_DIRECTORY
    12, // ZOMBIE_HEAD_DIRECTORY
    12, // ZOMBIE_HEAD_COLD_DIRECTORY
    8,  // ZOMBIE_BURNT_DIRECTORY

    8, // ZOMBIE_EATING_DIRECTORY
    8, // ZOMBIE_EATING_BLINK_DIRECTORY
    8, // ZOMBIE_EATING_COLD_DIRECTORY

    // FLAG ZOMBIE--------------------------------------------------------
    8, // FLAG_ZOMBIE_WALK_DIRECTORY
    8, // FLAG_ZOMBIE_WALK_BLINK_DIRECTORY
    8, // FLAG_ZOMBIE_WALK_COLD_DIRECTORY

    8, // FLAG_ZOMBIE_EATING_DIRECTORY
    8, // FLAG_ZOMBIE_EATING_BLINK_DIRECTORY
    8, // FLAG_ZOMBIE_EATING_COLD_DIRECTORY

    // CONEHEAD ZOMBIE------------------------------------------------------------
    12, // CONE_DROP_DIRECTORY

    8, // CONE_ZOMBIE_WALK_1_DIRECTORY
    8, // CONE_ZOMBIE_WALK_2_DIRECTORY
    8, // CONE_ZOMBIE_WALK_3_DIRECTORY

    8, // CONE_ZOMBIE_WALK_1_BLINK_DIRECTORY
    8, // CONE_ZOMBIE_WALK_2_BLINK_DIRECTORY
    8, // CONE_ZOMBIE_WALK_3_BLINK_DIRECTORY

    8, // CONE_ZOMBIE_WALK_1_COLD_DIRECTORY
    8, // CONE_ZOMBIE_WALK_2_COLD_DIRECTORY
    8, // CONE_ZOMBIE_WALK_3_COLD_DIRECTORY

    8, // CONE_ZOMBIE_EATING_1_DIRECTORY
    8, // CONE_ZOMBIE_EATING_2_DIRECTORY
    8, // CONE_ZOMBIE_EATING_3_DIRECTORY

    8, // CONE_ZOMBIE_EATING_1_BLINK_DIRECTORY
    8, // CONE_ZOMBIE_EATING_2_BLINK_DIRECTORY
    8, // CONE_ZOMBIE_EATING_3_BLINK_DIRECTORY

    8, // CONE_ZOMBIE_EATING_1_COLD_DIRECTORY
    8, // CONE_ZOMBIE_EATING_2_COLD_DIRECTORY
    8, // CONE_ZOMBIE_EATING_3_COLD_DIRECTORY

    // BUCKETHEAD ZOMBIE------------------------------------------------------------
    12, // BUCKET_DROP_DIRECTORY

    8, // BUCKET_ZOMBIE_WALK_1_DIRECTORY
    8, // BUCKET_ZOMBIE_WALK_2_DIRECTORY
    8, // BUCKET_ZOMBIE_WALK_3_DIRECTORY

    8, // BUCKET_ZOMBIE_WALK_1_BLINK_DIRECTORY
    8, // BUCKET_ZOMBIE_WALK_2_BLINK_DIRECTORY
    8, // BUCKET_ZOMBIE_WALK_3_BLINK_DIRECTORY

    8, // BUCKET_ZOMBIE_WALK_1_COLD_DIRECTORY
    8, // BUCKET_ZOMBIE_WALK_2_COLD_DIRECTORY
    8, // BUCKET_ZOMBIE_WALK_3_COLD_DIRECTORY

    8, // BUCKET_ZOMBIE_EATING_1_DIRECTORY
    8, // BUCKET_ZOMBIE_EATING_2_DIRECTORY
    8, // BUCKET_ZOMBIE_EATING_3_DIRECTORY

    8, // BUCKET_ZOMBIE_EATING_1_BLINK_DIRECTORY
    8, // BUCKET_ZOMBIE_EATING_2_BLINK_DIRECTORY
    8, // BUCKET_ZOMBIE_EATING_3_BLINK_DIRECTORY

    8, // BUCKET_ZOMBIE_EATING_1_COLD_DIRECTORY
    8, // BUCKET_ZOMBIE_EATING_2_COLD_DIRECTORY
    8, // BUCKET_ZOMBIE_EATING_3_COLD_DIRECTORY

    //-------------------------------------------------------------------------
    0, // START_READY_DIRECTORY
    0, // START_SET_DIRECTORY
    0, // START_PLANT_DIRECTORY
    0, // HUGE_WAVE_DIRECTORY
    0, // FINAL_WAVE_DIRECTORY

    0, // BLACK_SCREEN_DIRECTORY
    0, // WHITE_SCREEN_DIRECTORY

    0, // FRONTYARD_C1
    0, // FRONTYARD_C2
    0, // FRONTYARD_C3
    0, // FRONTYARD_C4
    0, // FRONTYARD_C5
    0, // FRONTYARD_C6
    0, // FRONTYARD_C7
    0, // FRONTYARD_C8
    0, // FRONTYARD_C9
    0, // FRONTYARD_R1
    0, // FRONTYARD_R2
    0, // FRONTYARD_R3
    0, // FRONTYARD_R4
    0, // FRONTYARD_R5

    0, // WIN_00_DIRECTORY
    0, // WIN_01_DIRECTORY
    0, // WIN_02_DIRECTORY
    0, // WIN_03_DIRECTORY
    0, // WIN_04_DIRECTORY
    0, // WIN_05_DIRECTORY
    0, // WIN_06_DIRECTORY
    0, // WIN_07_DIRECTORY
    0, // WIN_08_DIRECTORY
    0, // WIN_09_DIRECTORY
    0, // WIN_10_DIRECTORY
    0, // WIN_11_DIRECTORY
    0, // WIN_12_DIRECTORY
    0, // WIN_13_DIRECTORY
    0, // WIN_14_DIRECTORY
    0, // WIN_15_DIRECTORY
    0, // WIN_16_DIRECTORY
    0, // WIN_17_DIRECTORY
    0, // WIN_18_DIRECTORY
    0, // WIN_19_DIRECTORY
    0, // WIN_20_DIRECTORY
    0, // WIN_21_DIRECTORY
    0, // WIN_22_DIRECTORY
    0, // WIN_23_DIRECTORY
    0, // WIN_24_DIRECTORY
    0, // WIN_25_DIRECTORY
    0, // WIN_26_DIRECTORY
    0, // WIN_27_DIRECTORY
    0, // WIN_28_DIRECTORY
    0, // WIN_29_DIRECTORY
    0, // WIN_30_DIRECTORY
    0, // WIN_31_DIRECTORY
    0, // WIN_32_DIRECTORY
    0, // WIN_33_DIRECTORY
    0, // WIN_34_DIRECTORY
    0, // WIN_35_DIRECTORY
    0, // WIN_36_DIRECTORY
    0, // WIN_37_DIRECTORY
    0, // WIN_38_DIRECTORY
    0, // WIN_39_DIRECTORY
    0, // WIN_40_DIRECTORY
    0, // WIN_41_DIRECTORY
    0, // WIN_42_DIRECTORY
    0, // WIN_43_DIRECTORY
    0, // WIN_44_DIRECTORY
    0, // WIN_45_DIRECTORY
    0, // WIN_46_DIRECTORY
    0, // WIN_47_DIRECTORY
    0, // WIN_48_DIRECTORY
    0, // WIN_49_DIRECTORY
    0, // WIN_50_DIRECTORY
    0, // WIN_51_DIRECTORY
    0, // WIN_52_DIRECTORY
    0, // WIN_53_DIRECTORY
    0, // WIN_54_DIRECTORY
    0, // WIN_55_DIRECTORY
    0, // WIN_56_DIRECTORY
    0, // WIN_57_DIRECTORY
    0, // WIN_58_DIRECTORY
    0, // WIN_59_DIRECTORY
    0, // WIN_60_DIRECTORY
    0, // WIN_61_DIRECTORY
    0, // WIN_62_DIRECTORY
    0, // WIN_63_DIRECTORY
    0, // WIN_64_DIRECTORY
    0, // WIN_65_DIRECTORY
    0, // WIN_66_DIRECTORY
    0, // WIN_67_DIRECTORY
    0, // WIN_68_DIRECTORY
    0, // WIN_69_DIRECTORY
    0, // WIN_70_DIRECTORY
    0, // WIN_71_DIRECTORY
    0, // WIN_72_DIRECTORY
    0, // WIN_73_DIRECTORY

    1, // MOUSE_CURSOR_DIRECTORY
    8, // MOWER_DIRECTORY
};

/*
Corresponding blink of image
*/
static map<int, int> blink_of = {
    {PEASHOOTER_SHEET_DIRECTORY, PEASHOOTER_SHEET_BLINK_DIRECTORY},
    {PEASHOOTER_ATTACK_DIRECTORY, PEASHOOTER_ATTACK_BLINK_DIRECTORY},

    {SUNFLOWER_SHEET_DIRECTORY, SUNFLOWER_SHEET_BLINK_DIRECTORY},
    {SUNFLOWER_HAPPY_DIRECTORY, SUNFLOWER_HAPPY_BLINK_DIRECTORY},

    {WALNUT_1_DIRECTORY, WALNUT_1_BLINK_DIRECTORY},
    {WALNUT_2_DIRECTORY, WALNUT_2_BLINK_DIRECTORY},
    {WALNUT_3_DIRECTORY, WALNUT_3_BLINK_DIRECTORY},
    {WALNUT_4_DIRECTORY, WALNUT_4_BLINK_DIRECTORY},

    {SNOWPEA_SHEET_DIRECTORY, PEASHOOTER_SHEET_BLINK_DIRECTORY},
    {SNOWPEA_ATTACK_DIRECTORY, PEASHOOTER_ATTACK_BLINK_DIRECTORY},

    {CHERRYBOMB_SHEET_DIRECTORY, CHERRYBOMB_SHEET_BLINK_DIRECTORY},

    {ZOMBIE_WALK1_DIRECTORY, ZOMBIE_WALK1_BLINK_DIRECTORY},
    {ZOMBIE_WALK2_DIRECTORY, ZOMBIE_WALK2_BLINK_DIRECTORY},
    {ZOMBIE_WALK3_DIRECTORY, ZOMBIE_WALK3_BLINK_DIRECTORY},
    {ZOMBIE_EATING_DIRECTORY, ZOMBIE_EATING_BLINK_DIRECTORY},

    {FLAG_ZOMBIE_WALK_DIRECTORY, FLAG_ZOMBIE_WALK_BLINK_DIRECTORY},
    {FLAG_ZOMBIE_EATING_DIRECTORY, FLAG_ZOMBIE_EATING_BLINK_DIRECTORY},

    {CONE_ZOMBIE_WALK_1_DIRECTORY, CONE_ZOMBIE_WALK_1_BLINK_DIRECTORY},
    {CONE_ZOMBIE_WALK_2_DIRECTORY, CONE_ZOMBIE_WALK_2_BLINK_DIRECTORY},
    {CONE_ZOMBIE_WALK_3_DIRECTORY, CONE_ZOMBIE_WALK_3_BLINK_DIRECTORY},
    {CONE_ZOMBIE_EATING_1_DIRECTORY, CONE_ZOMBIE_EATING_1_BLINK_DIRECTORY},
    {CONE_ZOMBIE_EATING_2_DIRECTORY, CONE_ZOMBIE_EATING_2_BLINK_DIRECTORY},
    {CONE_ZOMBIE_EATING_3_DIRECTORY, CONE_ZOMBIE_EATING_3_BLINK_DIRECTORY},

    {BUCKET_ZOMBIE_WALK_1_DIRECTORY, BUCKET_ZOMBIE_WALK_1_BLINK_DIRECTORY},
    {BUCKET_ZOMBIE_WALK_2_DIRECTORY, BUCKET_ZOMBIE_WALK_2_BLINK_DIRECTORY},
    {BUCKET_ZOMBIE_WALK_3_DIRECTORY, BUCKET_ZOMBIE_WALK_3_BLINK_DIRECTORY},
    {BUCKET_ZOMBIE_EATING_1_DIRECTORY, BUCKET_ZOMBIE_EATING_1_BLINK_DIRECTORY},
    {BUCKET_ZOMBIE_EATING_2_DIRECTORY, BUCKET_ZOMBIE_EATING_2_BLINK_DIRECTORY},
    {BUCKET_ZOMBIE_EATING_3_DIRECTORY, BUCKET_ZOMBIE_EATING_3_BLINK_DIRECTORY}

};

/*
Corresponding cold of image
*/
static map<int, int> cold_of = {

    {ZOMBIE_WALK1_DIRECTORY, ZOMBIE_WALK1_COLD_DIRECTORY},
    {ZOMBIE_WALK2_DIRECTORY, ZOMBIE_WALK2_COLD_DIRECTORY},
    {ZOMBIE_WALK3_DIRECTORY, ZOMBIE_WALK3_COLD_DIRECTORY},
    {ZOMBIE_EATING_DIRECTORY, ZOMBIE_EATING_COLD_DIRECTORY},

    {FLAG_ZOMBIE_WALK_DIRECTORY, FLAG_ZOMBIE_WALK_COLD_DIRECTORY},
    {FLAG_ZOMBIE_EATING_DIRECTORY, FLAG_ZOMBIE_EATING_COLD_DIRECTORY},

    {ZOMBIE_HEAD_DIRECTORY, ZOMBIE_HEAD_COLD_DIRECTORY},
    {ZOMBIE_DIE_DIRECTORY, ZOMBIE_DIE_COLD_DIRECTORY},

    {CONE_ZOMBIE_WALK_1_DIRECTORY, CONE_ZOMBIE_WALK_1_COLD_DIRECTORY},
    {CONE_ZOMBIE_WALK_2_DIRECTORY, CONE_ZOMBIE_WALK_2_COLD_DIRECTORY},
    {CONE_ZOMBIE_WALK_3_DIRECTORY, CONE_ZOMBIE_WALK_3_COLD_DIRECTORY},
    {CONE_ZOMBIE_EATING_1_DIRECTORY, CONE_ZOMBIE_EATING_1_COLD_DIRECTORY},
    {CONE_ZOMBIE_EATING_2_DIRECTORY, CONE_ZOMBIE_EATING_2_COLD_DIRECTORY},
    {CONE_ZOMBIE_EATING_3_DIRECTORY, CONE_ZOMBIE_EATING_3_COLD_DIRECTORY},

    {BUCKET_ZOMBIE_WALK_1_DIRECTORY, BUCKET_ZOMBIE_WALK_1_COLD_DIRECTORY},
    {BUCKET_ZOMBIE_WALK_2_DIRECTORY, BUCKET_ZOMBIE_WALK_2_COLD_DIRECTORY},
    {BUCKET_ZOMBIE_WALK_3_DIRECTORY, BUCKET_ZOMBIE_WALK_3_COLD_DIRECTORY},
    {BUCKET_ZOMBIE_EATING_1_DIRECTORY, BUCKET_ZOMBIE_EATING_1_COLD_DIRECTORY},
    {BUCKET_ZOMBIE_EATING_2_DIRECTORY, BUCKET_ZOMBIE_EATING_2_COLD_DIRECTORY},
    {BUCKET_ZOMBIE_EATING_3_DIRECTORY, BUCKET_ZOMBIE_EATING_3_COLD_DIRECTORY}

};

/*
Corresponding eating of zombie walk image
*/
static map<int, int> eat_of = {

    {ZOMBIE_WALK1_DIRECTORY, ZOMBIE_EATING_DIRECTORY},
    {ZOMBIE_WALK2_DIRECTORY, ZOMBIE_EATING_DIRECTORY},
    {ZOMBIE_WALK3_DIRECTORY, ZOMBIE_EATING_DIRECTORY},

    {FLAG_ZOMBIE_WALK_DIRECTORY, FLAG_ZOMBIE_EATING_DIRECTORY},

    {CONE_ZOMBIE_WALK_1_DIRECTORY, CONE_ZOMBIE_EATING_1_DIRECTORY},
    {CONE_ZOMBIE_WALK_2_DIRECTORY, CONE_ZOMBIE_EATING_2_DIRECTORY},
    {CONE_ZOMBIE_WALK_3_DIRECTORY, CONE_ZOMBIE_EATING_3_DIRECTORY},

    {BUCKET_ZOMBIE_WALK_1_DIRECTORY, BUCKET_ZOMBIE_EATING_1_DIRECTORY},
    {BUCKET_ZOMBIE_WALK_2_DIRECTORY, BUCKET_ZOMBIE_EATING_2_DIRECTORY},
    {BUCKET_ZOMBIE_WALK_3_DIRECTORY, BUCKET_ZOMBIE_EATING_3_DIRECTORY}

};

/*
Corresponding walking of zombie eat image
*/
static map<int, int> walk_of = {

    {ZOMBIE_WALK1_DIRECTORY, ZOMBIE_EATING_DIRECTORY},
    {ZOMBIE_WALK2_DIRECTORY, ZOMBIE_EATING_DIRECTORY},
    {ZOMBIE_WALK3_DIRECTORY, ZOMBIE_EATING_DIRECTORY},

    {FLAG_ZOMBIE_EATING_DIRECTORY, FLAG_ZOMBIE_WALK_DIRECTORY},

    {CONE_ZOMBIE_EATING_1_DIRECTORY, CONE_ZOMBIE_WALK_1_DIRECTORY},
    {CONE_ZOMBIE_EATING_2_DIRECTORY, CONE_ZOMBIE_WALK_2_DIRECTORY},
    {CONE_ZOMBIE_EATING_3_DIRECTORY, CONE_ZOMBIE_WALK_3_DIRECTORY},

    {BUCKET_ZOMBIE_EATING_1_DIRECTORY, BUCKET_ZOMBIE_WALK_1_DIRECTORY},
    {BUCKET_ZOMBIE_EATING_2_DIRECTORY, BUCKET_ZOMBIE_WALK_2_DIRECTORY},
    {BUCKET_ZOMBIE_EATING_3_DIRECTORY, BUCKET_ZOMBIE_WALK_3_DIRECTORY}

};

const int front_yard_r = FRONTYARD_R1;
const int front_yard_c = FRONTYARD_C1;
const int win_dir = WIN_00_DIRECTORY;