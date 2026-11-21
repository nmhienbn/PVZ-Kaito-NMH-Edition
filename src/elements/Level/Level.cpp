#include "Level.hpp"
#include "draw/ImageType.hpp"
#include "elements/zombies/zombie.hpp" // delete pending grave risers
#include "game_param.hpp"
#include "music/music.hpp"

// Background indices (level_data's background_map, applied as
// BACKGROUND_LV1_DIRECTORY + map_type offsets).
static const int MAP_FRONTYARD_NIGHT = 3;
static const int MAP_POOL_NIGHT = 5;
static const int MAP_ROOF_NIGHT = 7;
static const int MAP_ROOF_DAY = 6;

extern Level level;

// The single name<->enum table for the JSON "special" key; both directions
// go through it so a rename/typo cannot live in one direction only.
static const struct
{
    const char *name;
    LevelSpecial value;
} SPECIAL_NAMES[] = {
    {"", LevelSpecial::None},
    {"whack", LevelSpecial::Whack},
    {"big_trouble", LevelSpecial::BigTrouble},
    {"vasebreaker", LevelSpecial::Vasebreaker},
    {"storm", LevelSpecial::Storm},
    {"bungee", LevelSpecial::Bungee},
    {"boss", LevelSpecial::Boss}};

bool parse_level_special(const string &name, LevelSpecial &out)
{
    for (const auto &entry : SPECIAL_NAMES)
        if (name == entry.name)
        {
            out = entry.value;
            return true;
        }
    return false;
}

const char *special_name(LevelSpecial special)
{
    for (const auto &entry : SPECIAL_NAMES)
        if (entry.value == special)
            return entry.name;
    return "";
}

int roof_slope_offset(const int &x)
{
    if (level.map_type != MAP_ROOF_DAY && level.map_type != MAP_ROOF_NIGHT)
        return 0;
    // Slope line = left edge of column 5 (the roof flattens past it).
    const int hinge = X_UPPER_LEFT + 5 * BLOCK_WIDTH; // 660 = 440 + 220 pan
    if (x >= hinge)
        return 0;
    // repvz GetPosYBasedOnRow: (440 - posX) * 0.25, clone x = orig x + 220.
    return (int)((hinge - x) * 0.25f + 0.5f);
}

int roof_tile_dy(const int &col)
{
    if (level.map_type != MAP_ROOF_DAY && level.map_type != MAP_ROOF_NIGHT)
        return 0;
    if (col > ROOF_SLOPE_LAST_COL)
        return 0;
    // repvz GridToPixelY: aSlopeOffset = (5 - gridX) * 20 for gridX < 5.
    return (ROOF_SLOPE_LAST_COL + 1 - col) * 20;
}

// Mushrooms stay awake on the night stages only (night yard, fog, boss roof).
bool Level::is_night() const
{
    return map_type == MAP_FRONTYARD_NIGHT || map_type == MAP_POOL_NIGHT ||
           map_type == MAP_ROOF_NIGHT;
}

bool Level::row_has_water(int row) const
{
    return row >= 0 && row < (int)water_rows.size() && water_rows[row];
}

bool Level::is_roof() const
{
    return map_type == MAP_ROOF_DAY || map_type == MAP_ROOF_NIGHT;
}

bool Level::has_water_rows() const
{
    for (bool w : water_rows)
        if (w)
            return true;
    return false;
}

float Level::grave_grow_progress(int row, int col) const
{
    for (const auto &g : grave_grows)
        if (g.row == row && g.col == col)
        {
            // clk above GRAVE_GROW_TICKS is the spawn stagger (repvz's
            // mGridItemCounter = -Rand(50) head start): still underground.
            if (g.clk > GRAVE_GROW_TICKS)
                return 0.0f;
            if (g.clk <= 0)
                return 1.0f;
            // Ease-out grow-in (pops fast, settles into full size) instead
            // of a mechanical linear ramp over the 1-s window.
            float p = 1.0f - (float)g.clk / GRAVE_GROW_TICKS;
            return 1.0f - (1.0f - p) * (1.0f - p) * (1.0f - p);
        }
    return 1.0f;
}

Level::Level()
    : background_directory(BACKGROUND_DIRECTORY),
      // Old reset() ended by re-arming the null announcer; make that the
      // construction default too (the bare Announcer() ctor leaves the fields
      // uninitialized).
      announcer(NULL_DIRECTORY, NULL_MUSIC_DIRECTORY)
{
}

Level::~Level()
{
    // Pending grave risers never joined game_characters.zombies: free them.
    for (auto &r : grave_rises)
        delete r.riser;
}

/*Reset level: Clear all zombie waves' information.
Every member restores its own default initializer via *this = Level(); only
the loader-identity fields survive (load_level reads level.num right after
reset_level() and re-reads the JSON for the rest; choose_level sets
background_directory after load_level).*/
void Level::reset()
{
    int keep_level_num = level_num;
    int keep_map_type = map_type;
    bool keep_has_sun_from_sky = has_sun_from_sky;
    int keep_background_directory = background_directory;
    ZOMBIE_DELAY_STATE keep_zombie_delay_state = zombie_delay_state;
    Advice keep_advice = advice;
    for (auto &r : grave_rises) // risers in the dirt phase die with the level
        delete r.riser;
    *this = Level();
    level_num = keep_level_num;
    map_type = keep_map_type;
    has_sun_from_sky = keep_has_sun_from_sky;
    background_directory = keep_background_directory;
    zombie_delay_state = keep_zombie_delay_state;
    advice = keep_advice;
}