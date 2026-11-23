#include "pogo_zombie.hpp"
#include "elements/Level/Level.hpp" // roof_slope_offset
#include "../roof/roof_shared.hpp"
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "game_param.hpp"
#include "music/music.hpp"
#include "reanim/reanim_zombie.hpp"

extern int game_state;

// Bounce hop over one plant: ~0.6 s, carries the zombie one tile + margin.
const int POGO_HOP_TICKS = 36;
// On-stick speed: the original bounces at a fixed 0.45 px/cs (POGO in
// PickRandomSpeed's brisk bracket, repvz Zombie.cpp:1196-1200) -> 24.5 px/s
// under HEAD_SPEED_SCALE.
const float POGO_PX = CS_VEL(0.45f) * HEAD_SPEED_SCALE;

PogoZombie::~PogoZombie()
{
}

void PogoZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = *ZOMBIE_HEALTH_LIMIT[POGO_TYPE].rbegin();
    directory_num = POGO_WALK_DIRECTORY;
}

void PogoZombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - dir_height);
    directory_num = POGO_WALK_DIRECTORY;
}

float PogoZombie::speed_px_per_tick() const
{
    return has_pogo ? POGO_PX : Zombie::speed_px_per_tick();
}

bool PogoZombie::on_reach_plant(const int &col)
{
    if (hop_ticks > 0)
        return true; // mid-hop: keep sailing over the tile
    if (!has_pogo)
        return false; // stick lost: regular chewing
    // A Tall-nut is too tall to bounce over: stop and chew through it.
    int p = plant_index_at(row, col);
    if (p >= 0 && game_characters.plants[p]->get_type() == TALLNUT_TYPE)
        return false;
    // Bounce over the plant: the hop slides the logical box past the tile
    // while the pogo bounce anim keeps playing.
    hop_ticks = POGO_HOP_TICKS;
    hop_start_x = x_location;
    hop_travel = BLOCK_WIDTH + 55;
    return true;
}

bool PogoZombie::on_move_tick()
{
    if (!sound_played)
    {
        play_sound_effect(POGO_ZOMBIE_MUSIC_DIRECTORY);
        sound_played = true;
    }
    if (hop_ticks <= 0)
        return false;
    hop_ticks--;
    x_location = hop_start_x - (POGO_HOP_TICKS - hop_ticks) * hop_travel / POGO_HOP_TICKS;
    if (hop_ticks <= 0)
        x_location = hop_start_x - hop_travel;
    return true;
}

bool PogoZombie::can_eat_plant() const
{
    return hop_ticks == 0;
}

void PogoZombie::drop_pogo()
{
    if (!has_pogo)
        return;
    has_pogo = false;
    hop_ticks = 0;
    anim_t = 0.0f;
}

void PogoZombie::determine_appearance(vector<ZombiePart> &)
{
    // No armor/arm visuals: the attack flash is the damage cue.
}

void PogoZombie::add_zombie_die(vector<ZombiePart> &zombie_parts)
{
    if (reanimz::ready())
    {
        zombie_parts.push_back(ZombiePart(reanimz::DEATH_PART_IMG, 1, row, x_location,
                                          ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                          reanimz::SKIN_POGO, false, head_dropped));
        return;
    }
    Zombie::add_zombie_die(zombie_parts);
}

void PogoZombie::render_zombie()
{
    bool eating = !is_moving;
    if (eating != was_eating)
    {
        anim_t = 0.0f;
        was_eating = eating;
    }
    const char *seg = has_pogo ? (eating ? "anim_eat" : "anim_pogo")
                               : (eating ? "anim_eat" : "anim_walk");
    if (reanimz::drawSkinSegment(reanimz::SKIN_POGO, seg, anim_t,
                                 x_location, y_location + roof_slope_offset(x_location),
                                 cold_time > 0, attacked_time > 0,
                                 0, 0, WORLD_SCALE))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && !check_status(game_state, IS_PAUSED); _lt--)
            anim_t += 0.2f;
        return;
    }
    Zombie::render_zombie();
}
