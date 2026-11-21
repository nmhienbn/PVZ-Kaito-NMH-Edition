#include "ladder_zombie.hpp"
#include "elements/Level/Level.hpp" // roof_slope_offset
#include "../roof/roof_shared.hpp"
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "game_param.hpp"
#include "music/music.hpp"
#include "reanim/reanim_zombie.hpp"

extern int game_state;

// zombies.json ladder thresholds are {25, 45}: the ladder shield soaks 25
// hits ("500 shield" half of the original's 1000-dmg pool on the 20:1
// effective scale: (1000 - 90)/20 ~= 45), the zombie body keeps the last 20
// -- door-shield pattern, original Zombie.cpp: mShieldHealth = mBodyHealth = 500.
const int LADDER_BREAK_HP = 25;
// anim_placeladder one-shot duration in game ticks (segment is calibrated at
// load; this is the fallback when the skin is missing).
const int LADDER_PLACE_TICKS = 50;

static bool is_defensive_plant(const int &type)
{
    return type == WALLNUT_TYPE || type == TALLNUT_TYPE || type == PUMPKIN_TYPE;
}

LadderZombie::~LadderZombie()
{
}

void LadderZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = *ZOMBIE_HEALTH_LIMIT[LADDER_TYPE].rbegin();
    directory_num = LADDER_WALK_DIRECTORY;
}

void LadderZombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - dir_height);
    directory_num = LADDER_WALK_DIRECTORY;
}

bool LadderZombie::on_reach_plant(const int &col)
{
    if (place_ticks > 0)
        return true; // busy planting the ladder against this plant
    if (!has_ladder)
        return false; // bare-handed: behave like a plain zombie
    int p = plant_index_at(row, col);
    if (p < 0 || !is_defensive_plant(game_characters.plants[p]->get_type()))
        return false; // ladder is only for defensive plants; chew the rest
    // Place the ladder: the tile stops blocking zombies for good (until the
    // plant dies). This zombie then climbs over it like everyone else.
    cells[row][col].has_ladder = true;
    has_ladder = false;
    place_ticks = LADDER_PLACE_TICKS;
    float seg = reanimz::skinSegmentLength(reanimz::SKIN_LADDER, "anim_placeladder");
    if (seg > 0)
        place_ticks = (int)(seg * 5.0f); // 12 reanim fps -> 5 game ticks per frame
    play_sound_effect(LADDER_ZOMBIE_MUSIC_DIRECTORY);
    return true;
}

bool LadderZombie::on_move_tick()
{
    if (place_ticks <= 0)
        return false;
    place_ticks--;
    return true;
}

bool LadderZombie::can_eat_plant() const
{
    return place_ticks == 0;
}

void LadderZombie::determine_appearance(vector<ZombiePart> &)
{
    // Ladder broken (past the shield threshold): the zombie walks on without
    // it. No tossed-part art for the ladder skin; the state change is the cue.
    if (has_ladder && health <= LADDER_BREAK_HP * health_scale)
        has_ladder = false;
}

void LadderZombie::add_zombie_die(vector<ZombiePart> &zombie_parts)
{
    if (reanimz::ready())
    {
        zombie_parts.push_back(ZombiePart(reanimz::DEATH_PART_IMG, 1, row, x_location,
                                          ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                          reanimz::SKIN_LADDER));
        return;
    }
    Zombie::add_zombie_die(zombie_parts);
}

void LadderZombie::render_zombie()
{
    bool eating = !is_moving && place_ticks == 0;
    if (eating != was_eating)
    {
        anim_t = 0.0f;
        was_eating = eating;
    }
    const char *seg;
    bool loop = true;
    if (place_ticks > 0)
    {
        seg = "anim_placeladder";
        loop = false;
    }
    else if (has_ladder)
    {
        seg = eating ? "anim_laddereat" : "anim_ladderwalk";
    }
    else
    {
        seg = eating ? "anim_eat" : "anim_walk";
    }
    if (reanimz::drawSkinSegment(reanimz::SKIN_LADDER, seg, anim_t,
                                 x_location, y_location + roof_slope_offset(x_location),
                                 cold_time > 0, attacked_time > 0,
                                 0, 0, WORLD_SCALE, loop))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && !check_status(game_state, IS_PAUSED); _lt--)
            anim_t += 0.2f;
        return;
    }
    Zombie::render_zombie();
}
