#include "bungee_zombie.hpp"
#include "../dancer/dancer_zombie.hpp" // queue_spawned_zombie
#include "../roof/roof_shared.hpp"
#include "../zombie_type.hpp" // init_zombie
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/plants/umbrellaleaf/umbrellaleaf.hpp" // umbrella cover check
#include "game_param.hpp"
#include "music/music.hpp"
#include "reanim/reanim_zombie.hpp"

extern int game_state;
extern Level level;

// Hanging time over the tile before the bungee acts: the original waits out
// mPhaseCounter = 300 centiseconds = 3 s (repvz Zombie.cpp:1334) -> 180 ticks
// at the 60 tps game tick.
const int BUNGEE_HANG_TICKS = 180;
// Descend / ascend speed (px per tick).
const int BUNGEE_ROPE_SPEED = 7;

BungeeZombie::~BungeeZombie()
{
}

void BungeeZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = *ZOMBIE_HEALTH_LIMIT[BUNGEE_TYPE].rbegin();
    directory_num = BUNGEE_WALK_DIRECTORY;

    // The rope drops over a placeholder tile; the steal target itself is
    // picked with the LIVE lawn when the rope lands (wave zombies are created
    // at level load, when nothing is planted yet) -- see resolve_target.
    // carry_type:
    //   -1 = wave bungee (repvz PickBungeeZombieTarget, Zombie.cpp:1084-1145):
    //        every grid tile is a candidate, planted ones outweigh 10000:1,
    //        and with no valid tile it leaves empty-handed (DieNoLoot) -- it
    //        never delivers a zombie. This is EVERY wave bungee: the
    //        adventure roof pool (5-x) AND Bungee Blitz (5-5), whose flag
    //        waves are exactly five plant thieves (Challenge.cpp's
    //        BungeeBlitz -- the level's core threat is losing your defense,
    //        not extra zombies; the old NORMAL_TYPE carry inverted that).
    //   >=0 = roof final-wave ambush drop (BungeeDropZombie): carries a
    //        zombie and always delivers it. Roof ambushes call force_target()
    //        instead -- see update_final_wave_ambush.
    row = level.playable_rows[rand(0, (int)level.playable_rows.size() - 1)];
    target_col = rand(1, 5);
    carry_type = -1;
    x_location = cells[row][target_col].x1 - 40;
    y_location = cells[row][0].y1 - 50;
    is_moving = false; // never walks the lawn
}

void BungeeZombie::force_target(const int &t_row, const int &t_col, const int &carry)
{
    row = t_row;
    target_col = t_col;
    carry_type = carry;
    forced = true;
    x_location = cells[row][target_col].x1 - 40;
    y_location = cells[row][0].y1 - 50;
}

// Rope reached the tile: pick the target with the CURRENT lawn.
void BungeeZombie::resolve_target()
{
    if (forced)
        return;
    if (carry_type >= 0)
        return; // ambush drop (force_target): the drop is the point, lawn or not

    // repvz PickBungeeZombieTarget (Zombie.cpp:1084-1145): weighted grid pick
    // -- planted tile outweighs an empty one 10000:1; tiles already claimed
    // by another bungee are off limits (BungeeIsTargetingCell); sun-makers are
    // skipped once all but the last are claimed (no sunflower wipe-out); a
    // grave-buster mid-chomp is not a target. No valid pick: the bungee pulls
    // back up with nothing (DieNoLoot -- no free zombie delivery).
    auto makes_sun = [](int ptype) {
        return ptype == SUNFLOWER_TYPE || ptype == SUNSHROOM_TYPE;
    };
    int sun_count = 0;
    for (auto *p : game_characters.plants)
        if (makes_sun(p->get_type()))
            sun_count++;
    int sun_targeted = 0;
    for (auto *z : game_characters.zombies)
    {
        if (z == this || z->type != BUNGEE_TYPE)
            continue;
        auto *b = static_cast<BungeeZombie *>(z);
        int p = plant_index_at(b->row, b->target_col);
        if (p >= 0 && makes_sun(game_characters.plants[p]->get_type()))
            sun_targeted++;
    }
    const bool allow_sun = sun_targeted != sun_count - 1;

    struct Pick
    {
        int row, col, weight;
    };
    vector<Pick> picks;
    for (int r : level.playable_rows)
        for (int c = 0; c < HORIZ_BLOCK_COUNT; c++)
        {
            bool claimed = false;
            for (auto *z : game_characters.zombies)
            {
                if (z == this || z->type != BUNGEE_TYPE)
                    continue;
                auto *b = static_cast<BungeeZombie *>(z);
                if (b->phase != GONE && b->row == r && b->target_col == c)
                {
                    claimed = true; // another bungee hangs over this tile
                    break;
                }
            }
            if (claimed)
                continue;
            int weight = 1;
            if (cells[r][c].is_planted)
            {
                int p = plant_index_at(r, c);
                if (p < 0)
                    continue;
                int ptype = game_characters.plants[p]->get_type();
                if (ptype == GRAVEBUSTER_TYPE)
                    continue;
                if (makes_sun(ptype) && !allow_sun)
                    continue; // never gang up on the last sun-maker
                if (umbrella_covers(r, c)) // umbrella cover repels steals
                    continue;
                weight = 10000;
            }
            picks.push_back({r, c, weight});
        }
    if (picks.empty())
    {
        phase = ASCEND; // DieNoLoot: nothing worth grabbing -> back up it goes
        return;
    }
    int total = 0;
    for (const auto &pk : picks)
        total += pk.weight;
    int rollit = rand(0, total - 1);
    for (const auto &pk : picks)
        if ((rollit -= pk.weight) < 0)
        {
            row = pk.row;
            target_col = pk.col;
            break;
        }
    x_location = cells[row][target_col].x1 - 40;
    y_location = cells[row][0].y1 - 50;
}

