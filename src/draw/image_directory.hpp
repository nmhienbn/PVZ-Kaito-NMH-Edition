#pragma once
#include <string>
#include <map>
using namespace std;

#define ZOMBIE_WALK_DIRECTORY rand(ZOMBIE_WALK1_DIRECTORY, ZOMBIE_WALK2_DIRECTORY)
#define ARMLESS_ZOMBIE_WALK_DIRECTORY rand(ARMLESS_ZOMBIE_WALK1_DIRECTORY, ARMLESS_ZOMBIE_WALK2_DIRECTORY)
#define ZOMBIE_DIE_DIRECTORY rand(ZOMBIE_DIE1_DIRECTORY, ZOMBIE_DIE2_DIRECTORY)

#define FLAG_ZOMBIE_WALK_1_DIRECTORY rand(FLAG_ZOMBIE_WALK1_1_DIRECTORY, FLAG_ZOMBIE_WALK2_1_DIRECTORY)
#define FLAG_ZOMBIE_WALK_2_DIRECTORY rand(FLAG_ZOMBIE_WALK1_2_DIRECTORY, FLAG_ZOMBIE_WALK2_2_DIRECTORY)
#define FLAG_ZOMBIE_WALK_3_DIRECTORY rand(FLAG_ZOMBIE_WALK1_3_DIRECTORY, FLAG_ZOMBIE_WALK2_3_DIRECTORY)

#define CONE_ZOMBIE_WALK_1_DIRECTORY rand(CONE_ZOMBIE_WALK1_1_DIRECTORY, CONE_ZOMBIE_WALK2_1_DIRECTORY)
#define CONE_ZOMBIE_WALK_2_DIRECTORY rand(CONE_ZOMBIE_WALK1_2_DIRECTORY, CONE_ZOMBIE_WALK2_2_DIRECTORY)
#define CONE_ZOMBIE_WALK_3_DIRECTORY rand(CONE_ZOMBIE_WALK1_3_DIRECTORY, CONE_ZOMBIE_WALK2_3_DIRECTORY)

#define BUCKET_ZOMBIE_WALK_1_DIRECTORY rand(BUCKET_ZOMBIE_WALK1_1_DIRECTORY, BUCKET_ZOMBIE_WALK2_1_DIRECTORY)
#define BUCKET_ZOMBIE_WALK_2_DIRECTORY rand(BUCKET_ZOMBIE_WALK1_2_DIRECTORY, BUCKET_ZOMBIE_WALK2_2_DIRECTORY)
#define BUCKET_ZOMBIE_WALK_3_DIRECTORY rand(BUCKET_ZOMBIE_WALK1_3_DIRECTORY, BUCKET_ZOMBIE_WALK2_3_DIRECTORY)

#define DOOR_ZOMBIE_WALK_1_DIRECTORY rand(DOOR_ZOMBIE_WALK1_1_DIRECTORY, DOOR_ZOMBIE_WALK2_1_DIRECTORY)
#define DOOR_ZOMBIE_WALK_2_DIRECTORY rand(DOOR_ZOMBIE_WALK1_2_DIRECTORY, DOOR_ZOMBIE_WALK2_2_DIRECTORY)
#define DOOR_ZOMBIE_WALK_3_DIRECTORY rand(DOOR_ZOMBIE_WALK1_3_DIRECTORY, DOOR_ZOMBIE_WALK2_3_DIRECTORY)

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
    WALNUT_5_DIRECTORY,
    WALNUT_1_BLINK_DIRECTORY,
    WALNUT_2_BLINK_DIRECTORY,
    WALNUT_3_BLINK_DIRECTORY,
    WALNUT_4_BLINK_DIRECTORY,
    WALNUT_5_BLINK_DIRECTORY,

    SNOWPEA_SHEET_DIRECTORY,
    SNOWPEA_SHEET_BLINK_DIRECTORY,
    SNOWPEA_ATTACK_DIRECTORY,
    SNOWPEA_ATTACK_BLINK_DIRECTORY,

    CHERRYBOMB_SHEET_DIRECTORY,
    CHERRYBOMB_SHEET_BLINK_DIRECTORY,

    SEED_CHOSEN_DIRECTORY,
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
    SUN_BAR_DIRECTORY,
    ICON_BAR_DIRECTORY,
    SHOVEL_BAR_DIRECTORY,
    SHOVEL_DIRECTORY,
    FLAG_METER,
    FLAG_METER_PROGRESS,
    FLAG_METER_PART,

    PAUSE_ICON_DIRECTORY,
    PAUSE_ICON_A_DIRECTORY,
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
    SLIDER_DIRECTORY,

    // NORMAL ZOMBIE---------------------------------------------------
    ZOMBIE_CREDIT1_DIRECTORY,
    ZOMBIE_CREDIT2_DIRECTORY,
    ZOMBIE_ARM_DIRECTORY,
    ZOMBIE_ARM_COLD_DIRECTORY,

    ZOMBIE_WALK1_DIRECTORY,
    ZOMBIE_WALK2_DIRECTORY,

    ZOMBIE_WALK1_BLINK_DIRECTORY,
    ZOMBIE_WALK2_BLINK_DIRECTORY,

    ZOMBIE_WALK1_COLD_DIRECTORY,
    ZOMBIE_WALK2_COLD_DIRECTORY,

    ZOMBIE_DIE1_DIRECTORY,
    ZOMBIE_DIE2_DIRECTORY,
    ZOMBIE_DIE1_COLD_DIRECTORY,
    ZOMBIE_DIE2_COLD_DIRECTORY,
    ZOMBIE_HEAD_DIRECTORY,
    ZOMBIE_HEAD_COLD_DIRECTORY,
    ZOMBIE_BURNT_DIRECTORY,

    ZOMBIE_EATING_DIRECTORY,
    ZOMBIE_EATING_BLINK_DIRECTORY,
    ZOMBIE_EATING_COLD_DIRECTORY,

    // ARMLESS ZOMBIE--------------------------------------------------------

    ARMLESS_ZOMBIE_WALK1_DIRECTORY,
    ARMLESS_ZOMBIE_WALK2_DIRECTORY,
    ARMLESS_ZOMBIE_WALK1_BLINK_DIRECTORY,
    ARMLESS_ZOMBIE_WALK2_BLINK_DIRECTORY,
    ARMLESS_ZOMBIE_WALK1_COLD_DIRECTORY,
    ARMLESS_ZOMBIE_WALK2_COLD_DIRECTORY,
    ARMLESS_ZOMBIE_EATING_DIRECTORY,
    ARMLESS_ZOMBIE_EATING_BLINK_DIRECTORY,
    ARMLESS_ZOMBIE_EATING_COLD_DIRECTORY,

    // FLAG ZOMBIE--------------------------------------------------------

    FLAG_ZOMBIE_WALK1_1_DIRECTORY,
    FLAG_ZOMBIE_WALK2_1_DIRECTORY,
    FLAG_ZOMBIE_WALK1_1_BLINK_DIRECTORY,
    FLAG_ZOMBIE_WALK2_1_BLINK_DIRECTORY,
    FLAG_ZOMBIE_WALK1_1_COLD_DIRECTORY,
    FLAG_ZOMBIE_WALK2_1_COLD_DIRECTORY,
    FLAG_ZOMBIE_EATING_1_DIRECTORY,
    FLAG_ZOMBIE_EATING_1_BLINK_DIRECTORY,
    FLAG_ZOMBIE_EATING_1_COLD_DIRECTORY,

    FLAG_ZOMBIE_WALK1_2_DIRECTORY,
    FLAG_ZOMBIE_WALK2_2_DIRECTORY,
    FLAG_ZOMBIE_WALK1_2_BLINK_DIRECTORY,
    FLAG_ZOMBIE_WALK2_2_BLINK_DIRECTORY,
    FLAG_ZOMBIE_WALK1_2_COLD_DIRECTORY,
    FLAG_ZOMBIE_WALK2_2_COLD_DIRECTORY,
    FLAG_ZOMBIE_EATING_2_DIRECTORY,
    FLAG_ZOMBIE_EATING_2_BLINK_DIRECTORY,
    FLAG_ZOMBIE_EATING_2_COLD_DIRECTORY,

    FLAG_ZOMBIE_WALK1_3_DIRECTORY,
    FLAG_ZOMBIE_WALK2_3_DIRECTORY,
    FLAG_ZOMBIE_WALK1_3_BLINK_DIRECTORY,
    FLAG_ZOMBIE_WALK2_3_BLINK_DIRECTORY,
    FLAG_ZOMBIE_WALK1_3_COLD_DIRECTORY,
    FLAG_ZOMBIE_WALK2_3_COLD_DIRECTORY,
    FLAG_ZOMBIE_EATING_3_DIRECTORY,
    FLAG_ZOMBIE_EATING_3_BLINK_DIRECTORY,
    FLAG_ZOMBIE_EATING_3_COLD_DIRECTORY,

    // CONEHEAD ZOMBIE--------------------------------------------------------
    CONE_DROP_DIRECTORY,
    CONE_ZOMBIE_CREDIT1_DIRECTORY,
    CONE_ZOMBIE_CREDIT2_DIRECTORY,

    CONE_ZOMBIE_WALK1_1_DIRECTORY,
    CONE_ZOMBIE_WALK2_1_DIRECTORY,
    CONE_ZOMBIE_WALK1_1_BLINK_DIRECTORY,
    CONE_ZOMBIE_WALK2_1_BLINK_DIRECTORY,
    CONE_ZOMBIE_WALK1_1_COLD_DIRECTORY,
    CONE_ZOMBIE_WALK2_1_COLD_DIRECTORY,
    CONE_ZOMBIE_EATING_1_DIRECTORY,
    CONE_ZOMBIE_EATING_1_BLINK_DIRECTORY,
    CONE_ZOMBIE_EATING_1_COLD_DIRECTORY,

    CONE_ZOMBIE_WALK1_2_DIRECTORY,
    CONE_ZOMBIE_WALK2_2_DIRECTORY,
    CONE_ZOMBIE_WALK1_2_BLINK_DIRECTORY,
    CONE_ZOMBIE_WALK2_2_BLINK_DIRECTORY,
    CONE_ZOMBIE_WALK1_2_COLD_DIRECTORY,
    CONE_ZOMBIE_WALK2_2_COLD_DIRECTORY,
    CONE_ZOMBIE_EATING_2_DIRECTORY,
    CONE_ZOMBIE_EATING_2_BLINK_DIRECTORY,
    CONE_ZOMBIE_EATING_2_COLD_DIRECTORY,

    CONE_ZOMBIE_WALK1_3_DIRECTORY,
    CONE_ZOMBIE_WALK2_3_DIRECTORY,
    CONE_ZOMBIE_WALK1_3_BLINK_DIRECTORY,
    CONE_ZOMBIE_WALK2_3_BLINK_DIRECTORY,
    CONE_ZOMBIE_WALK1_3_COLD_DIRECTORY,
    CONE_ZOMBIE_WALK2_3_COLD_DIRECTORY,
    CONE_ZOMBIE_EATING_3_DIRECTORY,
    CONE_ZOMBIE_EATING_3_BLINK_DIRECTORY,
    CONE_ZOMBIE_EATING_3_COLD_DIRECTORY,

    // BUCKETHEAD ZOMBIE--------------------------------------------------------
    BUCKET_DROP_DIRECTORY,
    BUCKET_ZOMBIE_CREDIT1_DIRECTORY,
    BUCKET_ZOMBIE_CREDIT2_DIRECTORY,

    BUCKET_ZOMBIE_WALK1_1_DIRECTORY,
    BUCKET_ZOMBIE_WALK2_1_DIRECTORY,
    BUCKET_ZOMBIE_WALK1_1_BLINK_DIRECTORY,
    BUCKET_ZOMBIE_WALK2_1_BLINK_DIRECTORY,
    BUCKET_ZOMBIE_WALK1_1_COLD_DIRECTORY,
    BUCKET_ZOMBIE_WALK2_1_COLD_DIRECTORY,
    BUCKET_ZOMBIE_EATING_1_DIRECTORY,
    BUCKET_ZOMBIE_EATING_1_BLINK_DIRECTORY,
    BUCKET_ZOMBIE_EATING_1_COLD_DIRECTORY,

    BUCKET_ZOMBIE_WALK1_2_DIRECTORY,
    BUCKET_ZOMBIE_WALK2_2_DIRECTORY,
    BUCKET_ZOMBIE_WALK1_2_BLINK_DIRECTORY,
    BUCKET_ZOMBIE_WALK2_2_BLINK_DIRECTORY,
    BUCKET_ZOMBIE_WALK1_2_COLD_DIRECTORY,
    BUCKET_ZOMBIE_WALK2_2_COLD_DIRECTORY,
    BUCKET_ZOMBIE_EATING_2_DIRECTORY,
    BUCKET_ZOMBIE_EATING_2_BLINK_DIRECTORY,
    BUCKET_ZOMBIE_EATING_2_COLD_DIRECTORY,

    BUCKET_ZOMBIE_WALK1_3_DIRECTORY,
    BUCKET_ZOMBIE_WALK2_3_DIRECTORY,
    BUCKET_ZOMBIE_WALK1_3_BLINK_DIRECTORY,
    BUCKET_ZOMBIE_WALK2_3_BLINK_DIRECTORY,
    BUCKET_ZOMBIE_WALK1_3_COLD_DIRECTORY,
    BUCKET_ZOMBIE_WALK2_3_COLD_DIRECTORY,
    BUCKET_ZOMBIE_EATING_3_DIRECTORY,
    BUCKET_ZOMBIE_EATING_3_BLINK_DIRECTORY,
    BUCKET_ZOMBIE_EATING_3_COLD_DIRECTORY,

    // DOOR ZOMBIE--------------------------------------------------------
    DOOR_ZOMBIE_CREDIT1_DIRECTORY,
    DOOR_ZOMBIE_CREDIT2_DIRECTORY,

    DOOR_ZOMBIE_WALK1_1_DIRECTORY,
    DOOR_ZOMBIE_WALK2_1_DIRECTORY,
    DOOR_ZOMBIE_WALK1_1_BLINK_DIRECTORY,
    DOOR_ZOMBIE_WALK2_1_BLINK_DIRECTORY,
    DOOR_ZOMBIE_WALK1_1_COLD_DIRECTORY,
    DOOR_ZOMBIE_WALK2_1_COLD_DIRECTORY,
    DOOR_ZOMBIE_EATING_1_DIRECTORY,
    DOOR_ZOMBIE_EATING_1_BLINK_DIRECTORY,
    DOOR_ZOMBIE_EATING_1_COLD_DIRECTORY,

    DOOR_ZOMBIE_WALK1_2_DIRECTORY,
    DOOR_ZOMBIE_WALK2_2_DIRECTORY,
    DOOR_ZOMBIE_WALK1_2_BLINK_DIRECTORY,
    DOOR_ZOMBIE_WALK2_2_BLINK_DIRECTORY,
    DOOR_ZOMBIE_WALK1_2_COLD_DIRECTORY,
    DOOR_ZOMBIE_WALK2_2_COLD_DIRECTORY,
    DOOR_ZOMBIE_EATING_2_DIRECTORY,
    DOOR_ZOMBIE_EATING_2_BLINK_DIRECTORY,
    DOOR_ZOMBIE_EATING_2_COLD_DIRECTORY,

    DOOR_ZOMBIE_WALK1_3_DIRECTORY,
    DOOR_ZOMBIE_WALK2_3_DIRECTORY,
    DOOR_ZOMBIE_WALK1_3_BLINK_DIRECTORY,
    DOOR_ZOMBIE_WALK2_3_BLINK_DIRECTORY,
    DOOR_ZOMBIE_WALK1_3_COLD_DIRECTORY,
    DOOR_ZOMBIE_WALK2_3_COLD_DIRECTORY,
    DOOR_ZOMBIE_EATING_3_DIRECTORY,
    DOOR_ZOMBIE_EATING_3_BLINK_DIRECTORY,
    DOOR_ZOMBIE_EATING_3_COLD_DIRECTORY,

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

    SHADOW_DIRECTORY,
    PEA_SHADOW_DIRECTORY,

    COUNT_USED_DIRECTORY
};

