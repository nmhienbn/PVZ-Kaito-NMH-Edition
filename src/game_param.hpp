#pragma once
#include <fstream>
#include <iostream>
#include <set>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <vector>
using namespace std;

#define WINDOW_WIDTH 1050
#define WINDOW_HEIGHT 600

// 0: plants keep the PvZ1 sprite sheets; 1: plants use the PvZ2 PAM skins.
#define USE_PAM_PLANT_SKIN 0

// 0: skip the PopCap reanim (per-part) plant renderer; 1: try reanim first
// when displaying plants (falls back to PAM, then the sprite sheets).
#define USE_REANIM_PLANT_SKIN 1

enum PlantType
{
    SUNFLOWER_TROPHY = -1,
    PEASHOOTER_TYPE,
    SUNFLOWER_TYPE,
    WALLNUT_TYPE,
    SNOWPEA_TYPE,
    POTATOMINE_TYPE,
    CHERRYBOMB_TYPE,
    REPEATER_TYPE,
    CHOMPER_TYPE,
    PUFFSHROOM_TYPE,
    SUNSHROOM_TYPE,
    FUMESHROOM_TYPE,
    GRAVEBUSTER_TYPE,
    HYPNOSHROOM_TYPE,
    SCAREDYSHROOM_TYPE,
    ICESHROOM_TYPE,
    DOOMSHROOM_TYPE,

    // Areas 3-5 (pool / fog / roof), appened in canon almanac order.
    LILYPAD_TYPE,
    SQUASH_TYPE,
    THREEPEATER_TYPE,
    TANGLEKELP_TYPE,
    JALAPENO_TYPE,
    SPIKEWEED_TYPE, // caltrop art (Spikeweed)
    TORCHWOOD_TYPE,
    TALLNUT_TYPE,
    SEASHROOM_TYPE,
    PLANTERN_TYPE,
    CACTUS_TYPE,
    BLOVER_TYPE,
    SPLITPEA_TYPE,
    STARFRUIT_TYPE,
    PUMPKIN_TYPE,
    MAGNETSHROOM_TYPE,
    CABBAGEPULT_TYPE,
    FLOWERPOT_TYPE,
    KERNELPULT_TYPE,
    COFFEEBEAN_TYPE,
    GARLIC_TYPE,
    UMBRELLALEAF_TYPE,
    MARIGOLD_TYPE,
    MELONPULT_TYPE,

    PLANT_COUNT
};

enum ZombieType
{
    NORMAL_TYPE,
    FLAG_TYPE,
    CONE_TYPE,
    BUCKET_TYPE,
    DOOR_TYPE,
    POLEVAULT_TYPE,
    NEWSPAPER_TYPE,
    FOOTBALL_TYPE,
    DANCER_TYPE,
    BACKUP_TYPE,

    // Areas 3-5 (pool / fog / roof zombies), canon order.
    DUCKYTUBE_TYPE, // water-lane visual variant of the basic zombie
    SNORKEL_TYPE,
    ZOMBONI_TYPE,
    BOBSLED_TYPE,
    DOLPHINRIDER_TYPE,
    JACKBOX_TYPE,
    BALLOON_TYPE,
    DIGGER_TYPE,
    POGO_TYPE,
    BUNGEE_TYPE,
    LADDER_TYPE,
    CATAPULT_TYPE,
    GARGANTUAR_TYPE,
    IMP_TYPE,
    ZOMBOSS_TYPE,

    // Zombie Yeti (rare cameo). Canonically it sits between POGO and BUNGEE
    // in the original enum; appended here instead. Safe because every
    // type-indexed table (zombies.json health, ZOMBIE_DEFS registry, almanac)
    // is now keyed by NAME, so the enum position no longer has to match any
    // file order.
    YETI_TYPE,

    COUNT_ZOMBIE_TYPE,
};

enum Game_State
{
    IS_LEVEL_CHOSEN = 1,
    IS_GAME_STARTED = 2,
    IS_PAUSED = 4,
    IS_UNLOCKING_PLANT = 8,
    IS_LEAVE = 16,
    IS_QUIT = 32,
    IS_RESET = 64,
    IS_RESTART = 128,
    IS_FAST = 256,
    IS_CHOOSING_SEEDS = 512,
    // After the loading screen: the selector/mode screen (adventure +
    // almanac + disabled modes) sits in front of the level list. Cleared
    // once the player picks ADVENTURE.
    IS_SELECTOR_SCREEN = 1024

};

inline bool check_status(const int &x, Game_State state)
{
    return ((x & state) > 0);
}

inline void set_status(int &x, Game_State state, const bool &status)
{
    if (status == false)
    {
        if (check_status(x, state))
        {
            x ^= state;
        }
    }
    else
    {
        if (check_status(x, state) == false)
        {
            x |= state;
        }
    }
}

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

// Playground elements size.
// repvz GridToPixelX = col*80 + LAWN_XMIN(40) (Board.cpp:9204). The original
// draws the 1400-px background panned by -BOARD_OFFSET(220) (Board.cpp
// DrawBackdrop); this clone shows the same art un-cropped at image x 0..1049,
// so X_UPPER_LEFT = 40 + 220 = 260.
extern const int BLOCK_WIDTH;
extern const int BLOCK_HEIGHT;
extern const int HORIZ_BLOCK_COUNT;
extern const int VERT_BLOCK_COUNT;

extern const int X_UPPER_LEFT;
extern const int Y_UPPER_LEFT;