bool BungeeZombie::on_move_tick()
{
    switch (phase)
    {
    case DESCEND:
        hang_dy += BUNGEE_ROPE_SPEED;
        if (hang_dy >= 0)
        {
            hang_dy = 0;
            resolve_target(); // pick the target with the live lawn
            phase = HANG;
            hang_ticks = BUNGEE_HANG_TICKS;
            play_sound_effect(BUNGEE_SCREAM_SND_DIRECTORY);
        }
        return true;
    case HANG:
        if (--hang_ticks <= 0)
            phase = ACT;
        return true;
    case ACT:
        if (carry_type < 0)
        {
            // Steal the plant under the rope (if it's still there) - unless an
            // umbrella leaf covers the tile: the bungee bounces off empty.
            if (umbrella_protects(row, target_col))
            {
                phase = ASCEND;
                return true;
            }
            int p = plant_index_at(row, target_col);
            if (p >= 0)
            {
                delete_plant(game_characters.plants, p);
                p++;
                play_sound_effect(BUNGEE_SCREAM_SND_DIRECTORY);
            }
        }
        else
        {
            // Deliver the carried zombie onto the tile (roof final-wave
            // ambush drop, BungeeDropZombie).
            Zombie *z = init_zombie(carry_type, level.level_num);
            z->row = row;
            z->x_location = cells[row][target_col].x2 - 90;
            z->y_location = cells[row][0].y1 - 50;
            z->wave_id = wave_id;
            queue_spawned_zombie(z);
            play_sound_effect(IMP_SND_DIRECTORY);
        }
        phase = ASCEND;
        return true;
    case ASCEND:
        hang_dy -= BUNGEE_ROPE_SPEED + 2;
        if (hang_dy < -420)
            phase = GONE;
        return true;
    case GONE:
        return true;
    }
    return true;
}

bool BungeeZombie::on_reach_plant(const int &)
{
    return true; // hangs above plants; never stops to chew
}

bool BungeeZombie::can_eat_plant() const
{
    return false;
}

bool BungeeZombie::despawn_ready() const
{
    return phase == GONE;
}

void BungeeZombie::determine_appearance(vector<ZombiePart> &)
{
    // No armor/arm visuals: the attack flash is the damage cue.
}

void BungeeZombie::add_zombie_die(vector<ZombiePart> &zombie_parts)
{
    // Shot down mid-hang: the bungee art set has no anim_death, so the downed
    // zombie drops a plain-skin corpse onto the lawn (approximation).
    Zombie::add_zombie_die(zombie_parts);
}

void BungeeZombie::render_zombie()
{
    if (phase != anim_phase)
    {
        anim_t = 0.0f;
        anim_phase = phase;
    }
    const char *seg = "anim_hold";
    if (phase == DESCEND)
        seg = carry_type < 0 ? "anim_hold" : "anim_drop";
    else if (phase == ACT)
        seg = carry_type < 0 ? "anim_grab" : "anim_drop";
    else if (phase == ASCEND)
        seg = "anim_raise";
    if (reanimz::drawSkinSegment(reanimz::SKIN_BUNGEE, seg, anim_t,
                                 x_location, y_location,
                                 cold_time > 0, attacked_time > 0,
                                 0, hang_dy, WORLD_SCALE, phase != ACT))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && !check_status(game_state, IS_PAUSED); _lt--)
            anim_t += 0.2f;
        return;
    }
    Zombie::render_zombie();
}
