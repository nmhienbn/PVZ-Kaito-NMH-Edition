#include "spikeweed.hpp"
#include "draw/world_scale.hpp"
#include "elements/Map/Map.hpp"
#include "elements/elements.hpp"
#include "support.hpp"
#include "elements/Level/Level.hpp" // roof_tile_dy

// Reanim-only art (2Pak Caltrop.reanim: idle 0..108, attack 109..117). 5 tpf.
static PlantAssets SPIKEWEED_ASSET(5, 88, 40, 88, 40, 2, 40);
// Stab cadence: GOTY re-locks a target ~every 100 ticks @100fps ~= 60 ticks;
// damage lands mid-animation (stateCountdown 75 of 100).
#define SPIKEWEED_HIT_CLK 60 // ~1.0 s between stabs (repvz Plant.cpp:1282-1312)
#define SPIKEWEED_ATTACK_TICKS 40

extern int game_state;
extern Map cells;
extern Window win;
extern Elements game_characters;

/*
Spike Weed constructor: lies flat in its tile; zombies step over it (the tile
is NOT planted as is_block_zombie -- see player_click) and can't eat it.
*/
SpikeWeed::SpikeWeed(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = SPIKEWEED_TYPE;
    health = PLANT_HEALTH_LIMIT[SPIKEWEED_TYPE];
    sec_for_prepare = 1;
    directory_num = SPIKEWEED_SHEET_DIRECTORY;
    attack_range = NO_ATTACK; // damage pulse is handled manually
    hit_clock = 0;
}

SpikeWeed::~SpikeWeed()
{
}

/*
Spike Weed's action: everything walking over the spikes gets stabbed once per
beat (1 pea of damage, GOTY 20 dmg). Vehicles running them over (Zomboni,
catapult) blow their tires -- and the spikes go down with the kill (repvz
DoRowAreaDamage: spikeweed Die()s on a tire pop; spikerock would just shed a
spike tier, but there is no spikerock here). Gargantuar smash has no
counterpart yet (TODO with the zombie agents).
*/
void SpikeWeed::action()
{
    if (check_status(game_state, IS_PAUSED))
        return;
    if (hit_clock)
    {
        hit_clock--;
        return;
    }
    int left_limit = cells[row][col].x1;
    int right_limit = cells[row][col].x2;
    auto &zombies = game_characters.zombies;
    bool hit_type = false;
    for (int j = 0; j < (int)zombies.size(); j++)
    {
        Zombie *z = zombies[j];
        if (z->row != row || z->is_charmed || z->freeze_time)
            continue;
        // stepping onto the spike tile: the torso takes the stab whenever it
        // overlaps the tile (GOTY rect-overlap on the plant's own square)
        int zx = z->x_location + 70;
        if (!is_in(left_limit, zx, right_limit + 40))
            continue;
        hit_type = true;
        if (z->type == ZOMBONI_TYPE || z->type == CATAPULT_TYPE)
        {
            // Tire pop (repvz Plant::DoRowAreaDamage DAMAGE_SPIKE,
            // Plant.cpp:678-697): the vehicle takes the 1800-dmg burst and
            // the spikeweed goes with it -- a plain spikeweed Die()s popping
            // the tire; only a spikerock would just lose a spike tier, and
            // this clone has no spikerock. is_died() means update_plants_status
            // carries the plant away right after this action.
            z->add_zombie_die(game_characters.zombie_parts);
            delete z;
            zombies.erase(zombies.begin() + j);
            health = 0;
            break;
        }
        if (z->take_hit(1, game_characters.zombie_parts))
        {
            z->add_zombie_die(game_characters.zombie_parts);
            delete z;
            zombies.erase(zombies.begin() + j);
            j--;
        }
    }
    if (hit_type)
    {
        hit_clock = SPIKEWEED_HIT_CLK;
        // stab flash: replay the short attack clip
        directory_num = SPIKEWEED_ATTACK_DIRECTORY;
        frame = 0;
    }
    else if (directory_num == SPIKEWEED_ATTACK_DIRECTORY &&
             frame >= SPIKEWEED_ATTACK_TICKS)
    {
        directory_num = SPIKEWEED_SHEET_DIRECTORY;
        frame = 0;
    }
}

/*
Display spike weeds in row
*/
void SpikeWeed::display(const int &_row)
{
    if (row != _row)
        return;
    if (display_reanim(SPIKEWEED_ASSET))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        Plants::next_frame(SPIKEWEED_ASSET);
        return;
    }
    win.draw_png_width_scaled(SPIKEWEED_DIRECTORY, cells[row][col].x1,
                              (cells[row][col].y2 + roof_tile_dy(col)) - 32, world_px(cells[row][col].x2 - cells[row][col].x1));
    Plants::next_frame(SPIKEWEED_ASSET);
}