struct Image
{
    /*
    Contain all image directory corresponding
    */
    string img_dir;
    /*
    The number of sheet, column in sprite of images
    */
    int n_sheet, c_sheet;
    Image(string _dir);
    Image(string _dir, int _n, int _c);
};

const Image all_img[] = {
    Image("./image/Plant_sheet/peashooter/Peashooter.png"), // PEASHOOTER_DIRECTORY
    Image("./image/Plant_sheet/sunflower/Sunflower.png"),   // SUNFLOWER_DIRECTORY
    Image("./image/Plant_sheet/walnut/Walnut.png"),         // WALNUT_DIRECTORY
    Image("./image/Plant_sheet/snowpea/SnowPea.png"),       // SNOWPEA_DIRECTORY
    Image("./image/Plant_sheet/cherrybomb/Cherrybomb.png"), // CHERRYBOMB_DIRECTORY

    Image("./image/Plant_sheet/peashooter/peashooter_sheet.png", 60, 8),        // PEASHOOTER_SHEET_DIRECTORY
    Image("./image/Plant_sheet/peashooter/peashooter_sheet_blink.png", 60, 8),  // PEASHOOTER_SHEET_BLINK_DIRECTORY
    Image("./image/Plant_sheet/peashooter/peashooter_attack.png", 60, 8),       // PEASHOOTER_ATTACK_DIRECTORY
    Image("./image/Plant_sheet/peashooter/peashooter_attack_blink.png", 60, 8), // PEASHOOTER_ATTACK_BLINK_DIRECTORY

    Image("./image/Plant_sheet/sunflower/sunflower_sheet.png", 60, 8),       // SUNFLOWER_SHEET_DIRECTORY
    Image("./image/Plant_sheet/sunflower/sunflower_sheet_blink.png", 60, 8), // SUNFLOWER_SHEET_BLINK_DIRECTORY
    Image("./image/Plant_sheet/sunflower/sunflower_happy.png", 60, 8),       // SUNFLOWER_HAPPY_DIRECTORY
    Image("./image/Plant_sheet/sunflower/sunflower_happy_blink.png", 60, 8), // SUNFLOWER_HAPPY_BLINK_DIRECTORY

    Image("./image/Plant_sheet/walnut/Walnut-sheet-1.png", 58, 8),       // WALNUT_1_DIRECTORY
    Image("./image/Plant_sheet/walnut/Walnut-sheet-2.png", 31, 8),       // WALNUT_2_DIRECTORY
    Image("./image/Plant_sheet/walnut/Walnut-sheet-3.png", 50, 8),       // WALNUT_3_DIRECTORY
    Image("./image/Plant_sheet/walnut/Walnut-sheet-4.png", 50, 8),       // WALNUT_4_DIRECTORY
    Image("./image/Plant_sheet/walnut/Walnut-sheet-5.png", 50, 8),       // WALNUT_5_DIRECTORY
    Image("./image/Plant_sheet/walnut/Walnut-sheet-1_blink.png", 58, 8), // WALNUT_1_BLINK_DIRECTORY
    Image("./image/Plant_sheet/walnut/Walnut-sheet-2_blink.png", 31, 8), // WALNUT_2_BLINK_DIRECTORY
    Image("./image/Plant_sheet/walnut/Walnut-sheet-3_blink.png", 50, 8), // WALNUT_3_BLINK_DIRECTORY
    Image("./image/Plant_sheet/walnut/Walnut-sheet-4_blink.png", 50, 8), // WALNUT_4_BLINK_DIRECTORY
    Image("./image/Plant_sheet/walnut/Walnut-sheet-5_blink.png", 50, 8), // WALNUT_5_BLINK_DIRECTORY

    Image("./image/Plant_sheet/snowpea/snowpea_sheet.png", 30, 8),        // SNOWPEA_SHEET_DIRECTORY
    Image("./image/Plant_sheet/snowpea/snowpea_sheet_blink.png", 30, 8),  // SNOWPEA_SHEET_BLINK_DIRECTORY
    Image("./image/Plant_sheet/snowpea/snowpea_attack.png", 30, 8),       // SNOWPEA_ATTACK_DIRECTORY
    Image("./image/Plant_sheet/snowpea/snowpea_attack_blink.png", 30, 8), // SNOWPEA_ATTACK_BLINK_DIRECTORY

    Image("./image/Plant_sheet/cherrybomb/cherrybomb_sheet.png", 24, 8),       // CHERRYBOMB_SHEET_DIRECTORY
    Image("./image/Plant_sheet/cherrybomb/cherrybomb_sheet_blink.png", 24, 8), // CHERRYBOMB_SHEET_BLINK_DIRECTORY

    Image("./image/Plant_seed/Seedpacketselect.png"),           // SEED_CHOSEN_DIRECTORY
    Image("./image/Plant_seed/Peashooter_Seed_Packet.png"),     // PEASHOOTER_ICON_BRIGHT_DIRECTORY
    Image("./image/Plant_seed/Peashooter_Seed_Packet_dim.png"), // PEASHOOTER_ICON_DIM_DIRECTORY
    Image("./image/Plant_seed/Sunflower_Seed_Packet.png"),      // SUNFLOWER_ICON_BRIGHT_DIRECTORY
    Image("./image/Plant_seed/Sunflower_Seed_Packet_dim.png"),  // SUNFLOWER_ICON_DIM_DIRECTORY
    Image("./image/Plant_seed/Walnut_Seed_Packet.png"),         // WALNUT_ICON_BRIGHT_DIRECTORY
    Image("./image/Plant_seed/Walnut_Seed_Packet_dim.png"),     // WALNUT_ICON_DIM_DIRECTORY
    Image("./image/Plant_seed/Snowpea_Seed_Packet.png"),        // SNOWPEA_ICON_BRIGHT_DIRECTORY
    Image("./image/Plant_seed/Snowpea_Seed_Packet_dim.png"),    // SNOWPEA_ICON_DIM_DIRECTORY
    Image("./image/Plant_seed/Cherrybomb_Seed_Packet.png"),     // CHERRYBOMB_ICON_BRIGHT_DIRECTORY
    Image("./image/Plant_seed/Cherrybomb_Seed_Packet_dim.png"), // CHERRYBOMB_ICON_DIM_DIRECTORY

    Image("./image/New_plant/Golden_Sunflower_Trophy.png"), // SUNFLOWER_TROPHY_DIRECTORY
    Image("./image/New_plant/new_peashooter.png"),          // NEW_PEASHOOTER_DIRECTORY
    Image("./image/New_plant/new_sunflower.png"),           // NEW_SUNFLOWER_DIRECTORY
    Image("./image/New_plant/new_walnut.png"),              // NEW_WALNUT_DIRECTORY
    Image("./image/New_plant/new_snowpea.png"),             // NEW_SNOWPEA_DIRECTORY
    Image("./image/New_plant/new_cherrybomb.png"),          // NEW_CHERRYBOMB_DIRECTORY

    Image("./image/Bullets/Pea/pea.png"),                     // PEA_DIRECTORY
    Image("./image/Bullets/Pea/pea_explode.png"),             // PEA_EXPLODE_DIRECTORY
    Image("./image/Bullets/Snowz_pea/snowz_pea.png"),         // SNOWZ_PEA_DIRECTORY
    Image("./image/Bullets/Snowz_pea/snowz_pea_explode.png"), // SNOWZ_PEA_EXPLODE_DIRECTORY

    Image("./image/Background/Starting_Screen.png"),         // STARTING_SCREEN_DIRECTORY
    Image("./image/Background/Starting_Screen_clicked.png"), // STARTING_SCREEN_CLICKED_DIRECTORY
    Image("./image/Background/Choose_levels.png"),           // CHOOSE_LEVELS_DIRECTORY
    Image("./image/Background/level_blink.png"),             // LEVEL_BLINK_DIRECTORY
    Image("./image/Background/Frontyard_LV1.png"),           // BACKGROUND_LV1_DIRECTORY
    Image("./image/Background/Frontyard_LV2.png"),           // BACKGROUND_LV2_DIRECTORY
    Image("./image/Background/Frontyard.png"),               // BACKGROUND_DIRECTORY
    Image("./image/Background/Frontyard_night.png"),         // BACKGROUND_NIGHT_DIRECTORY
    Image("./image/Background/ZombiesWon.png"),              // LOSING_MESSAGE_DIRECTORY
    Image("./image/Background/Winning_Pic.png"),             // WINNING_MESSAGE_DIRECTORY

    Image("./image/Icons/sun.png"),                    // SUN_DIRECTORY
    Image("./image/Icons/sun_bar.png"),                // SUN_BAR_DIRECTORY
    Image("./image/Icons/Item_Bar.png"),               // ICON_BAR_DIRECTORY
    Image("./image/Icons/Shovel.png"),                 // SHOVEL_BAR_DIRECTORY
    Image("./image/Icons/Shovel2.png"),                // SHOVEL_DIRECTORY
    Image("./image/Icons/FlagMeter.png"),              // FLAG_METER
    Image("./image/Icons/FlagMeterLevelProgress.png"), // FLAG_METER_PROGRESS
    Image("./image/Icons/FlagMeterParts.png"),         // FLAG_METER_PART

    Image("./image/Icons/Pause_Button.png"),           // PAUSE_ICON_DIRECTORY
    Image("./image/Icons/Pause_Button_Activited.png"), // PAUSE_ICON_A_DIRECTORY
    Image("./image/Icons/Turbo_Button.png"),           // TURBO_ICON_DIRECTORY
    Image("./image/Icons/Turbo_Button_Activited.png"), // TURBO_ICON_A_DIRECTORY
    Image("./image/Icons/menu.png"),                   // MENU_DIRECTORY
    Image("./image/Icons/continue.png"),               // CONTINUE_DIRECTORY
    Image("./image/Icons/rename.png"),                 // RENAME_DIRECTORY
    Image("./image/Icons/reset_level.png"),            // RESET_LEVEL_DIRECTORY
    Image("./image/Icons/quit.png"),                   // QUIT_DIRECTORY
    Image("./image/Icons/ok.png"),                     // OK_DIRECTORY
    Image("./image/Icons/cancel.png"),                 // CANCEL_DIRECTORY
    Image("./image/Icons/rename_menu.png"),            // RENAME_MENU_DIRECTORY
    Image("./image/Icons/new_user.png"),               // NEW_USER_DIRECTORY
    Image("./image/Icons/leave_prompt.png"),           // LEAVE_PROMPT_DIRECTORY
    Image("./image/Icons/quit_prompt.png"),            // QUIT_PROMPT_DIRECTORY
    Image("./image/Icons/reset_prompt.png"),           // RESET_PROMPT_DIRECTORY
    Image("./image/Icons/restart_prompt.png"),         // RESTART_PROMPT_DIRECTORY
    Image("./image/Icons/options_sliderknob2.png"),    // SLIDER_DIRECTORY

    // NORMAL ZOMBIE------------------------------------------------------------------------
    Image("./image/Zombie/Normal/Zombie_credit1.png", 22, 8),     // ZOMBIE_CREDIT1_DIRECTORY
    Image("./image/Zombie/Normal/Zombie_credit2.png", 22, 8),     // ZOMBIE_CREDIT2_DIRECTORY
    Image("./image/Zombie/Normal/Zombie_arm.png", 30, 8),         // ZOMBIE_ARM_DIRECTORY
    Image("./image/Zombie/Normal/Zombie_arm_cold.png", 30, 8),    // ZOMBIE_ARM_COLD_DIRECTORY
    Image("./image/Zombie/Normal/ZombieWalk1.png", 46, 8),        // ZOMBIE_WALK1_DIRECTORY
    Image("./image/Zombie/Normal/ZombieWalk2.png", 46, 8),        // ZOMBIE_WALK2_DIRECTORY
    Image("./image/Zombie/Normal/ZombieWalk1_blink.png", 46, 8),  // ZOMBIE_WALK1_BLINK_DIRECTORY
    Image("./image/Zombie/Normal/ZombieWalk2_blink.png", 46, 8),  // ZOMBIE_WALK2_BLINK_DIRECTORY
    Image("./image/Zombie/Normal/ZombieWalk1_cold.png", 46, 8),   // ZOMBIE_WALK1_COLD_DIRECTORY
    Image("./image/Zombie/Normal/ZombieWalk2_cold.png", 46, 8),   // ZOMBIE_WALK2_COLD_DIRECTORY
    Image("./image/Zombie/Normal/Zombie_die1.png", 33, 8),        // ZOMBIE_DIE1_DIRECTORY
    Image("./image/Zombie/Normal/Zombie_die2.png", 39, 8),        // ZOMBIE_DIE2_DIRECTORY
    Image("./image/Zombie/Normal/Zombie_die1_cold.png", 33, 8),   // ZOMBIE_DIE1_COLD_DIRECTORY
    Image("./image/Zombie/Normal/Zombie_die2_cold.png", 39, 8),   // ZOMBIE_DIE2_COLD_DIRECTORY
    Image("./image/Zombie/Normal/Zombie_head.png", 12, 12),       // ZOMBIE_HEAD_DIRECTORY
    Image("./image/Zombie/Normal/Zombie_head_cold.png", 12, 12),  // ZOMBIE_HEAD_COLD_DIRECTORY
    Image("./image/Zombie/Zombie_burnt.png", 43, 8),              // ZOMBIE_BURNT_DIRECTORY
    Image("./image/Zombie/Normal/ZombieAttack.png", 39, 8),       // ZOMBIE_EATING_DIRECTORY
    Image("./image/Zombie/Normal/ZombieAttack_blink.png", 39, 8), // ZOMBIE_EATING_BLINK_DIRECTORY
    Image("./image/Zombie/Normal/ZombieAttack_cold.png", 39, 8),  // ZOMBIE_EATING_COLD_DIRECTORY

    // ARMLESS ZOMBIE--------------------------------------------------------
    Image("./image/Zombie/Armless/ArmlessWalk1.png", 46, 8),        // ARMLESS_ZOMBIE_WALK1_DIRECTORY
    Image("./image/Zombie/Armless/ArmlessWalk2.png", 46, 8),        // ARMLESS_ZOMBIE_WALK2_DIRECTORY
    Image("./image/Zombie/Armless/ArmlessWalk1_blink.png", 46, 8),  // ARMLESS_ZOMBIE_WALK1_BLINK_DIRECTORY
    Image("./image/Zombie/Armless/ArmlessWalk2_blink.png", 46, 8),  // ARMLESS_ZOMBIE_WALK2_BLINK_DIRECTORY
    Image("./image/Zombie/Armless/ArmlessWalk1_cold.png", 46, 8),   // ARMLESS_ZOMBIE_WALK1_COLD_DIRECTORY
    Image("./image/Zombie/Armless/ArmlessWalk2_cold.png", 46, 8),   // ARMLESS_ZOMBIE_WALK2_COLD_DIRECTORY
    Image("./image/Zombie/Armless/ArmlessAttack.png", 39, 8),       // ARMLESS_ZOMBIE_EATING_DIRECTORY
    Image("./image/Zombie/Armless/ArmlessAttack_blink.png", 39, 8), // ARMLESS_ZOMBIE_EATING_BLINK_DIRECTORY
    Image("./image/Zombie/Armless/ArmlessAttack_cold.png", 39, 8),  // ARMLESS_ZOMBIE_EATING_COLD_DIRECTORY

    // FLAG ZOMBIE--------------------------------------------------------
    Image("./image/Zombie/Flag/Degrade1/FlagZombieWalk1_1.png", 46, 8),        // FLAG_ZOMBIE_WALK1_1_DIRECTORY
    Image("./image/Zombie/Flag/Degrade1/FlagZombieWalk2_1.png", 46, 8),        // FLAG_ZOMBIE_WALK2_1_DIRECTORY
    Image("./image/Zombie/Flag/Degrade1/FlagZombieWalk1_1_blink.png", 46, 8),  // FLAG_ZOMBIE_WALK1_1_BLINK_DIRECTORY
    Image("./image/Zombie/Flag/Degrade1/FlagZombieWalk2_1_blink.png", 46, 8),  // FLAG_ZOMBIE_WALK2_1_BLINK_DIRECTORY
    Image("./image/Zombie/Flag/Degrade1/FlagZombieWalk1_1_cold.png", 46, 8),   // FLAG_ZOMBIE_WALK1_1_COLD_DIRECTORY
    Image("./image/Zombie/Flag/Degrade1/FlagZombieWalk2_1_cold.png", 46, 8),   // FLAG_ZOMBIE_WALK2_1_COLD_DIRECTORY
    Image("./image/Zombie/Flag/Degrade1/FlagZombieAttack_1.png", 39, 8),       // FLAG_ZOMBIE_EATING_1_DIRECTORY
    Image("./image/Zombie/Flag/Degrade1/FlagZombieAttack_1_blink.png", 39, 8), // FLAG_ZOMBIE_EATING_1_BLINK_DIRECTORY
    Image("./image/Zombie/Flag/Degrade1/FlagZombieAttack_1_cold.png", 39, 8),  // FLAG_ZOMBIE_EATING_1_COLD_DIRECTORY
    Image("./image/Zombie/Flag/Degrade2/FlagZombieWalk1_2.png", 46, 8),        // FLAG_ZOMBIE_WALK1_2_DIRECTORY
    Image("./image/Zombie/Flag/Degrade2/FlagZombieWalk2_2.png", 46, 8),        // FLAG_ZOMBIE_WALK2_2_DIRECTORY
    Image("./image/Zombie/Flag/Degrade2/FlagZombieWalk1_2_blink.png", 46, 8),  // FLAG_ZOMBIE_WALK1_2_BLINK_DIRECTORY
    Image("./image/Zombie/Flag/Degrade2/FlagZombieWalk2_2_blink.png", 46, 8),  // FLAG_ZOMBIE_WALK2_2_BLINK_DIRECTORY
    Image("./image/Zombie/Flag/Degrade2/FlagZombieWalk1_2_cold.png", 46, 8),   // FLAG_ZOMBIE_WALK1_2_COLD_DIRECTORY
    Image("./image/Zombie/Flag/Degrade2/FlagZombieWalk2_2_cold.png", 46, 8),   // FLAG_ZOMBIE_WALK2_2_COLD_DIRECTORY
    Image("./image/Zombie/Flag/Degrade2/FlagZombieAttack_2.png", 39, 8),       // FLAG_ZOMBIE_EATING_2_DIRECTORY
    Image("./image/Zombie/Flag/Degrade2/FlagZombieAttack_2_blink.png", 39, 8), // FLAG_ZOMBIE_EATING_2_BLINK_DIRECTORY
    Image("./image/Zombie/Flag/Degrade2/FlagZombieAttack_2_cold.png", 39, 8),  // FLAG_ZOMBIE_EATING_2_COLD_DIRECTORY
    Image("./image/Zombie/Flag/Degrade3/FlagZombieWalk1_3.png", 46, 8),        // FLAG_ZOMBIE_WALK1_3_DIRECTORY
    Image("./image/Zombie/Flag/Degrade3/FlagZombieWalk2_3.png", 46, 8),        // FLAG_ZOMBIE_WALK2_3_DIRECTORY
    Image("./image/Zombie/Flag/Degrade3/FlagZombieWalk1_3_blink.png", 46, 8),  // FLAG_ZOMBIE_WALK1_3_BLINK_DIRECTORY
    Image("./image/Zombie/Flag/Degrade3/FlagZombieWalk2_3_blink.png", 46, 8),  // FLAG_ZOMBIE_WALK2_3_BLINK_DIRECTORY
    Image("./image/Zombie/Flag/Degrade3/FlagZombieWalk1_3_cold.png", 46, 8),   // FLAG_ZOMBIE_WALK1_3_COLD_DIRECTORY
    Image("./image/Zombie/Flag/Degrade3/FlagZombieWalk2_3_cold.png", 46, 8),   // FLAG_ZOMBIE_WALK2_3_COLD_DIRECTORY
    Image("./image/Zombie/Flag/Degrade3/FlagZombieAttack_3.png", 39, 8),       // FLAG_ZOMBIE_EATING_3_DIRECTORY
    Image("./image/Zombie/Flag/Degrade3/FlagZombieAttack_3_blink.png", 39, 8), // FLAG_ZOMBIE_EATING_3_BLINK_DIRECTORY
    Image("./image/Zombie/Flag/Degrade3/FlagZombieAttack_3_cold.png", 39, 8),  // FLAG_ZOMBIE_EATING_3_COLD_DIRECTORY

    // CONEHEAD ZOMBIE------------------------------------------------------------------------
    Image("./image/Zombie/Cone/cone_drop.png", 8, 8),                          // CONE_DROP_DIRECTORY
    Image("./image/Zombie/Cone/ConeZombie_credit1.png", 22, 8),                // CONE_ZOMBIE_CREDIT1_DIRECTORY
    Image("./image/Zombie/Cone/ConeZombie_credit2.png", 22, 8),                // CONE_ZOMBIE_CREDIT2_DIRECTORY
    Image("./image/Zombie/Cone/Degrade1/ConeZombieWalk1_1.png", 46, 8),        // CONE_ZOMBIE_WALK1_1_DIRECTORY
    Image("./image/Zombie/Cone/Degrade1/ConeZombieWalk2_1.png", 46, 8),        // CONE_ZOMBIE_WALK2_1_DIRECTORY
    Image("./image/Zombie/Cone/Degrade1/ConeZombieWalk1_1_blink.png", 46, 8),  // CONE_ZOMBIE_WALK1_1_BLINK_DIRECTORY
    Image("./image/Zombie/Cone/Degrade1/ConeZombieWalk2_1_blink.png", 46, 8),  // CONE_ZOMBIE_WALK2_1_BLINK_DIRECTORY
    Image("./image/Zombie/Cone/Degrade1/ConeZombieWalk1_1_cold.png", 46, 8),   // CONE_ZOMBIE_WALK1_1_COLD_DIRECTORY
    Image("./image/Zombie/Cone/Degrade1/ConeZombieWalk2_1_cold.png", 46, 8),   // CONE_ZOMBIE_WALK2_1_COLD_DIRECTORY
    Image("./image/Zombie/Cone/Degrade1/ConeZombieAttack_1.png", 39, 8),       // CONE_ZOMBIE_EATING_1_DIRECTORY
    Image("./image/Zombie/Cone/Degrade1/ConeZombieAttack_1_blink.png", 39, 8), // CONE_ZOMBIE_EATING_1_BLINK_DIRECTORY
    Image("./image/Zombie/Cone/Degrade1/ConeZombieAttack_1_cold.png", 39, 8),  // CONE_ZOMBIE_EATING_1_COLD_DIRECTORY
    Image("./image/Zombie/Cone/Degrade2/ConeZombieWalk1_2.png", 46, 8),        // CONE_ZOMBIE_WALK1_2_DIRECTORY
    Image("./image/Zombie/Cone/Degrade2/ConeZombieWalk2_2.png", 46, 8),        // CONE_ZOMBIE_WALK2_2_DIRECTORY
    Image("./image/Zombie/Cone/Degrade2/ConeZombieWalk1_2_blink.png", 46, 8),  // CONE_ZOMBIE_WALK1_2_BLINK_DIRECTORY
    Image("./image/Zombie/Cone/Degrade2/ConeZombieWalk2_2_blink.png", 46, 8),  // CONE_ZOMBIE_WALK2_2_BLINK_DIRECTORY
    Image("./image/Zombie/Cone/Degrade2/ConeZombieWalk1_2_cold.png", 46, 8),   // CONE_ZOMBIE_WALK1_2_COLD_DIRECTORY
    Image("./image/Zombie/Cone/Degrade2/ConeZombieWalk2_2_cold.png", 46, 8),   // CONE_ZOMBIE_WALK2_2_COLD_DIRECTORY
    Image("./image/Zombie/Cone/Degrade2/ConeZombieAttack_2.png", 39, 8),       // CONE_ZOMBIE_EATING_2_DIRECTORY
    Image("./image/Zombie/Cone/Degrade2/ConeZombieAttack_2_blink.png", 39, 8), // CONE_ZOMBIE_EATING_2_BLINK_DIRECTORY
    Image("./image/Zombie/Cone/Degrade2/ConeZombieAttack_2_cold.png", 39, 8),  // CONE_ZOMBIE_EATING_2_COLD_DIRECTORY
    Image("./image/Zombie/Cone/Degrade3/ConeZombieWalk1_3.png", 46, 8),        // CONE_ZOMBIE_WALK1_3_DIRECTORY
    Image("./image/Zombie/Cone/Degrade3/ConeZombieWalk2_3.png", 46, 8),        // CONE_ZOMBIE_WALK2_3_DIRECTORY
    Image("./image/Zombie/Cone/Degrade3/ConeZombieWalk1_3_blink.png", 46, 8),  // CONE_ZOMBIE_WALK1_3_BLINK_DIRECTORY
    Image("./image/Zombie/Cone/Degrade3/ConeZombieWalk2_3_blink.png", 46, 8),  // CONE_ZOMBIE_WALK2_3_BLINK_DIRECTORY
    Image("./image/Zombie/Cone/Degrade3/ConeZombieWalk1_3_cold.png", 46, 8),   // CONE_ZOMBIE_WALK1_3_COLD_DIRECTORY
    Image("./image/Zombie/Cone/Degrade3/ConeZombieWalk2_3_cold.png", 46, 8),   // CONE_ZOMBIE_WALK2_3_COLD_DIRECTORY
    Image("./image/Zombie/Cone/Degrade3/ConeZombieAttack_3.png", 39, 8),       // CONE_ZOMBIE_EATING_3_DIRECTORY
    Image("./image/Zombie/Cone/Degrade3/ConeZombieAttack_3_blink.png", 39, 8), // CONE_ZOMBIE_EATING_3_BLINK_DIRECTORY
    Image("./image/Zombie/Cone/Degrade3/ConeZombieAttack_3_cold.png", 39, 8),  // CONE_ZOMBIE_EATING_3_COLD_DIRECTORY

    // BUCKETHEAD ZOMBIE------------------------------------------------------------------------
    Image("./image/Zombie/Bucket/bucket_drop.png", 8, 8),                          // BUCKET_DROP_DIRECTORY
    Image("./image/Zombie/Bucket/BucketZombie_credit1.png", 22, 8),                // BUCKET_ZOMBIE_CREDIT1_DIRECTORY
    Image("./image/Zombie/Bucket/BucketZombie_credit2.png", 22, 8),                // BUCKET_ZOMBIE_CREDIT2_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade1/BucketZombieWalk1_1.png", 46, 8),        // BUCKET_ZOMBIE_WALK1_1_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade1/BucketZombieWalk2_1.png", 46, 8),        // BUCKET_ZOMBIE_WALK2_1_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade1/BucketZombieWalk1_1_blink.png", 46, 8),  // BUCKET_ZOMBIE_WALK1_1_BLINK_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade1/BucketZombieWalk2_1_blink.png", 46, 8),  // BUCKET_ZOMBIE_WALK2_1_BLINK_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade1/BucketZombieWalk1_1_cold.png", 46, 8),   // BUCKET_ZOMBIE_WALK1_1_COLD_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade1/BucketZombieWalk2_1_cold.png", 46, 8),   // BUCKET_ZOMBIE_WALK2_1_COLD_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade1/BucketZombieAttack_1.png", 39, 8),       // BUCKET_ZOMBIE_EATING_1_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade1/BucketZombieAttack_1_blink.png", 39, 8), // BUCKET_ZOMBIE_EATING_1_BLINK_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade1/BucketZombieAttack_1_cold.png", 39, 8),  // BUCKET_ZOMBIE_EATING_1_COLD_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade2/BucketZombieWalk1_2.png", 46, 8),        // BUCKET_ZOMBIE_WALK1_2_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade2/BucketZombieWalk2_2.png", 46, 8),        // BUCKET_ZOMBIE_WALK2_2_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade2/BucketZombieWalk1_2_blink.png", 46, 8),  // BUCKET_ZOMBIE_WALK1_2_BLINK_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade2/BucketZombieWalk2_2_blink.png", 46, 8),  // BUCKET_ZOMBIE_WALK2_2_BLINK_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade2/BucketZombieWalk1_2_cold.png", 46, 8),   // BUCKET_ZOMBIE_WALK1_2_COLD_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade2/BucketZombieWalk2_2_cold.png", 46, 8),   // BUCKET_ZOMBIE_WALK2_2_COLD_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade2/BucketZombieAttack_2.png", 39, 8),       // BUCKET_ZOMBIE_EATING_2_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade2/BucketZombieAttack_2_blink.png", 39, 8), // BUCKET_ZOMBIE_EATING_2_BLINK_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade2/BucketZombieAttack_2_cold.png", 39, 8),  // BUCKET_ZOMBIE_EATING_2_COLD_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade3/BucketZombieWalk1_3.png", 46, 8),        // BUCKET_ZOMBIE_WALK1_3_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade3/BucketZombieWalk2_3.png", 46, 8),        // BUCKET_ZOMBIE_WALK2_3_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade3/BucketZombieWalk1_3_blink.png", 46, 8),  // BUCKET_ZOMBIE_WALK1_3_BLINK_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade3/BucketZombieWalk2_3_blink.png", 46, 8),  // BUCKET_ZOMBIE_WALK2_3_BLINK_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade3/BucketZombieWalk1_3_cold.png", 46, 8),   // BUCKET_ZOMBIE_WALK1_3_COLD_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade3/BucketZombieWalk2_3_cold.png", 46, 8),   // BUCKET_ZOMBIE_WALK2_3_COLD_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade3/BucketZombieAttack_3.png", 39, 8),       // BUCKET_ZOMBIE_EATING_3_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade3/BucketZombieAttack_3_blink.png", 39, 8), // BUCKET_ZOMBIE_EATING_3_BLINK_DIRECTORY
    Image("./image/Zombie/Bucket/Degrade3/BucketZombieAttack_3_cold.png", 39, 8),  // BUCKET_ZOMBIE_EATING_3_COLD_DIRECTORY

    // DOOR ZOMBIE------------------------------------------------------------------------
    Image("./image/Zombie/Door/DoorZombie_credit1.png", 22, 8),                // DOOR_ZOMBIE_CREDIT1_DIRECTORY
    Image("./image/Zombie/Door/DoorZombie_credit1.png", 22, 8),                // DOOR_ZOMBIE_CREDIT2_DIRECTORY
    Image("./image/Zombie/Door/Degrade1/DoorZombieWalk1_1.png", 46, 8),        // DOOR_ZOMBIE_WALK1_1_DIRECTORY
    Image("./image/Zombie/Door/Degrade1/DoorZombieWalk2_1.png", 46, 8),        // DOOR_ZOMBIE_WALK2_1_DIRECTORY
    Image("./image/Zombie/Door/Degrade1/DoorZombieWalk1_1_blink.png", 46, 8),  // DOOR_ZOMBIE_WALK1_1_BLINK_DIRECTORY
    Image("./image/Zombie/Door/Degrade1/DoorZombieWalk2_1_blink.png", 46, 8),  // DOOR_ZOMBIE_WALK2_1_BLINK_DIRECTORY
    Image("./image/Zombie/Door/Degrade1/DoorZombieWalk1_1_cold.png", 46, 8),   // DOOR_ZOMBIE_WALK1_1_COLD_DIRECTORY
    Image("./image/Zombie/Door/Degrade1/DoorZombieWalk2_1_cold.png", 46, 8),   // DOOR_ZOMBIE_WALK2_1_COLD_DIRECTORY
    Image("./image/Zombie/Door/Degrade1/DoorZombieAttack_1.png", 39, 8),       // DOOR_ZOMBIE_EATING_1_DIRECTORY
    Image("./image/Zombie/Door/Degrade1/DoorZombieAttack_1_blink.png", 39, 8), // DOOR_ZOMBIE_EATING_1_BLINK_DIRECTORY
    Image("./image/Zombie/Door/Degrade1/DoorZombieAttack_1_cold.png", 39, 8),  // DOOR_ZOMBIE_EATING_1_COLD_DIRECTORY
    Image("./image/Zombie/Door/Degrade2/DoorZombieWalk1_2.png", 46, 8),        // DOOR_ZOMBIE_WALK1_2_DIRECTORY
    Image("./image/Zombie/Door/Degrade2/DoorZombieWalk2_2.png", 46, 8),        // DOOR_ZOMBIE_WALK2_2_DIRECTORY
    Image("./image/Zombie/Door/Degrade2/DoorZombieWalk1_2_blink.png", 46, 8),  // DOOR_ZOMBIE_WALK1_2_BLINK_DIRECTORY
    Image("./image/Zombie/Door/Degrade2/DoorZombieWalk2_2_blink.png", 46, 8),  // DOOR_ZOMBIE_WALK2_2_BLINK_DIRECTORY
    Image("./image/Zombie/Door/Degrade2/DoorZombieWalk1_2_cold.png", 46, 8),   // DOOR_ZOMBIE_WALK1_2_COLD_DIRECTORY
    Image("./image/Zombie/Door/Degrade2/DoorZombieWalk2_2_cold.png", 46, 8),   // DOOR_ZOMBIE_WALK2_2_COLD_DIRECTORY
    Image("./image/Zombie/Door/Degrade2/DoorZombieAttack_2.png", 39, 8),       // DOOR_ZOMBIE_EATING_2_DIRECTORY
    Image("./image/Zombie/Door/Degrade2/DoorZombieAttack_2_blink.png", 39, 8), // DOOR_ZOMBIE_EATING_2_BLINK_DIRECTORY
    Image("./image/Zombie/Door/Degrade2/DoorZombieAttack_2_cold.png", 39, 8),  // DOOR_ZOMBIE_EATING_2_COLD_DIRECTORY
    Image("./image/Zombie/Door/Degrade3/DoorZombieWalk1_3.png", 46, 8),        // DOOR_ZOMBIE_WALK1_3_DIRECTORY
    Image("./image/Zombie/Door/Degrade3/DoorZombieWalk2_3.png", 46, 8),        // DOOR_ZOMBIE_WALK2_3_DIRECTORY
    Image("./image/Zombie/Door/Degrade3/DoorZombieWalk1_3_blink.png", 46, 8),  // DOOR_ZOMBIE_WALK1_3_BLINK_DIRECTORY
    Image("./image/Zombie/Door/Degrade3/DoorZombieWalk2_3_blink.png", 46, 8),  // DOOR_ZOMBIE_WALK2_3_BLINK_DIRECTORY
    Image("./image/Zombie/Door/Degrade3/DoorZombieWalk1_3_cold.png", 46, 8),   // DOOR_ZOMBIE_WALK1_3_COLD_DIRECTORY
    Image("./image/Zombie/Door/Degrade3/DoorZombieWalk2_3_cold.png", 46, 8),   // DOOR_ZOMBIE_WALK2_3_COLD_DIRECTORY
    Image("./image/Zombie/Door/Degrade3/DoorZombieAttack_3.png", 39, 8),       // DOOR_ZOMBIE_EATING_3_DIRECTORY
    Image("./image/Zombie/Door/Degrade3/DoorZombieAttack_3_blink.png", 39, 8), // DOOR_ZOMBIE_EATING_3_BLINK_DIRECTORY
    Image("./image/Zombie/Door/Degrade3/DoorZombieAttack_3_cold.png", 39, 8),  // DOOR_ZOMBIE_EATING_3_COLD_DIRECTORY

    //----------------------------------------------------------------------------------------------
    Image("./image/Announcement/StartReady.png"), // START_READY_DIRECTORY
    Image("./image/Announcement/StartSet.png"),   // START_SET_DIRECTORY
    Image("./image/Announcement/StartPlant.png"), // START_PLANT_DIRECTORY
    Image("./image/Announcement/NextWave.png"),   // HUGE_WAVE_DIRECTORY
    Image("./image/Announcement/FinalWave.png"),  // FINAL_WAVE_DIRECTORY
    Image("./image/Black_Screen.png"),            // BLACK_SCREEN_DIRECTORY
    Image("./image/White_Screen.png"),            // WHITE_SCREEN_DIRECTORY

    Image("./image/Background/yard_blink/Frontyardc1.png"), // FRONTYARD_C1
    Image("./image/Background/yard_blink/Frontyardc2.png"), // FRONTYARD_C2
    Image("./image/Background/yard_blink/Frontyardc3.png"), // FRONTYARD_C3
    Image("./image/Background/yard_blink/Frontyardc4.png"), // FRONTYARD_C4
    Image("./image/Background/yard_blink/Frontyardc5.png"), // FRONTYARD_C5
    Image("./image/Background/yard_blink/Frontyardc6.png"), // FRONTYARD_C6
    Image("./image/Background/yard_blink/Frontyardc7.png"), // FRONTYARD_C7
    Image("./image/Background/yard_blink/Frontyardc8.png"), // FRONTYARD_C8
    Image("./image/Background/yard_blink/Frontyardc9.png"), // FRONTYARD_C9
    Image("./image/Background/yard_blink/Frontyardr1.png"), // FRONTYARD_R1
    Image("./image/Background/yard_blink/Frontyardr2.png"), // FRONTYARD_R2
    Image("./image/Background/yard_blink/Frontyardr3.png"), // FRONTYARD_R3
    Image("./image/Background/yard_blink/Frontyardr4.png"), // FRONTYARD_R4
    Image("./image/Background/yard_blink/Frontyardr5.png"), // FRONTYARD_R5

    Image("./image/Background/win/win_e_00.png"), // WIN_00_DIRECTORY
    Image("./image/Background/win/win_e_01.png"), // WIN_01_DIRECTORY
    Image("./image/Background/win/win_e_02.png"), // WIN_02_DIRECTORY
    Image("./image/Background/win/win_e_03.png"), // WIN_03_DIRECTORY
    Image("./image/Background/win/win_e_04.png"), // WIN_04_DIRECTORY
    Image("./image/Background/win/win_e_05.png"), // WIN_05_DIRECTORY
    Image("./image/Background/win/win_e_06.png"), // WIN_06_DIRECTORY
    Image("./image/Background/win/win_e_07.png"), // WIN_07_DIRECTORY
    Image("./image/Background/win/win_e_08.png"), // WIN_08_DIRECTORY
    Image("./image/Background/win/win_e_09.png"), // WIN_09_DIRECTORY
    Image("./image/Background/win/win_e_10.png"), // WIN_10_DIRECTORY
    Image("./image/Background/win/win_e_11.png"), // WIN_11_DIRECTORY
    Image("./image/Background/win/win_e_12.png"), // WIN_12_DIRECTORY
    Image("./image/Background/win/win_e_13.png"), // WIN_13_DIRECTORY
    Image("./image/Background/win/win_e_14.png"), // WIN_14_DIRECTORY
    Image("./image/Background/win/win_e_15.png"), // WIN_15_DIRECTORY
    Image("./image/Background/win/win_e_16.png"), // WIN_16_DIRECTORY
    Image("./image/Background/win/win_e_17.png"), // WIN_17_DIRECTORY
    Image("./image/Background/win/win_e_18.png"), // WIN_18_DIRECTORY
    Image("./image/Background/win/win_e_19.png"), // WIN_19_DIRECTORY
    Image("./image/Background/win/win_e_20.png"), // WIN_20_DIRECTORY
    Image("./image/Background/win/win_e_21.png"), // WIN_21_DIRECTORY
    Image("./image/Background/win/win_e_22.png"), // WIN_22_DIRECTORY
    Image("./image/Background/win/win_e_23.png"), // WIN_23_DIRECTORY
    Image("./image/Background/win/win_e_24.png"), // WIN_24_DIRECTORY
    Image("./image/Background/win/win_e_25.png"), // WIN_25_DIRECTORY
    Image("./image/Background/win/win_e_26.png"), // WIN_26_DIRECTORY
    Image("./image/Background/win/win_e_27.png"), // WIN_27_DIRECTORY
    Image("./image/Background/win/win_e_28.png"), // WIN_28_DIRECTORY
    Image("./image/Background/win/win_e_29.png"), // WIN_29_DIRECTORY
    Image("./image/Background/win/win_e_30.png"), // WIN_30_DIRECTORY
    Image("./image/Background/win/win_e_31.png"), // WIN_31_DIRECTORY
    Image("./image/Background/win/win_e_32.png"), // WIN_32_DIRECTORY
    Image("./image/Background/win/win_e_33.png"), // WIN_33_DIRECTORY
    Image("./image/Background/win/win_e_34.png"), // WIN_34_DIRECTORY
    Image("./image/Background/win/win_e_35.png"), // WIN_35_DIRECTORY
    Image("./image/Background/win/win_e_36.png"), // WIN_36_DIRECTORY
    Image("./image/Background/win/win_e_37.png"), // WIN_37_DIRECTORY
    Image("./image/Background/win/win_e_38.png"), // WIN_38_DIRECTORY
    Image("./image/Background/win/win_e_39.png"), // WIN_39_DIRECTORY
    Image("./image/Background/win/win_e_40.png"), // WIN_40_DIRECTORY
    Image("./image/Background/win/win_e_41.png"), // WIN_41_DIRECTORY
    Image("./image/Background/win/win_e_42.png"), // WIN_42_DIRECTORY
    Image("./image/Background/win/win_e_43.png"), // WIN_43_DIRECTORY
    Image("./image/Background/win/win_e_44.png"), // WIN_44_DIRECTORY
    Image("./image/Background/win/win_e_45.png"), // WIN_45_DIRECTORY
    Image("./image/Background/win/win_e_46.png"), // WIN_46_DIRECTORY
    Image("./image/Background/win/win_e_47.png"), // WIN_47_DIRECTORY
    Image("./image/Background/win/win_e_48.png"), // WIN_48_DIRECTORY
    Image("./image/Background/win/win_e_49.png"), // WIN_49_DIRECTORY
    Image("./image/Background/win/win_e_50.png"), // WIN_50_DIRECTORY
    Image("./image/Background/win/win_e_51.png"), // WIN_51_DIRECTORY
    Image("./image/Background/win/win_e_52.png"), // WIN_52_DIRECTORY
    Image("./image/Background/win/win_e_53.png"), // WIN_53_DIRECTORY
    Image("./image/Background/win/win_e_54.png"), // WIN_54_DIRECTORY
    Image("./image/Background/win/win_e_55.png"), // WIN_55_DIRECTORY
    Image("./image/Background/win/win_e_56.png"), // WIN_56_DIRECTORY
    Image("./image/Background/win/win_e_57.png"), // WIN_57_DIRECTORY
    Image("./image/Background/win/win_e_58.png"), // WIN_58_DIRECTORY
    Image("./image/Background/win/win_e_59.png"), // WIN_59_DIRECTORY
    Image("./image/Background/win/win_e_60.png"), // WIN_60_DIRECTORY
    Image("./image/Background/win/win_e_61.png"), // WIN_61_DIRECTORY
    Image("./image/Background/win/win_e_62.png"), // WIN_62_DIRECTORY
    Image("./image/Background/win/win_e_63.png"), // WIN_63_DIRECTORY
    Image("./image/Background/win/win_e_64.png"), // WIN_64_DIRECTORY
    Image("./image/Background/win/win_e_65.png"), // WIN_65_DIRECTORY
    Image("./image/Background/win/win_e_66.png"), // WIN_66_DIRECTORY
    Image("./image/Background/win/win_e_67.png"), // WIN_67_DIRECTORY
    Image("./image/Background/win/win_e_68.png"), // WIN_68_DIRECTORY
    Image("./image/Background/win/win_e_69.png"), // WIN_69_DIRECTORY
    Image("./image/Background/win/win_e_70.png"), // WIN_70_DIRECTORY
    Image("./image/Background/win/win_e_71.png"), // WIN_71_DIRECTORY
    Image("./image/Background/win/win_e_72.png"), // WIN_72_DIRECTORY
    Image("./image/Background/win/win_e_73.png"), // WIN_73_DIRECTORY

    Image("./image/Icons/starter_fire_pack.png", 1, 1), // MOUSE_CURSOR_DIRECTORY
    Image("./image/Icons/mower.png", 17, 8),            // MOWER_DIRECTORY

    Image("./image/Shadow/plantshadow.png"), // SHADOW_DIRECTORY
    Image("./image/Shadow/pea_shadows.png"), // PEA_SHADOW_DIRECTORY

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
    {WALNUT_5_DIRECTORY, WALNUT_5_BLINK_DIRECTORY},

    {SNOWPEA_SHEET_DIRECTORY, SNOWPEA_SHEET_BLINK_DIRECTORY},
    {SNOWPEA_ATTACK_DIRECTORY, SNOWPEA_ATTACK_BLINK_DIRECTORY},

    {CHERRYBOMB_SHEET_DIRECTORY, CHERRYBOMB_SHEET_BLINK_DIRECTORY},

    {ZOMBIE_WALK1_DIRECTORY, ZOMBIE_WALK1_BLINK_DIRECTORY},
    {ZOMBIE_WALK2_DIRECTORY, ZOMBIE_WALK2_BLINK_DIRECTORY},
    {ZOMBIE_EATING_DIRECTORY, ZOMBIE_EATING_BLINK_DIRECTORY},

    {ARMLESS_ZOMBIE_WALK1_DIRECTORY, ARMLESS_ZOMBIE_WALK1_BLINK_DIRECTORY},
    {ARMLESS_ZOMBIE_WALK2_DIRECTORY, ARMLESS_ZOMBIE_WALK2_BLINK_DIRECTORY},
    {ARMLESS_ZOMBIE_EATING_DIRECTORY, ARMLESS_ZOMBIE_EATING_BLINK_DIRECTORY},

    {FLAG_ZOMBIE_WALK1_1_DIRECTORY, FLAG_ZOMBIE_WALK1_1_BLINK_DIRECTORY},
    {FLAG_ZOMBIE_WALK1_2_DIRECTORY, FLAG_ZOMBIE_WALK1_2_BLINK_DIRECTORY},
    {FLAG_ZOMBIE_WALK1_3_DIRECTORY, FLAG_ZOMBIE_WALK1_3_BLINK_DIRECTORY},
    {FLAG_ZOMBIE_WALK2_1_DIRECTORY, FLAG_ZOMBIE_WALK2_1_BLINK_DIRECTORY},
    {FLAG_ZOMBIE_WALK2_2_DIRECTORY, FLAG_ZOMBIE_WALK2_2_BLINK_DIRECTORY},
    {FLAG_ZOMBIE_WALK2_3_DIRECTORY, FLAG_ZOMBIE_WALK2_3_BLINK_DIRECTORY},
    {FLAG_ZOMBIE_EATING_1_DIRECTORY, FLAG_ZOMBIE_EATING_1_BLINK_DIRECTORY},
    {FLAG_ZOMBIE_EATING_2_DIRECTORY, FLAG_ZOMBIE_EATING_2_BLINK_DIRECTORY},
    {FLAG_ZOMBIE_EATING_3_DIRECTORY, FLAG_ZOMBIE_EATING_3_BLINK_DIRECTORY},

    {CONE_ZOMBIE_WALK1_1_DIRECTORY, CONE_ZOMBIE_WALK1_1_BLINK_DIRECTORY},
    {CONE_ZOMBIE_WALK1_2_DIRECTORY, CONE_ZOMBIE_WALK1_2_BLINK_DIRECTORY},
    {CONE_ZOMBIE_WALK1_3_DIRECTORY, CONE_ZOMBIE_WALK1_3_BLINK_DIRECTORY},
    {CONE_ZOMBIE_WALK2_1_DIRECTORY, CONE_ZOMBIE_WALK2_1_BLINK_DIRECTORY},
    {CONE_ZOMBIE_WALK2_2_DIRECTORY, CONE_ZOMBIE_WALK2_2_BLINK_DIRECTORY},
    {CONE_ZOMBIE_WALK2_3_DIRECTORY, CONE_ZOMBIE_WALK2_3_BLINK_DIRECTORY},
    {CONE_ZOMBIE_EATING_1_DIRECTORY, CONE_ZOMBIE_EATING_1_BLINK_DIRECTORY},
    {CONE_ZOMBIE_EATING_2_DIRECTORY, CONE_ZOMBIE_EATING_2_BLINK_DIRECTORY},
    {CONE_ZOMBIE_EATING_3_DIRECTORY, CONE_ZOMBIE_EATING_3_BLINK_DIRECTORY},

    {BUCKET_ZOMBIE_WALK1_1_DIRECTORY, BUCKET_ZOMBIE_WALK1_1_BLINK_DIRECTORY},
    {BUCKET_ZOMBIE_WALK1_2_DIRECTORY, BUCKET_ZOMBIE_WALK1_2_BLINK_DIRECTORY},
    {BUCKET_ZOMBIE_WALK1_3_DIRECTORY, BUCKET_ZOMBIE_WALK1_3_BLINK_DIRECTORY},
    {BUCKET_ZOMBIE_WALK2_1_DIRECTORY, BUCKET_ZOMBIE_WALK2_1_BLINK_DIRECTORY},
    {BUCKET_ZOMBIE_WALK2_2_DIRECTORY, BUCKET_ZOMBIE_WALK2_2_BLINK_DIRECTORY},
    {BUCKET_ZOMBIE_WALK2_3_DIRECTORY, BUCKET_ZOMBIE_WALK2_3_BLINK_DIRECTORY},
    {BUCKET_ZOMBIE_EATING_1_DIRECTORY, BUCKET_ZOMBIE_EATING_1_BLINK_DIRECTORY},
    {BUCKET_ZOMBIE_EATING_2_DIRECTORY, BUCKET_ZOMBIE_EATING_2_BLINK_DIRECTORY},
    {BUCKET_ZOMBIE_EATING_3_DIRECTORY, BUCKET_ZOMBIE_EATING_3_BLINK_DIRECTORY},

    {DOOR_ZOMBIE_WALK1_1_DIRECTORY, DOOR_ZOMBIE_WALK1_1_BLINK_DIRECTORY},
    {DOOR_ZOMBIE_WALK1_2_DIRECTORY, DOOR_ZOMBIE_WALK1_2_BLINK_DIRECTORY},
    {DOOR_ZOMBIE_WALK1_3_DIRECTORY, DOOR_ZOMBIE_WALK1_3_BLINK_DIRECTORY},
    {DOOR_ZOMBIE_WALK2_1_DIRECTORY, DOOR_ZOMBIE_WALK2_1_BLINK_DIRECTORY},
    {DOOR_ZOMBIE_WALK2_2_DIRECTORY, DOOR_ZOMBIE_WALK2_2_BLINK_DIRECTORY},
    {DOOR_ZOMBIE_WALK2_3_DIRECTORY, DOOR_ZOMBIE_WALK2_3_BLINK_DIRECTORY},
    {DOOR_ZOMBIE_EATING_1_DIRECTORY, DOOR_ZOMBIE_EATING_1_BLINK_DIRECTORY},
    {DOOR_ZOMBIE_EATING_2_DIRECTORY, DOOR_ZOMBIE_EATING_2_BLINK_DIRECTORY},
    {DOOR_ZOMBIE_EATING_3_DIRECTORY, DOOR_ZOMBIE_EATING_3_BLINK_DIRECTORY}

};

