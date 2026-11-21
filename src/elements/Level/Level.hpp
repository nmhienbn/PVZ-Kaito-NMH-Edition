#pragma once
#include "elements/announcer/announcer.hpp"
#include "elements/announcer/advice.hpp"
#include "elements/zombies/wave.hpp"
#include <string>
using std::string;

class Zombie; // pending grave risers (grave_rises below)

enum ZOMBIE_DELAY_STATE
{
    NO_DELAY = 1,
    HUGE_WAVE = 2,
    FINAL_WAVE = 4,
    AFTER_ANNOUNCE = 8
};

/*Level special modes (JSON "special" key): the single-name table lives next
to the enum parsing in Level.cpp; gameplay code goes through
Level::is_special() instead of comparing strings.*/
enum class LevelSpecial
{
    None,
    Whack,       // whack-a-zombie (2-5)
    BigTrouble,  // Big Trouble Little Zombie (3-5)
    Vasebreaker, // (4-5, + vases grid)
    Storm,       // (4-10)
    Bungee,      // Bungee Blitz (5-5)
    Boss         // Zomboss fight (5-10)
};

// Parse a JSON "special" string; false = name unknown to the table
// (empty string maps to None).
bool parse_level_special(const string &name, LevelSpecial &out);
// Reverse lookup for logs/diagnostics: "" for None.
const char *special_name(LevelSpecial special);

/*
Information about level:
*/
struct Level
{
    // Every member carries its own default; reset() restores the construction
    // state with *this = Level() (keeping the loader-identity fields), so a
    // new field can no longer leak state between levels by missing the reset
    // list. Identity/config fields preserved across reset(): level_num,
    // map_type, has_sun_from_sky, background_directory, zombie_delay_state,
    // advice (read or re-set by load_level()/choose_level around the reset).
    int level_num = 0;            // level num
    int map_type = 0;             // type of map: day, night
    string level_name;            // "Level 1-1" style name
    bool has_sun_from_sky = false; // true if has sun from the sky
    LevelSpecial special = LevelSpecial::None; // special mode (see enum above)
    // Reanim playback speed multiplier for zombies (Big Trouble Little Zombie:
    // mini zombies animate at 2x, exactly 1/mScaleZombie -- repvz Zombie.cpp
    // UpdateAnimSpeed). Read from the level JSON's "zombie_time_mul";
    // big_trouble defaults to 2.0, all other levels 1.0.
    float zombie_time_mul = 1.0f;

    vector<int> playable_rows; // lanes where plants and zombies are allowed
    vector<bool> water_rows;   // pool/fog: water lanes (is_water tiles)
    int fog_column = -1;       // fog starts at this 0-based column (-1: none)
    // Zomboni ice trail per row: leftmost x covered by the slick (it runs
    // back to the street); -1 = none.
    vector<int> ice_min_x;
    // Zomboni ice trail lifetime per row, in ticks (original Board:mIceTimer
    // = 3000cs -> 1800 ticks at 60 tps). The zomboni refreshes it while
    // driving; when it runs out the trail's ice_min_x resets to -1 (melted).
    // Jalapeno's burn_row clears the row's ice on the spot.
    vector<int> ice_timer;

    // true when the stage has nighttime rules (mushrooms awake): night yard,
    // night pool (fog), night roof (boss).
    bool is_night() const;
    bool row_has_water(int row) const;
    // roof day / roof night (background_map 6/7): the slope stages
    bool is_roof() const;
    // any lane is water (pool/fog): gates the final-wave ambush choice
    bool has_water_rows() const;
    // current special mode check (replaces string compares on `special`)
    bool is_special(LevelSpecial s) const { return special == s; }

    string seed_mode;        // fixed | all_unlocked | bowling | conveyor
    vector<int> seed_plants; // plant types available for this level
    // conveyor/bowling: (plant type, spawn weight) pairs dispensed by the belt
    vector<pair<int, int>> belt_seeds;
    bool has_shovel = false; // shovel available (from level 5 on)

    int sky_sun_clk = 0;    // ticks until the next sun drops from the sky
    int sky_sun_fallen = 0; // sky suns dropped so far (intervals tighten over time)

    int wave_spawn_health = 0; // total health of the wave currently on the lawn

    // Early-trigger threshold for the wave being waited on (the clone's
    // mZombieHealthToNextWave, Board.cpp:5367): rolled when the previous wave
    // spawns as RandRangeFloat(0.5,0.65) x its spawn health -- the next wave's
    // countdown is cut to 2 s once that wave is chewed below this. 0 for the
    // wait into a flag wave (full clear only); -1 before the first wave.
    float wave_trigger_health = -1.0f;

    int wave_count = 0;          // number of wave
    int cur_wave = 0;            // current wave
    int final_wave = -1;          // final wave
    bool waves_finished = false; // true if waves has finished

    bool zombie_has_coming = false; // true if any zombie has appeared
    int zombie_count = 0;           // total level zombies
    int used_zombie_count = 0;      // appeared zombies