// Pool/fog (6-row) board geometry (GridToPixelY of the original: rows of 85 px
// starting at y 80, Board.cpp:9241). Classic 5-row levels: 100 px rows from
// 80. Roof stages: 85 px rows from LAWN_YMIN-10 = 70, plus the per-column
// slope step (see roof_tile_dy in elements/Level/Level.hpp).
extern const int POOL_BLOCK_HEIGHT;
extern const int POOL_Y_UPPER_LEFT;
extern const int ROOF_BLOCK_HEIGHT;
extern const int ROOF_Y_UPPER_LEFT;

// Runtime grid geometry of the CURRENT level, applied by load_level:
// day/night keep the classic 5x9 block layout; pool/fog use 6 rows; roof
// stages use the 85-px rows. GRID_ROOF flags the sloped-roof click mapping.
extern int GRID_ROWS;
extern int GRID_Y_TOP;
extern int GRID_BLOCK_H;
extern bool GRID_ROOF;

// repvz GameConstants.h:17: LAWN_YMIN = 80 -- the click-test top bound for
// every stage (PixelToGridY, Board.cpp:9163).
#define LAWN_YMIN_CLONE 80
#define ROOF_SLOPE_LAST_COL 4 // cols 0..4 are the sloped half

extern const int ELEMENT_WIDTH;
extern const int ELEMENT_HEIGHT;

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Speed properties: increase or decrease the following 9 define values to speed up or speed down entities in the game
extern const int FPS;
// ticks per frame
extern const int TICKS_PER_FRAME;

// Wall-clock frame pacer behind the frame cap (Window::update_screen): holds
// the presentation cadence at exactly FPS frames/s, which is also the
// gameplay tick rate (one tick per main-loop iteration -- level.cpp
// start_level). Pure integer math on a caller-supplied monotonic clock
// (SDL_GetPerformanceCounter in the game; a scripted clock in
// demo/tick_rate_check.cpp), so the mechanism is unit-testable headless.
struct FramePacer
{
    // Deadline (caller clock units) the last presented frame was due at;
    // -1 = disarmed, the next advance() arms the schedule at that instant.
    long long next_deadline = -1;
    long long carry = 0; // Bresenham accumulator for the rational period (0..tps-1)
    // Records the frame just presented at clock `now` and returns the
    // deadline the next frame may start at (caller sleeps until then).
    // hz = clock units per second, tps = target frames per second (FPS).
    long long advance(long long now, long long hz, int tps);
};

// Auto-FPS gate for display-driven animations: the number of fixed-rate
// logic ticks covered by the current rendered frame (0 = none ran). Set by
// the gameplay frame driver (level.cpp) to ticks_run; blocking scenes that
// self-pace at the 60 fps cap set it to 1. Display functions that advance
// their own anim state per call must advance it ONCE PER TICK -- i.e. loop
// `for (int _lt = logic_ticked; _lt > 0; _lt--)` around the per-tick
// advance, merged with the existing pause guard -- so the anim cadence
// exactly tracks the logic tick rate (turbo's 120 tps advances anims 2x,
// matching the movement; the old once-per-render gate left anims at 60/s
// while movement ran 120/s -- visible foot-slide in turbo and on machines
// rendering below the tick rate).
extern int logic_ticked;
// Time appear of first wave of zombies (18 s)
extern const int FIRST_WAVE_CLK_COUNT;
// Time between zombies' different groups (base 25 s wait) ...
extern const int ZOMBIE_CREATE_CLK_COUNT;
// ... plus this random jitter (600 cs -> 360 ticks), for the 25-31 s band
extern const int ZOMBIE_CREATE_CLK_RANGE;
// Wait into a flag wave (45 s breather before a huge wave)
extern const int ZOMBIE_BEFORE_FLAG_CLK_COUNT;
// Zombie bite plant frequency
extern const int BITE_CLK_COUNT;
// Zombie moving frequency -- DEPRECATED legacy global clock: movement is
// per-zombie float px/tick now (Zombie::speed_px_per_tick /
// Zombie::pick_speed, port of repvz PickRandomSpeed). Still parsed from
// parameters.json but nothing reads it.
extern const int ZOMBIE_CLK_COUNT;

// Announcer appear time
extern const int ANNOUNCER_CLK_COUNT;
// Sun change color frequency
extern const int SUN_CHANGE_COLOR_CLK_COUNT;
// Max time blink when attack
extern const int MAX_TIME_BLINK;
// Max time being cold (10s; snow-pea chill)
extern const int MAX_COLD_TIME;
// Ice-shroom lingering chill once the freeze ends (20s in the original)
extern const int ICE_SHROOM_COLD_TIME;
// Max time that a sun can appear
extern const int SUN_APPEAR_LIMIT;

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Initial sun count and normal sun value
extern const int NORMAL_SUN_VALUE;
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

extern const int INF;

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

/*
Moving speed of some elements
*/
extern const int ZOMBIE_INIT_X;
extern const int ZOMBIE_DX;
extern const int ZOMBIE_COLD_DX;
extern const float PEA_DX;
extern const int SUN_DY;
extern const int CLICKED_SUN_D;

extern const vector<int> PLANT_SUN_COST;

extern const vector<int> PLANT_LOADING_TIME;

extern const vector<int> PLANT_HEALTH_LIMIT;

// plants.json "aquatic" flag (lilypad/tanglekelp/seashroom can sit in water)
extern const vector<bool> PLANT_IS_AQUATIC;

extern const vector<set<float>> ZOMBIE_HEALTH_LIMIT;