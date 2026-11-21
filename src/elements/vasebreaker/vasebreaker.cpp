#include "vasebreaker.hpp"
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/Player/Player.hpp"
#include "elements/elements.hpp"
#include "elements/plants/plant_type.hpp"
#include "elements/zombies/zombie_type.hpp"
#include "level/level_data.hpp"
#include "music/music.hpp"
#include <algorithm>

extern Level level;
extern Elements game_characters;
extern Map cells;
extern Window win;

namespace vasebreaker {

namespace {

enum VaseContent
{
    VASE_PLANT,
    VASE_ZOMBIE
};

struct Vase
{
    int row, col;
    VaseContent content;
    int type; // plant type / zombie type
    bool broken = false;
};

vector<Vase> g_vases;
bool g_populated = false;    // grid built for the current level instance
int g_populated_level = -1;  // guards against re-populating mid-level

// Adventure 4-5 exactly per the original ScaryPotterPopulate stage 0
// (repvz Challenge.cpp:3919-3933): 15 pots on cols 6-8 x rows 0..4, holding
// 5 peashooter + 5 squash + 4 normal zombies + 1 buckethead.
struct ContentDef
{
    VaseContent content;
    int type;
    int count;
};
const ContentDef SCARY_POTTER_45[] = {
    {VASE_PLANT, PEASHOOTER_TYPE, 5},
    {VASE_PLANT, SQUASH_TYPE, 5},
    {VASE_ZOMBIE, NORMAL_TYPE, 4},
    {VASE_ZOMBIE, BUCKET_TYPE, 1},
};

void populate()
{
    g_populated = true;
    // Build the content deck from the preset, then shuffle.
    vector<pair<VaseContent, int>> deck;
    for (const auto &def : SCARY_POTTER_45)
        for (int i = 0; i < def.count; i++)
            deck.push_back({def.content, def.type});
    for (int i = (int)deck.size() - 1; i > 0; i--)
        std::swap(deck[i], deck[rand(0, i)]);
    // The original's grid loop takes rows 0..MAX_GRID_SIZE_Y-2 (five rows on
    // the 6-row fog board) and ScaryPotterDontPlaceInCol removes cols 0-5.
    size_t k = 0;
    for (int col = 6; col < HORIZ_BLOCK_COUNT && k < deck.size(); col++)
        for (int row = 0; row < GRID_ROWS - 1 && k < deck.size(); row++)
        {
            g_vases.push_back(Vase{row, col, deck[k].first, deck[k].second, false});
            k++;
        }
    // Progress bar rides on zombie counts even in this waveless level: the
    // vases hold the whole horde, so pre-count them here.
    for (const auto &v : g_vases)
        if (v.content == VASE_ZOMBIE)
            level.zombie_count++;
}

void break_vase(Vase &vase);

void break_vase(Vase &vase)
{
    vase.broken = true;
    play_sound_effect(SHIELD_HIT_MUSIC_DIRECTORY); // ceramic clank
    if (vase.content == VASE_ZOMBIE)
    {
        Zombie *z = init_zombie(vase.type, level.level_num);
        z->row = vase.row;
        z->x_location = cells[vase.row][vase.col].x2 - 90;
        z->y_location = cells[vase.row][0].y1 - 50;
        game_characters.zombies.push_back(z);
        level.used_zombie_count++;
        if (!level.zombie_has_coming)
        {
            play_sound_effect(ZOMBIE_COMING_MUSIC_DIRECTORY);
            level.zombie_has_coming = true;
        }
        return;
    }
    // Plant vase: the original drops a usable seed-packet coin hopping out
    // of the pot (ScaryPotterOpenPot, Challenge.cpp:3879-3881); clicking the
    // packet puts it onto the cursor (pick_sun_if_clicked_on) for
    // click-to-plant. No auto-planting anymore.
    spawn_seed_packet_pickup(game_characters.suns, vase.row, vase.col, vase.type);
}

} // namespace

void update()
{
    if (!level.is_special(LevelSpecial::Vasebreaker))
        return;
    if (!g_populated || g_populated_level != level.level_num)
    {
        g_vases.clear();
        g_populated = false;
        g_populated_level = level.level_num;
        populate();
    }
    // Win gate: once every vase is broken the level reports its waves as
    // finished; has_player_won() then only waits for the spawned horde.
    if (level.waves_finished)
        return;
    for (const auto &v : g_vases)
        if (!v.broken)
            return;
    level.waves_finished = true;
}

bool break_vase_if_clicked(const int &mouse_x, const int &mouse_y)
{
    for (auto &v : g_vases)
    {
        if (v.broken)
            continue;
        if (mouse_x > cells[v.row][v.col].x1 && mouse_x < cells[v.row][v.col].x2 &&
            mouse_y > cells[v.row][v.col].y1 && mouse_y < cells[v.row][v.col].y2)
        {
            break_vase(v);
            return true;
        }
    }
    return false;
}

void display_row(const int &row)
{
    for (const auto &v : g_vases)
    {
        if (v.broken || v.row != row)
            continue;
        // Top edge sits 96 px over the tile bottom (the 76-tall art keeps its
        // frame pad); the world knob scales offset and art alike, so at 1.0
        // the placement is unchanged.
        const int vw = world_px(62), vh = world_px(76);
        win.draw_png(VASEBREAKER_VASE_DIRECTORY,
                     cells[row][v.col].x1 + (BLOCK_WIDTH - vw) / 2,
                     cells[row][v.col].y2 - world_px(96), vw, vh);
    }
}

int standing_vases()
{
    int count = 0;
    for (const auto &v : g_vases)
        if (!v.broken)
            count++;
    return count;
}

} // namespace vasebreaker