/*
Corresponding cold of image
*/
static map<int, int> cold_of = {

    {ZOMBIE_WALK1_DIRECTORY, ZOMBIE_WALK1_COLD_DIRECTORY},
    {ZOMBIE_WALK2_DIRECTORY, ZOMBIE_WALK2_COLD_DIRECTORY},
    {ZOMBIE_EATING_DIRECTORY, ZOMBIE_EATING_COLD_DIRECTORY},

    {ZOMBIE_ARM_DIRECTORY, ZOMBIE_ARM_COLD_DIRECTORY},
    {ARMLESS_ZOMBIE_WALK1_DIRECTORY, ARMLESS_ZOMBIE_WALK1_COLD_DIRECTORY},
    {ARMLESS_ZOMBIE_WALK2_DIRECTORY, ARMLESS_ZOMBIE_WALK2_COLD_DIRECTORY},
    {ARMLESS_ZOMBIE_EATING_DIRECTORY, ARMLESS_ZOMBIE_EATING_COLD_DIRECTORY},

    {FLAG_ZOMBIE_WALK1_1_DIRECTORY, FLAG_ZOMBIE_WALK1_1_COLD_DIRECTORY},
    {FLAG_ZOMBIE_WALK1_2_DIRECTORY, FLAG_ZOMBIE_WALK1_2_COLD_DIRECTORY},
    {FLAG_ZOMBIE_WALK1_3_DIRECTORY, FLAG_ZOMBIE_WALK1_3_COLD_DIRECTORY},
    {FLAG_ZOMBIE_WALK2_1_DIRECTORY, FLAG_ZOMBIE_WALK2_1_COLD_DIRECTORY},
    {FLAG_ZOMBIE_WALK2_2_DIRECTORY, FLAG_ZOMBIE_WALK2_2_COLD_DIRECTORY},
    {FLAG_ZOMBIE_WALK2_3_DIRECTORY, FLAG_ZOMBIE_WALK2_3_COLD_DIRECTORY},
    {FLAG_ZOMBIE_EATING_1_DIRECTORY, FLAG_ZOMBIE_EATING_1_COLD_DIRECTORY},
    {FLAG_ZOMBIE_EATING_2_DIRECTORY, FLAG_ZOMBIE_EATING_2_COLD_DIRECTORY},
    {FLAG_ZOMBIE_EATING_3_DIRECTORY, FLAG_ZOMBIE_EATING_3_COLD_DIRECTORY},

    {ZOMBIE_HEAD_DIRECTORY, ZOMBIE_HEAD_COLD_DIRECTORY},
    {ZOMBIE_DIE1_DIRECTORY, ZOMBIE_DIE1_COLD_DIRECTORY},
    {ZOMBIE_DIE2_DIRECTORY, ZOMBIE_DIE2_COLD_DIRECTORY},

    {CONE_ZOMBIE_WALK1_1_DIRECTORY, CONE_ZOMBIE_WALK1_1_COLD_DIRECTORY},
    {CONE_ZOMBIE_WALK1_2_DIRECTORY, CONE_ZOMBIE_WALK1_2_COLD_DIRECTORY},
    {CONE_ZOMBIE_WALK1_3_DIRECTORY, CONE_ZOMBIE_WALK1_3_COLD_DIRECTORY},
    {CONE_ZOMBIE_WALK2_1_DIRECTORY, CONE_ZOMBIE_WALK2_1_COLD_DIRECTORY},
    {CONE_ZOMBIE_WALK2_2_DIRECTORY, CONE_ZOMBIE_WALK2_2_COLD_DIRECTORY},
    {CONE_ZOMBIE_WALK2_3_DIRECTORY, CONE_ZOMBIE_WALK2_3_COLD_DIRECTORY},
    {CONE_ZOMBIE_EATING_1_DIRECTORY, CONE_ZOMBIE_EATING_1_COLD_DIRECTORY},
    {CONE_ZOMBIE_EATING_2_DIRECTORY, CONE_ZOMBIE_EATING_2_COLD_DIRECTORY},
    {CONE_ZOMBIE_EATING_3_DIRECTORY, CONE_ZOMBIE_EATING_3_COLD_DIRECTORY},

    {BUCKET_ZOMBIE_WALK1_1_DIRECTORY, BUCKET_ZOMBIE_WALK1_1_COLD_DIRECTORY},
    {BUCKET_ZOMBIE_WALK1_2_DIRECTORY, BUCKET_ZOMBIE_WALK1_2_COLD_DIRECTORY},
    {BUCKET_ZOMBIE_WALK1_3_DIRECTORY, BUCKET_ZOMBIE_WALK1_3_COLD_DIRECTORY},
    {BUCKET_ZOMBIE_WALK2_1_DIRECTORY, BUCKET_ZOMBIE_WALK2_1_COLD_DIRECTORY},
    {BUCKET_ZOMBIE_WALK2_2_DIRECTORY, BUCKET_ZOMBIE_WALK2_2_COLD_DIRECTORY},
    {BUCKET_ZOMBIE_WALK2_3_DIRECTORY, BUCKET_ZOMBIE_WALK2_3_COLD_DIRECTORY},
    {BUCKET_ZOMBIE_EATING_1_DIRECTORY, BUCKET_ZOMBIE_EATING_1_COLD_DIRECTORY},
    {BUCKET_ZOMBIE_EATING_2_DIRECTORY, BUCKET_ZOMBIE_EATING_2_COLD_DIRECTORY},
    {BUCKET_ZOMBIE_EATING_3_DIRECTORY, BUCKET_ZOMBIE_EATING_3_COLD_DIRECTORY},

    {DOOR_ZOMBIE_WALK1_1_DIRECTORY, DOOR_ZOMBIE_WALK1_1_COLD_DIRECTORY},
    {DOOR_ZOMBIE_WALK1_2_DIRECTORY, DOOR_ZOMBIE_WALK1_2_COLD_DIRECTORY},
    {DOOR_ZOMBIE_WALK1_3_DIRECTORY, DOOR_ZOMBIE_WALK1_3_COLD_DIRECTORY},
    {DOOR_ZOMBIE_WALK2_1_DIRECTORY, DOOR_ZOMBIE_WALK2_1_COLD_DIRECTORY},
    {DOOR_ZOMBIE_WALK2_2_DIRECTORY, DOOR_ZOMBIE_WALK2_2_COLD_DIRECTORY},
    {DOOR_ZOMBIE_WALK2_3_DIRECTORY, DOOR_ZOMBIE_WALK2_3_COLD_DIRECTORY},
    {DOOR_ZOMBIE_EATING_1_DIRECTORY, DOOR_ZOMBIE_EATING_1_COLD_DIRECTORY},
    {DOOR_ZOMBIE_EATING_2_DIRECTORY, DOOR_ZOMBIE_EATING_2_COLD_DIRECTORY},
    {DOOR_ZOMBIE_EATING_3_DIRECTORY, DOOR_ZOMBIE_EATING_3_COLD_DIRECTORY}

};