    int next_wave_clk = 0; // next wave clk
    // Countdown value when the current wave wait was armed (the wave's
    // delay_time). The early trigger requires at least 400 cs (240 ticks)
    // of it to have elapsed before it may cut the wait short
    // (repvz Board.cpp:5589: mZombieCountDownStart - mZombieCountDown > 400).
    int next_wave_clk_start = 0;
    // Huge/final wave announce -> spawn countdown, 7.5 s like the original's
    // mHugeWaveCountDown = 750 (repvz Board.cpp:5490-5492,5568): armed when
    // the announce state is entered, the banner (300 ticks) covers most of
    // it, and the wave then spawns at once -- it never waits for the lawn
    // to thin out.
    int huge_wave_clk = 0;

    int background_directory = 0; // Level() sets BACKGROUND_DIRECTORY

    bool graves_have_cones = false; // night: grave-spawned zombies may wear cones

    // Final-wave ambush beat (repvz Board.cpp:5159-5162 + 5481-5486): when the
    // final wave spawns the original arms mRiseFromGraveCounter = 210cs; at 0
    // the map's ambush fires (pool zombies out of the water / bungees from the
    // sky / graves rise) -- 126 ticks at 60 tps. 0 = idle.
    int final_ambush_clk = 0;

    /*Night gravestone bookkeeping (driven by update_graves in
    elements_actions.cpp):
      + grave_grows: a stone sprouted at a huge wave scales in over
        GRAVE_GROW_TICKS (60 ticks = the original's 100cs, DrawGraveStone's
        dirt->stone moment -- repvz GridItem mGridItemCounter -Rand(50) -> 100);
      + grave_rises: a pending grave-spawned zombie: dirt bursts at the
        grave for GRAVE_RISE_HOLD_TICKS (90 ticks = the original's 150cs
        PHASE_RISING_FROM_GRAVE, Zombie.cpp:8275-8276); the last
        GRAVE_RISE_EMERGE_TICKS (the original's altitude window, 50cs of
        -200..0 ground-clipped, UpdateZombieRiseFromGrave Zombie.cpp:3093)
        the body exists as `riser` and slides out of the ground
        render-side before joining game_characters.zombies when clk hits 0.
        Level owns pending risers (reset and ~Level delete them).*/
    struct GraveGrow { int row, col, clk; };
    struct GraveRise { int type, row, col, clk; Zombie *riser = nullptr; };
    vector<GraveGrow> grave_grows;
    vector<GraveRise> grave_rises;
    // Render hook: stone's grow-in scale for the tile, 0..1 (1 = full-grown or
    // not sprouting). render_elements.cpp multiplies the grave sprite by it.
    float grave_grow_progress(int row, int col) const;

    // Mower-lane spawn relief (repvz Board::PickRowForNewZombie's
    // mWaveRowGotLawnMowered row weights): wave index at which the row's mower
    // fired; -1 = never. Sized to the grid on first use.
    vector<int> row_mowed_wave;

    Announcer announcer; // Level() arms the null announcer (see Level.cpp)
    Advice advice;

    ZOMBIE_DELAY_STATE zombie_delay_state = NO_DELAY;

    vector<Wave> waves;
    vector<int> count_zombie_type;

    Level();
    ~Level(); // frees pending grave risers
    void reset();
};

// Zomboni ice trail lifetime: original Board mIceTimer = 3000 centiseconds
// (30 s) refreshed while the zomboni drives -> 1800 ticks at 60 tps.
const int ICE_TRAIL_TICKS = 1800;
/*Roof slope (map_type 6/7 == "roof"/"roof_night"), origin Board.cpp:9226:
GetPosYBasedOnRow adds (440 - posX) * 0.25 in the 800-px board for posX left
of column 5 (440 = 40 + 5*80). The clone board shows the same art un-panned
(BOARD_OFFSET 220), so the hinge sits at 440 + 220 = 660 = X_UPPER_LEFT +
5*BLOCK_WIDTH and the factor stays exactly 0.25. Render-only for
zombies/vehicles: draw y only, never movement/collision.
@return extra y pixels (+ = further down) for a zombie standing at x;
0 off-roof or past the slope line.*/
int roof_slope_offset(const int &x);

/*Roof plant/grid placement step (repvz GridToPixelY, Board.cpp:9242): a tile
in sloped columns 0..4 sits (5 - col) * 20 px lower than the flat lane base.
Plants (and their shadows) anchor to this; 0 on non-roof stages or cols >= 5.*/
int roof_tile_dy(const int &col);

// Grave growth-in duration and risen-zombie hold, in ticks at 60 tps
// (see Level::grave_grows / grave_rises). GRAVE_GROW_TICKS is the original's
// 100cs GridItem counter run (1 s); GRAVE_RISE_EMERGE_TICKS is the visible
// altitude window of UpdateZombieRiseFromGrave (50cs -> 30 ticks).
const int GRAVE_GROW_TICKS = 60;
const int GRAVE_RISE_HOLD_TICKS = 90;
const int GRAVE_RISE_EMERGE_TICKS = 30;