/*
Corresponding eating of zombie walk image
*/
static map<int, int> eat_of = {

    {ZOMBIE_WALK1_DIRECTORY, ZOMBIE_EATING_DIRECTORY},
    {ZOMBIE_WALK2_DIRECTORY, ZOMBIE_EATING_DIRECTORY},

    {ARMLESS_ZOMBIE_WALK1_DIRECTORY, ARMLESS_ZOMBIE_EATING_DIRECTORY},
    {ARMLESS_ZOMBIE_WALK2_DIRECTORY, ARMLESS_ZOMBIE_EATING_DIRECTORY},

    {FLAG_ZOMBIE_WALK1_1_DIRECTORY, FLAG_ZOMBIE_EATING_1_DIRECTORY},
    {FLAG_ZOMBIE_WALK1_2_DIRECTORY, FLAG_ZOMBIE_EATING_2_DIRECTORY},
    {FLAG_ZOMBIE_WALK1_3_DIRECTORY, FLAG_ZOMBIE_EATING_3_DIRECTORY},
    {FLAG_ZOMBIE_WALK2_1_DIRECTORY, FLAG_ZOMBIE_EATING_1_DIRECTORY},
    {FLAG_ZOMBIE_WALK2_2_DIRECTORY, FLAG_ZOMBIE_EATING_2_DIRECTORY},
    {FLAG_ZOMBIE_WALK2_3_DIRECTORY, FLAG_ZOMBIE_EATING_3_DIRECTORY},

    {CONE_ZOMBIE_WALK1_1_DIRECTORY, CONE_ZOMBIE_EATING_1_DIRECTORY},
    {CONE_ZOMBIE_WALK1_2_DIRECTORY, CONE_ZOMBIE_EATING_2_DIRECTORY},
    {CONE_ZOMBIE_WALK1_3_DIRECTORY, CONE_ZOMBIE_EATING_3_DIRECTORY},
    {CONE_ZOMBIE_WALK2_1_DIRECTORY, CONE_ZOMBIE_EATING_1_DIRECTORY},
    {CONE_ZOMBIE_WALK2_2_DIRECTORY, CONE_ZOMBIE_EATING_2_DIRECTORY},
    {CONE_ZOMBIE_WALK2_3_DIRECTORY, CONE_ZOMBIE_EATING_3_DIRECTORY},

    {BUCKET_ZOMBIE_WALK1_1_DIRECTORY, BUCKET_ZOMBIE_EATING_1_DIRECTORY},
    {BUCKET_ZOMBIE_WALK1_2_DIRECTORY, BUCKET_ZOMBIE_EATING_2_DIRECTORY},
    {BUCKET_ZOMBIE_WALK1_3_DIRECTORY, BUCKET_ZOMBIE_EATING_3_DIRECTORY},
    {BUCKET_ZOMBIE_WALK2_1_DIRECTORY, BUCKET_ZOMBIE_EATING_1_DIRECTORY},
    {BUCKET_ZOMBIE_WALK2_2_DIRECTORY, BUCKET_ZOMBIE_EATING_2_DIRECTORY},
    {BUCKET_ZOMBIE_WALK2_3_DIRECTORY, BUCKET_ZOMBIE_EATING_3_DIRECTORY},

    {DOOR_ZOMBIE_WALK1_1_DIRECTORY, DOOR_ZOMBIE_EATING_1_DIRECTORY},
    {DOOR_ZOMBIE_WALK1_2_DIRECTORY, DOOR_ZOMBIE_EATING_2_DIRECTORY},
    {DOOR_ZOMBIE_WALK1_3_DIRECTORY, DOOR_ZOMBIE_EATING_3_DIRECTORY},
    {DOOR_ZOMBIE_WALK2_1_DIRECTORY, DOOR_ZOMBIE_EATING_1_DIRECTORY},
    {DOOR_ZOMBIE_WALK2_2_DIRECTORY, DOOR_ZOMBIE_EATING_2_DIRECTORY},
    {DOOR_ZOMBIE_WALK2_3_DIRECTORY, DOOR_ZOMBIE_EATING_3_DIRECTORY}

};

/*
Corresponding walking of zombie eat image
*/
int walk_of(const int &img_dir);

/*
Corresponding walking of zombie eat image
*/
static map<int, int> degrade_of = {
    {ZOMBIE_WALK1_DIRECTORY, ARMLESS_ZOMBIE_WALK1_DIRECTORY},
    {ZOMBIE_WALK2_DIRECTORY, ARMLESS_ZOMBIE_WALK2_DIRECTORY},
    {ZOMBIE_EATING_DIRECTORY, ARMLESS_ZOMBIE_EATING_DIRECTORY},

    {FLAG_ZOMBIE_WALK1_1_DIRECTORY, FLAG_ZOMBIE_WALK1_2_DIRECTORY},
    {FLAG_ZOMBIE_WALK2_1_DIRECTORY, FLAG_ZOMBIE_WALK2_2_DIRECTORY},
    {FLAG_ZOMBIE_EATING_1_DIRECTORY, FLAG_ZOMBIE_EATING_2_DIRECTORY},
    {FLAG_ZOMBIE_WALK1_2_DIRECTORY, FLAG_ZOMBIE_WALK1_3_DIRECTORY},
    {FLAG_ZOMBIE_WALK2_2_DIRECTORY, FLAG_ZOMBIE_WALK2_3_DIRECTORY},
    {FLAG_ZOMBIE_EATING_2_DIRECTORY, FLAG_ZOMBIE_EATING_3_DIRECTORY},
    {FLAG_ZOMBIE_WALK1_3_DIRECTORY, ARMLESS_ZOMBIE_WALK1_DIRECTORY},
    {FLAG_ZOMBIE_WALK2_3_DIRECTORY, ARMLESS_ZOMBIE_WALK2_DIRECTORY},
    {FLAG_ZOMBIE_EATING_3_DIRECTORY, ARMLESS_ZOMBIE_EATING_DIRECTORY},

    {CONE_ZOMBIE_WALK1_1_DIRECTORY, CONE_ZOMBIE_WALK1_2_DIRECTORY},
    {CONE_ZOMBIE_WALK2_1_DIRECTORY, CONE_ZOMBIE_WALK2_2_DIRECTORY},
    {CONE_ZOMBIE_EATING_1_DIRECTORY, CONE_ZOMBIE_EATING_2_DIRECTORY},
    {CONE_ZOMBIE_WALK1_2_DIRECTORY, CONE_ZOMBIE_WALK1_3_DIRECTORY},
    {CONE_ZOMBIE_WALK2_2_DIRECTORY, CONE_ZOMBIE_WALK2_3_DIRECTORY},
    {CONE_ZOMBIE_EATING_2_DIRECTORY, CONE_ZOMBIE_EATING_3_DIRECTORY},
    {CONE_ZOMBIE_WALK1_3_DIRECTORY, ZOMBIE_WALK1_DIRECTORY},
    {CONE_ZOMBIE_WALK2_3_DIRECTORY, ZOMBIE_WALK2_DIRECTORY},
    {CONE_ZOMBIE_EATING_3_DIRECTORY, ZOMBIE_EATING_DIRECTORY},

    {BUCKET_ZOMBIE_WALK1_1_DIRECTORY, BUCKET_ZOMBIE_WALK1_2_DIRECTORY},
    {BUCKET_ZOMBIE_WALK2_1_DIRECTORY, BUCKET_ZOMBIE_WALK2_2_DIRECTORY},
    {BUCKET_ZOMBIE_EATING_1_DIRECTORY, BUCKET_ZOMBIE_EATING_2_DIRECTORY},
    {BUCKET_ZOMBIE_WALK1_2_DIRECTORY, BUCKET_ZOMBIE_WALK1_3_DIRECTORY},
    {BUCKET_ZOMBIE_WALK2_2_DIRECTORY, BUCKET_ZOMBIE_WALK2_3_DIRECTORY},
    {BUCKET_ZOMBIE_EATING_2_DIRECTORY, BUCKET_ZOMBIE_EATING_3_DIRECTORY},
    {BUCKET_ZOMBIE_WALK1_3_DIRECTORY, ZOMBIE_WALK1_DIRECTORY},
    {BUCKET_ZOMBIE_WALK2_3_DIRECTORY, ZOMBIE_WALK2_DIRECTORY},
    {BUCKET_ZOMBIE_EATING_3_DIRECTORY, ZOMBIE_EATING_DIRECTORY},

    {DOOR_ZOMBIE_WALK1_1_DIRECTORY, DOOR_ZOMBIE_WALK1_2_DIRECTORY},
    {DOOR_ZOMBIE_WALK2_1_DIRECTORY, DOOR_ZOMBIE_WALK2_2_DIRECTORY},
    {DOOR_ZOMBIE_EATING_1_DIRECTORY, DOOR_ZOMBIE_EATING_2_DIRECTORY},
    {DOOR_ZOMBIE_WALK1_2_DIRECTORY, DOOR_ZOMBIE_WALK1_3_DIRECTORY},
    {DOOR_ZOMBIE_WALK2_2_DIRECTORY, DOOR_ZOMBIE_WALK2_3_DIRECTORY},
    {DOOR_ZOMBIE_EATING_2_DIRECTORY, DOOR_ZOMBIE_EATING_3_DIRECTORY},
    {DOOR_ZOMBIE_WALK1_3_DIRECTORY, ZOMBIE_WALK1_DIRECTORY},
    {DOOR_ZOMBIE_WALK2_3_DIRECTORY, ZOMBIE_WALK2_DIRECTORY},
    {DOOR_ZOMBIE_EATING_3_DIRECTORY, ZOMBIE_EATING_DIRECTORY}

};

const int front_yard_r = FRONTYARD_R1;
const int front_yard_c = FRONTYARD_C1;
const int win_dir = WIN_00_DIRECTORY;