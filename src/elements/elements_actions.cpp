#include "elements_actions.hpp"
#include "draw/rsdl.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/Player/Player.hpp"
#include "elements/boss/boss_fight.hpp"
#include "elements/bowling/rolling_nut.hpp"
#include "elements/collision/BulletVsZombie.hpp"
#include "elements/collision/MowerVsZombie.hpp"
#include "elements/collision/ZombieVsPlant.hpp"
#include "elements/conveyor/conveyor.hpp"
#include "elements/elements.hpp"
#include "elements/mower/mower.hpp"
#include "elements/storm/storm.hpp"
#include "elements/vasebreaker/vasebreaker.hpp"
#include "elements/zombies/bungee/bungee_zombie.hpp"
#include "elements/zombies/dancer/dancer_zombie.hpp"
#include "elements/zombies/zombie_type.hpp"
#include "reanim/one_shot_fx.hpp"
#include "support.hpp"

extern int clk;
extern Level level;
extern Elements game_characters;

extern Player player;
extern Map cells;
Wave current_wave;

/*Handles all the changes to the game:
    + Create new wave of zombies.
    + Check all bullets' moving status.
    + Check all mowers' moving status.
    + Update all plants' & zombies' status.
    + Generate suns from the sky (not same time). No sun at night.
    + Remove the suns that appear for a long time.
    + Update remaining time of each plant seed.
*/
/*Zomboni ice trails melt ~30 s after the last zomboni pass
(Board::UpdateIce: mIceTimer countdown; at 0 the trail resets).*/
void update_ice_trails()
{
    for (int row = 0; row < (int)level.ice_timer.size(); row++)
    {
        if (level.ice_timer[row] > 0 && --level.ice_timer[row] == 0 &&
            row < (int)level.ice_min_x.size())
            level.ice_min_x[row] = -1;
    }
}

/*=== Grave Buster chew FX + stone shrink bookkeeping =======================
While a buster chews a stone (repvz Plant.cpp:1076-1100 UpdateGraveBuster,
STATE_GRAVEBUSTER_EATING, 400cs) the scene sinks with the chew: the buster's
own draw slides down 0->30 px over the countdown (Plant.cpp:2745-2750) and
the stone shrinks away under it until GridItemDie clears it, when a
GraveBusterDie dirt+dust burst fires (Plant.cpp:1092-1097); pebble sprays
(PARTICLE_GRAVE_BUSTER, Plant.cpp:1085) mark the chew. The buster's
landing/eat clocks live on the plant (gravebuster.cpp, out of this file's
grave section), so this block just watches the board: a grave tile gains an
eat mark the first tick a GraveBuster is seen standing on it, ages ~1
tick/frame in step with the plant's own clocks, and drops the mark when the
buster leaves (a stone whose buster was killed snaps back to full size).
Landing ticks (50 -> the first 0.83 s) show no shrink, then the full
GRAVEBUSTER chew (240) maps linearly to 0..1 of the stone.

Constants mirror gravebuster.cpp (GRAVEBUSTER_LANDING_TICKS / _EAT_TICKS).*/
static const int GRAVE_EAT_LANDING_TICKS = 50;
static const int GRAVE_EAT_TICKS = 240;
static const int GRAVE_EAT_FX_EVERY = 24; // pebble-spray cadence mid-chew

struct GraveEatMark
{
    int row, col;
    int age;    // ticks since the buster first appeared on the stone
    int fx_clk; // ticks until the next chomp spray
};
static vector<GraveEatMark> grave_eats;

/*0 = stone full size, ->1 gone; consumed by the grave renderer.*/
float grave_eaten_fraction(int row, int col)
{
    for (const auto &m : grave_eats)
        if (m.row == row && m.col == col)
        {
            if (m.age <= GRAVE_EAT_LANDING_TICKS)
                return 0.0f;
            return min(1.0f, (float)(m.age - GRAVE_EAT_LANDING_TICKS) /
                                 GRAVE_EAT_TICKS);
        }
    return 0.0f;
}

static void update_grave_eats()
{
    // Find every buster standing on a stone this tick.
    vector<pair<int, int>> busy;
    for (const auto p : game_characters.plants)
        if (p->get_type() == GRAVEBUSTER_TYPE)
        {
            int r = p->get_row(), c = p->get_col();
            if (r >= 0 && r < (int)cells.size() && c >= 0 &&
                c < HORIZ_BLOCK_COUNT && cells[r][c].has_grave)
                busy.push_back({r, c});
        }
    // Age surviving marks; a vanished buster ends the mark -- with the stone
    // gone that is the GraveBusterDie moment (Plant.cpp:1120).
    for (int i = 0; i < (int)grave_eats.size();)
    {
        const auto m = grave_eats[i];
        bool live = false;
        for (const auto &b : busy)
            live |= b.first == m.row && b.second == m.col;
        if (live)
        {
            grave_eats[i].age++;
            // Pebble spray while chewing (PARTICLE_GRAVE_BUSTER, repvz
            // Plant.cpp:1108 AddAttachedParticle, re-attached every
            // GRAVE_EAT_FX_EVERY ticks) at the buster's center -- retail
            // anchors the emitter at mX+40 / mY+40 (Plant.cpp:1107).
            if (grave_eats[i].age > GRAVE_EAT_LANDING_TICKS &&
                --grave_eats[i].fx_clk <= 0)
            {
                grave_eats[i].fx_clk = GRAVE_EAT_FX_EVERY;
                const auto &t = cells[m.row][m.col];
                reanimfx::spawn(reanimfx::FX_GRAVEBUSTER_CHOMP,
                                (t.x1 + t.x2) / 2, t.y1 + 40, m.row);
            }
            i++;
            continue;
        }
        if (!cells[m.row][m.col].has_grave &&
            m.age > GRAVE_EAT_LANDING_TICKS)
        {
            // Stone crumbled: the GraveBusterDie dirt burst, same mX+40 /
            // mY+40 anchor (Plant.cpp:1120).
            const auto &t = cells[m.row][m.col];
            reanimfx::spawn(reanimfx::FX_GRAVEBUSTER_DIE, (t.x1 + t.x2) / 2,
                            t.y1 + 40, m.row);
        }
        grave_eats.erase(grave_eats.begin() + i);
    }
    // First tick a buster is seen on its stone: open the mark.
    for (const auto &b : busy)
    {
        bool known = false;
        for (const auto &m : grave_eats)
            known |= m.row == b.first && m.col == b.second;
        if (!known)
            grave_eats.push_back({b.first, b.second, 0, GRAVE_EAT_FX_EVERY});
    }
}

/*Sprouting-stone clocks, one tick: each pending stone's staggered delay runs
down and the entry retires as the stone reaches full size (the render side
scales it, Level::grave_grow_progress). Split out of update_graves so the
intro pans (seed chooser / credit roll) grow the initial stones BEFORE
gameplay -- repvz sprouts them during the level-intro cutscene, not after
READY-SET-PLANT.*/
void tick_grave_grows()
{
    for (int i = 0; i < (int)level.grave_grows.size();)
    {
        if (--level.grave_grows[i].clk <= 0)
            level.grave_grows.erase(level.grave_grows.begin() + i);
        else
            i++;
    }
}

/*Night gravestone upkeep, one tick (Board::UpdateGridItems, repvz Board.cpp:
9794-9817 -- the grave item counter ages toward 100 while the stone settles):
sprouting stones finish their grow-in, and pending grave risers go through the
original's ground phase (Zombie::RiseFromGrave, Zombie.cpp:7947-8028 /
UpdateZombieRiseFromGrave, Zombie.cpp:3093-3103): the body materializes when
the last GRAVE_RISE_EMERGE_TICKS (50cs) start and slides out of the ground
render-side (altitude -200->0 with the ground clip, drawn by
display_grave_rise from render_elements.cpp), then joins the walking zombies
when the 90-tick hold (150cs) runs out. While pending it keeps retail
behavior: not targetable, not moving (the original's UpdateZombie early-out
at Zombie.cpp:4637).*/
void update_graves()
{
    tick_grave_grows();
    for (int i = 0; i < (int)level.grave_rises.size();)
    {
        auto &rise = level.grave_rises[i];
        if (--rise.clk <= 0)
        {
            // The dirt phase is over: the zombie stands fully emerged and
            // starts walking (RiseFromGrave end state, Zombie.cpp:8265+).
            Zombie *z = rise.riser;
            if (z == nullptr) // safety: never skip the riser entirely
                z = init_zombie(rise.type, level.level_num);
            z->row = rise.row;
            z->x_location = cells[rise.row][rise.col].x2 - 90;
            z->y_location = cells[rise.row][0].y1 - 50;
            z->wave_id = level.cur_wave;
            rise.riser = nullptr; // ownership moves to game_characters
            game_characters.zombies.push_back(z);
            level.grave_rises.erase(level.grave_rises.begin() + i);
            continue;
        }
        if (rise.riser == nullptr && rise.clk <= GRAVE_RISE_EMERGE_TICKS)
        {
            // mPhaseCounter window 50cs->0: build the body now so the row
            // render can slide it up out of the dirt.
            rise.riser = init_zombie(rise.type, level.level_num);
            rise.riser->row = rise.row;
            rise.riser->x_location = cells[rise.row][rise.col].x2 - 90;
            rise.riser->y_location = cells[rise.row][0].y1 - 50;
            rise.riser->wave_id = level.cur_wave;
        }
        i++;
    }

    // Grave Buster chew: age the eat marks (stone shrink + pebble sprays),
    // fire the die burst when a chewed-out stone's buster leaves the board.
    update_grave_eats();
}

/*Doom-shroom craters heal after ~180 s (Board::UpdateGridItems: the crater's
grid-item counter ticks down and the hole closes at 0).*/
void update_doom_craters()
{
    for (int row = 0; row < (int)cells.size(); row++)
        for (int col = 0; col < HORIZ_BLOCK_COUNT; col++)
            if (cells[row][col].has_crater && cells[row][col].crater_clk > 0 &&
                --cells[row][col].crater_clk == 0)
                cells[row][col].has_crater = false;
}

void handle_changes()
{
    // Backup dancers queued by a dancing zombie mid-move-loop (spawning into
    // the vector while it is iterated would invalidate the iterators).
    drain_spawning_zombies(game_characters.zombies);

    // Bungee zombies that flew back off the lawn despawn here (they are not
    // killed by bullets, so the regular death paths never erase them).
    for (int i = 0; i < (int)game_characters.zombies.size();)
    {
        if (game_characters.zombies[i]->despawn_ready())
        {
            delete game_characters.zombies[i];
            game_characters.zombies.erase(game_characters.zombies.begin() + i);
        }
        else
            i++;
    }

    // Zomboss boss level (5-10): the fight controller spawns the Zombot,
    // drives its attack loop and ends the level by killing it. No-op elsewhere.
    bossfight::update();

    // Tutorial advice state machine (levels 1-1/1-2/1-3); also gates the
    // first zombie of 1-1 behind "two peashooters planted".
    level.advice.update();

    // 4-5 vases: click-revealed pots drive the zombie trickle; 4-10 storm:
    // rain loop + lightning schedule. Both are no-ops on other levels.
    vasebreaker::update();
    storm::update();

    // Create new wave of zombies.
    update_new_wave_zombies();

    // Final-wave ambush: pool rise / bungee drop / grave rise, 2.0 s after
    // the final wave spawns (Board.cpp:5258-5265 ticking mRiseFromGraveCounter).
    update_final_wave_ambush();

    // Check all bullets' moving status
    handle_bullet_encounter_zombie(game_characters.bullets, game_characters.zombies, game_characters.zombie_parts);

    // Mower-lane spawn relief bookkeeping (Board::PickRowForNewZombie's
    // mWaveRowGotLawnMowered): detect this tick's mower activations and stamp
    // the row with the wave being spawned. add_zombies_from_level_data reads
    // row_mowed_wave when it rolls lanes.
    vector<char> mower_seen(game_characters.mowers.size(), 0);
    for (int i = 0; i < (int)game_characters.mowers.size(); i++)
        mower_seen[i] = game_characters.mowers[i] == nullptr ||
                        game_characters.mowers[i]->status == MOWER_ACTIVE;

    // Check all mowers' status
    handle_mower_zombie_encounter(game_characters.mowers, game_characters.zombies, game_characters.zombie_parts);

    for (int i = 0; i < (int)game_characters.mowers.size(); i++)
        if (!mower_seen[i] && game_characters.mowers[i] != nullptr &&
            game_characters.mowers[i]->status == MOWER_ACTIVE)
        {
            if ((int)level.row_mowed_wave.size() < (int)cells.size())
                level.row_mowed_wave.assign(cells.size(), -1);
            level.row_mowed_wave[game_characters.mowers[i]->row] = level.cur_wave;
        }

    // Update all plants' status
    update_plants_status(game_characters.plants);
    // Update all zombies' status
    update_zombies_status(game_characters.zombies, game_characters.plants, game_characters.zombie_parts);

    // Zomboni ice trails melt ~30 s after the last zomboni pass; doom-shroom
    // craters heal after ~180 s.
    update_ice_trails();
    update_doom_craters();
    // Night gravestones: sprouting stones finish growing in, risen zombies
    // end their ground-hold and start walking.
    update_graves();

    // Generate suns from the sky. No sun at night.
    // First sun 4.25-7 s in; intervals grow to min(9.5 s, ...) + 0-2.75 s
    // (repvz Board.cpp:5425-5432: min(SUN_COUNTDOWN_MAX=950, SUN_COUNTDOWN=
    // 425 + mNumSunsFallen*10) + Rand(SUN_COUNTDOWN_RANGE=275) centiseconds,
    // i.e. x0.6 ticks here at 60 tps).
    // Exception: the 1-1 tutorial holds the sky sun until the first
    // peashooter is planted (repvz Board.cpp:5417-5421) -- countdown frozen.
    if (level.has_sun_from_sky == true && !level.advice.holds_sky_sun() && --level.sky_sun_clk <= 0)
    {
        gen_random_sun_from_sky(game_characters.suns);
        level.sky_sun_fallen++;
        int interval = 255 + level.sky_sun_fallen * 6;
        if (interval > 570)
            interval = 570;
        level.sky_sun_clk = interval + rand(0, 165);
    }

    // Remove the suns that appear for a long time
    remove_suns(game_characters.suns);

    // Conveyor belt (conveyor & bowling levels): slide/dispense packets
    belt.update();
    // Rolling nuts (bowling): move, hit, explode
    if (belt.bowling)
        update_rolling_nuts();

    // Update remaining time of each plant seed
    update_plant_seeds_remaining_time();
}

/*
Night grave rise: gated to the FINAL flag wave (repvz Board.cpp:5159-5162:
mRiseFromGraveCounter = 210 is armed only when the last wave spawns, and the
graves open when it runs out -- Board.cpp:5486). Each grave sprouts a zombie
with ~70% chance, at least one over all graves. The horde wears a cone some
of the time once the level roster knows coneheads.

Each riser goes through the original's ground phase (Zombie::RiseFromGrave,
Zombie.cpp:7947-8028, including its per-riser foley: FOLEY_GRAVESTONE_RUMBLE
once per zombie, Zombie.cpp:8340): the dirt FX + the rumble fire as the phase
starts, the figure slides out of the ground over the last
GRAVE_RISE_EMERGE_TICKS (the original's 50cs altitude window,
UpdateZombieRiseFromGrave Zombie.cpp:3093), and it joins the lawn when
GRAVE_RISE_HOLD_TICKS (150cs) runs out.
*/
static void schedule_grave_rise(int row, int col, int type)
{
    level.grave_rises.push_back({type, row, col, GRAVE_RISE_HOLD_TICKS});
    const auto &t = cells[row][col];
    // PARTICLE ZombieRise port: arm out of the mound + dirt splash/clumps.
    reanimfx::spawn(reanimfx::FX_ZOMBIE_GRAVE_RISE, (t.x1 + t.x2) / 2,
                    t.y2 - 12, row);
    // The rumble is per riser in the original, not once per wave.
    play_sound_effect(GRAVESTONE_RUMBLE_MUSIC_DIRECTORY);
}

static void rise_zombies_from_graves(vector<Zombie *> &zombies)
{
    (void)zombies; // kept for the old signature; spawns go to game_characters
    vector<pair<int, int>> graves;
    for (int r = 0; r < (int)cells.size(); r++)
        for (int c = 0; c < HORIZ_BLOCK_COUNT; c++)
            if (cells[r][c].has_grave)
                graves.push_back({r, c});
    if (graves.empty())
        return;
    int spawned = 0;
    for (const auto &g : graves)
    {
        if (rand(0, 9) > 6)
            continue;
        int type = (level.graves_have_cones && rand(0, 9) < 4) ? CONE_TYPE : NORMAL_TYPE;
        schedule_grave_rise(g.first, g.second, type);
        spawned++;
    }
    if (spawned == 0)
    {
        const auto &g = graves[rand(0, (int)graves.size() - 1)];
        schedule_grave_rise(g.first, g.second, NORMAL_TYPE);
        spawned = 1;
    }
    // No shared rumble here: schedule_grave_rise plays it per riser (repvz
    // Zombie.cpp:8340), the way the original fires it inside RiseFromGrave.
    level.zombie_count += spawned;
    level.used_zombie_count += spawned;
}

/*=== Final-wave ambush beat (repvz Board::SpawnZombiesFromPool/:5053
SpawnZombiesFromSky, dispatched from SpawnZombiesFromGraves when the rise
counter -- armed at the final wave spawn -- runs out) =======================
At 2.0 s (200 cs -> 120 ticks at 60 tps) after the final wave lands:
    + roof maps: bungees drop 2-3 zombies onto the lawn,
    + pool/fog maps: 2-3 zombies rise out of the pool (cols 5-8 water tiles),
    + other night stages: the grave stones open (above).
The original's ice-trap check (mIceTrapCounter) has no clone counterpart --
the clone's ice-shroom freezes zombies themselves, not the board clock.*/

// Ambush roster (Board::PickGraveRisingZombieType, Board.cpp:2488-2514):
// weighted normal 4000 / cone 4000 (+ bucket 3000 only on stages without
// gravestones -- pool day and roof; night yard and fog keep the tube walkers
// basic). Cone joins when the level roster knows it. Weights are the
// gZombieDefs mPickWeight values (repvz Zombie.cpp:80-114).
static int pick_rising_zombie_type()
{
    int weights[] = {4000, 0, 0};
    if (level.graves_have_cones) // roster includes coneheads
        weights[1] = 4000;
    bool stage_has_graves = false;
    for (int r = 0; r < (int)cells.size() && !stage_has_graves; r++)
        for (int c = 0; c < HORIZ_BLOCK_COUNT; c++)
            if (cells[r][c].has_grave)
            {
                stage_has_graves = true;
                break;
            }
    if (!stage_has_graves && (int)level.count_zombie_type.size() > BUCKET_TYPE &&
        level.count_zombie_type[BUCKET_TYPE] > 0)
        weights[2] = 3000;
    int total = weights[0] + weights[1] + weights[2];
    int roll = rand(0, total - 1);
    if ((roll -= weights[0]) < 0)
        return NORMAL_TYPE;
    if ((roll -= weights[1]) < 0)
        return CONE_TYPE;
    return BUCKET_TYPE;
}

// Original per-level ambush sizes (Board.cpp:4914-4929 pool, 5006-5019 sky):
// 3-1/3-2/4-1/4-2 and 5-1/5-2 get 2, everything else 3.
static int final_ambush_count()
{
    if (level.level_num == 21 || level.level_num == 22 ||
        level.level_num == 31 || level.level_num == 32 ||
        level.level_num == 41 || level.level_num == 42)
        return 2;
    return 3;
}

/*Pool final wave (Board::SpawnZombiesFromPool, Board.cpp:4910-4970): zombies
claw out of the water on distinct water tiles of cols 5-8 (rows 2-3 in the
original), wearing the duck tube, splash included. The clone has no
rise-from-water animation, so they start walking on the spot.*/
static void spawn_zombies_from_pool()
{
    vector<pair<int, int>> tiles;
    for (int row : level.playable_rows)
        if (level.row_has_water(row))
            for (int col = 5; col < HORIZ_BLOCK_COUNT; col++)
                tiles.push_back({row, col});
    int spawned = 0;
    while (spawned < final_ambush_count() && !tiles.empty())
    {
        int pick = rand(0, (int)tiles.size() - 1);
        const auto tile = tiles[pick];
        tiles.erase(tiles.begin() + pick);
        Zombie *z = init_zombie(pick_rising_zombie_type(), level.level_num);
        z->row = tile.first;
        z->x_location = cells[tile.first][tile.second].x2 - 90;
        z->y_location = cells[tile.first][0].y1 - 50;
        z->wave_id = level.cur_wave;
        wrap_in_duckytube(z); // plain family rises out of the pool on its tube
        reanimfx::spawn(reanimfx::FX_SPLASH, z->x_location + 85,
                        cells[tile.first][0].y1 + 55, tile.first);
        play_sound_effect(ZOMBIE_ENTERING_WATER_MUSIC_DIRECTORY);
        game_characters.zombies.push_back(z);
        spawned++;
    }
    if (spawned)
        play_sound_effect(GRAVESTONE_RUMBLE_MUSIC_DIRECTORY);
    level.zombie_count += spawned;
    level.used_zombie_count += spawned;
}

/*Roof final wave (Board::SpawnZombiesFromSky + SetupBungeeDrop,
Board.cpp:4979-5045): bungees hang over distinct tiles of cols 4-8 and
deliver a zombie each (the 5-5 bungee force_target/deliver machinery).*/
static void spawn_zombies_from_sky()
{
    vector<pair<int, int>> tiles;
    for (int row : level.playable_rows)
        for (int col = 4; col < HORIZ_BLOCK_COUNT; col++)
            tiles.push_back({row, col});
    int dropped = 0;
    while (dropped < final_ambush_count() && !tiles.empty())
    {
        int pick = rand(0, (int)tiles.size() - 1);
        const auto tile = tiles[pick];
        tiles.erase(tiles.begin() + pick);
        BungeeZombie *b = static_cast<BungeeZombie *>(init_zombie(BUNGEE_TYPE, level.level_num));
        b->force_target(tile.first, tile.second, pick_rising_zombie_type());
        b->wave_id = level.cur_wave;
        game_characters.zombies.push_back(b);
        // carrier + payload both spawn-counted, as the original's two
        // AddZombie calls do; the payload materializes when the rope lands.
        level.zombie_count += 2;
        level.used_zombie_count += 2;
        dropped++;
    }
}

/*Per-tick driver: when the armed ambush clock runs out, fire the map's
variant (Board.cpp:5481-5486): roof -> bungee drop, pool/fog -> pool rise,
night yard -> the grave rise.*/
void update_final_wave_ambush()
{
    if (level.final_ambush_clk <= 0 || --level.final_ambush_clk > 0)
        return;
    if (level.is_roof())
        spawn_zombies_from_sky();
    else if (level.has_water_rows())
        spawn_zombies_from_pool();
    else
        rise_zombies_from_graves(game_characters.zombies);
}
/*=== 2-5 Whack-a-Zombie (port of Challenge::WhackAZombieSpawning) ============
Wave zombies do not march in from the right edge: they claw out of the level's
grave stones in bursts of 1-3, on the original's own clock -- a fixed 20 s per
wave, bursts rising during the first 17 s, the final wave pouring its whole
(grave-count-capped) 20 lot 3 s in. The generated wave plans carry NO
zombies here (materialize skips them for the Whack special); every riser is
created fresh by the burst loop, so the level runs at the original's volume
(~130 zombies, not the ~32 the tiny plans hold). The graveyard GROWS like the
original's: 100 ticks before every wave rollover it gains max(1, 5 - standing
stones) new stones on free tiles (cols 3-8), so the 9 start stones become
~16 by the final pour (Challenge.cpp:2540-2544). Each riser fires the
WhackAZombieRise dirt burst + the dirt_rise foley like the original, and rolls
its own pace: mVelX = RandRangeFloat(0.5, PvzpAnimateCurve(1,12,wave,1,3,
CURVE_EASE_IN)) px/cs (Challenge.cpp:2600-2603, 2622-2623; final wave max 2).
*/

void go_to_next_wave(); // defined below with the regular wave logic

static int whack_drain_wave = -1;  // wave whose burst window is open (-1: none)
static int whack_window_clk = 0;   // ticks left in that wave's burst window
static int whack_burst_clk = 0;    // ticks until the next burst

static void whack_reset_state()
{
    whack_drain_wave = -1;
    whack_window_clk = 0;
    whack_burst_clk = 1;  // counter = 1 (Challenge.cpp:2554): first burst on
                          // the window's first tick
}

static vector<pair<int, int>> whack_grave_tiles()
{
    vector<pair<int, int>> graves;
    for (int r : level.playable_rows)
        for (int c = 0; c < HORIZ_BLOCK_COUNT; c++)
            if (cells[r][c].has_grave)
                graves.push_back({r, c});
    return graves;
}

/*A grave a Grave Buster is currently eating is not a spawn target (repvz
collects aGridPicks with a SEED_GRAVEBUSTER test, Challenge.cpp:2606-2611):
no burst riser and no final-pour riser may leave it, and the burst size / 20
pour are capped by that reduced count (aZombieCount = min(count,
aGridPicksCount), Challenge.cpp:2616).*/
static bool whack_grave_bustered(int row, int col)
{
    for (const Plants *p : game_characters.plants)
        if (p->get_row() == row && p->get_col() == col &&
            p->get_type() == GRAVEBUSTER_TYPE)
            return true;
    return false;
}

/*Riser spawn targets: the standing stones no Grave Buster is eating
(Challenge.cpp:2600-2616's aGridPicks).*/
static vector<pair<int, int>> whack_riser_tiles()
{
    vector<pair<int, int>> graves;
    for (const auto &g : whack_grave_tiles())
        if (!whack_grave_bustered(g.first, g.second))
            graves.push_back(g);
    return graves;
}

/*Grow new grave stones, with the original's placement (repvz
Challenge::WhackAZombiePlaceGraves, Challenge.cpp:2491-2532): any grass tile
cols 3-8 without a stone/crater is a candidate, EMPTY tiles are heavily
preferred (weight 100000 vs 1 for planted -- a plant only dies under the
rising stone on the rare planted pick), and the count comes from the caller:
the original passes max(1, 5 - standing stones) once per wave
(Challenge.cpp:2540-2544), so the graveyard never shrinks and always grows
by at least one stone per wave.*/
static void whack_kill_plants_at(int row, int col)
{
    for (size_t i = 0; i < game_characters.plants.size();)
    {
        Plants *p = game_characters.plants[i];
        if (p->get_row() == row && p->get_col() == col)
        {
            delete p;
            game_characters.plants.erase(game_characters.plants.begin() + i);
        }
        else
            i++;
    }
    cells[row][col].is_planted = false;
    cells[row][col].is_block_zombie = false;
}

static void whack_top_up_graves()
{
    struct Pick
    {
        int row, col, weight;
    };
    vector<Pick> picks;
    for (int r : level.playable_rows)
        for (int c = 3; c < HORIZ_BLOCK_COUNT; c++)
            if (!cells[r][c].is_water && !cells[r][c].has_grave &&
                !cells[r][c].has_crater)
                picks.push_back({r, c, cells[r][c].is_planted ? 1 : 100000});
    int need = max(1, 5 - (int)whack_grave_tiles().size());
    while (need-- > 0 && !picks.empty())
    {
        long total = 0;
        for (const auto &p : picks)
            total += p.weight;
        long ticket = (long)rand(0, (int)(total - 1));
        size_t idx = picks.size() - 1;
        for (size_t i = 0; i < picks.size(); i++)
            if ((ticket -= picks[i].weight) < 0)
            {
                idx = i;
                break;
            }
        whack_kill_plants_at(picks[idx].row, picks[idx].col);
        cells[picks[idx].row][picks[idx].col].has_grave = true;
        // Stone breaks out of the ground like the flag-wave sprout (the
        // original's AddAGraveStone also starts the stone at counter 0 --
        // GraveStoneRise dirt splash + scale-in).
        level.grave_grows.push_back({picks[idx].row, picks[idx].col,
                                     GRAVE_GROW_TICKS});
        const auto &b = cells[picks[idx].row][picks[idx].col];
        reanimfx::spawn(reanimfx::FX_GRAVE_SPROUT, (b.x1 + b.x2) / 2,
                        b.y2 - 12, picks[idx].row);
        picks.erase(picks.begin() + idx);
    }
}

/*Place one fresh whack riser onto its grave tile (RiseFromGrave position).
The whack riser stands up immediately (the original gives it the same
PHASE_RISING_FROM_GRAVE treatment, but the whack pacing wants instant
walkers); the hole gets the real WhackAZombieRise port -- dirt splash + big
clumps, no arm (compiled emitters DirtSplash/IMAGE_DIRTSMALL +
DirtClumps/IMAGE_DIRTBIG, repvz AddTodParticle PARTICLE_WHACK_A_ZOMBIE_RISE)
plus its FOLEY_DIRT_RISE (Zombie.cpp:8336), one burst per riser like the
original. The caller rolls the pace (WhackAZombieSpawning's RandRangeFloat
(0.5, aMaxSpeed) px/cs).*/
static void whack_spawn_zombie_from_grave(Zombie *z, const pair<int, int> &g)
{
    z->row = g.first;
    z->x_location = cells[g.first][g.second].x2 - 90;
    z->y_location = cells[g.first][0].y1 - 50;
    z->wave_id = whack_drain_wave;
    const auto &b = cells[g.first][g.second];
    reanimfx::spawn(reanimfx::FX_WHACK_ZOMBIE_RISE, (b.x1 + b.x2) / 2,
                    b.y2 - 12, g.first);
    play_sound_effect(DIRT_RISE_MUSIC_DIRECTORY);
    game_characters.zombies.push_back(z);
    level.wave_spawn_health += z->get_health();
    level.used_zombie_count++;
    level.zombie_count++;
}

/*Per-tick whack driver: the burst clock of the wave whose window
create_new_zombies has opened (whack_drain_wave). Grave upkeep is NOT here:
the original tops the graveyard up on the wave clock's countdown == 100 cs
milestone (Challenge.cpp:2540-2544), which create_new_zombies fires 100
ticks before every wave rollover.*/
static void whack_update()
{
    // Level (re)start under an armed window: the wave bookkeeping reset to
    // 0 while the stale window still points at the old run's wave. A live
    // window always tracks cur_wave (cur = window's wave, or the wave
    // after it once the 20 s clock has rolled on).
    if (whack_drain_wave >= 0 &&
        level.cur_wave != whack_drain_wave +
                              (whack_drain_wave == level.final_wave ? 0 : 1))
        whack_reset_state();

    if (whack_drain_wave < 0)
        return;
    const int wave = whack_drain_wave + 1; // 1-based wave number, for the curves
    const bool final = whack_drain_wave == level.final_wave;

    // The burst window counts down with the wave (repvz: bursts rise while
    // mZombieCountDown >= 300 cs of the 2000 cs wave clock); the final wave
    // spends the whole window as its pour delay (300 cs) instead.
    if (whack_window_clk > 0 && --whack_window_clk == 0)
    {
        if (final)
        {
            // Final pour (repvz Challenge.cpp:2573-2578, 2616-2647): 20
            // cone/pail risers, one per standing stone (capped by the
            // stone count, and stones under a Grave Buster are out of the
            // pick list), each rolling its own type 50/50 and the capped
            // pace rand(0.5, 2) px/cs. mZombieCountDown = 0 right after
            // (Challenge.cpp:2643-2646): the level ends when the last one
            // drops.
            vector<pair<int, int>> graves = whack_riser_tiles();
            if (graves.empty())
            {
                whack_top_up_graves();
                graves = whack_riser_tiles();
            }
            for (size_t gi = 0; gi < graves.size() && gi < 20; gi++)
            {
                int want = rand(0, 1) == 0 ? CONE_TYPE : BUCKET_TYPE;
                Zombie *z = init_zombie(want, level.level_num);
                z->step_speed = CS_VEL(rand_float(0.5f, 2.0f));
                whack_spawn_zombie_from_grave(z, graves[gi]);
            }
            go_to_next_wave(); // cur_wave == final_wave: waves_finished
        }
        whack_drain_wave = -1; // window spent: quiet until the 20 s clock
                               // brings the next wave
        return;
    }
    // The final wave's window is ONLY the pour delay: mChallengeStateCounter
    // starts at 300 cs there (Challenge.cpp:2554) and nothing but the pour
    // ever fires off it -- no bursts rise during the delay.
    if (final)
        return;
    if (--whack_burst_clk > 0)
        return;
    // Burst cadence (repvz Challenge.cpp:2628-2632): rand between
    // PvzpAnimateCurve(1,12,wave,100,30,LINEAR) and (...,200,60,LINEAR) cs
    // = 60..18 / 120..36 ticks over the original's 12-wave curve (the
    // adventure whack's 8 waves ride the same curve, verbatim).
    {
        float t = (wave - 1) / 11.0f;
        int lo = (int)(60.0f + (18.0f - 60.0f) * t + 0.5f);
        int hi = (int)(120.0f + (36.0f - 120.0f) * t + 0.5f);
        whack_burst_clk = lo + rand(0, hi - lo);
    }

    // First burst of the level: sound effect "The zombies are coming".
    if (level.zombie_has_coming == false)
    {
        play_sound_effect(ZOMBIE_COMING_MUSIC_DIRECTORY);
        level.zombie_has_coming = true;
    }
    play_sound_effect(GRAVESTONE_RUMBLE_MUSIC_DIRECTORY);

    vector<pair<int, int>> graves = whack_riser_tiles();
    if (graves.empty())
    {
        whack_top_up_graves();
        graves = whack_riser_tiles();
        if (graves.empty())
            return; // no room anywhere: try again next burst
    }
    // Burst size/type come from the original's escalation phase tables
    // (repvz Challenge.cpp:2563-2568): aPhase = clamp((wave-1)*6/12, 0, 5);
    // one riser per standing stone (the weighted grave pick zeroes a stone's
    // weight after its riser, repvz Challenge.cpp:2608-2616).
    const int WHACK_DOUBLE_CHANCE[6] = {0, 30, 10, 10, 15, 18};
    const int WHACK_TRIPLE_CHANCE[6] = {0, 0, 0, 0, 10, 13};
    const int WHACK_PAIL_CHANCE[6] = {0, 0, 0, 10, 15, 15};
    const int WHACK_CONE_CHANCE[6] = {0, 0, 30, 30, 30, 30};
    int phase = min((wave - 1) * 6 / 12, 5);
    int now = 1;
    {
        int hit = rand(0, 99);
        if (hit < WHACK_TRIPLE_CHANCE[phase])
            now = 3;
        else if (hit < WHACK_TRIPLE_CHANCE[phase] + WHACK_DOUBLE_CHANCE[phase])
            now = 2;
    }
    now = min(now, (int)graves.size());
    // Non-final bursts share one type roll (type rolled once per burst).
    int want = NORMAL_TYPE;
    {
        int type_hit = rand(0, 99);
        if (type_hit < WHACK_PAIL_CHANCE[phase] && now < 3)
            want = BUCKET_TYPE;
        else if (type_hit < WHACK_PAIL_CHANCE[phase] + WHACK_CONE_CHANCE[phase])
            want = CONE_TYPE;
    }
    // Riser pace: mVelX = RandRangeFloat(0.5, PvzpAnimateCurve(1,12,wave,
    // 1,3,CURVE_EASE_IN)) px/cs (repvz Challenge.cpp:2600-2603); EASE_IN is
    // the quad warp (PvzpCommon.h PvzpCurveQuad: t*t).
    float ct = (wave - 1) / 11.0f;
    float max_speed = 1.0f + 2.0f * ct * ct;
    for (; now > 0; now--)
    {
        int gi = rand(0, (int)graves.size() - 1);
        const pair<int, int> g = graves[gi];
        graves.erase(graves.begin() + gi); // distinct stones per burst
        Zombie *z = init_zombie(want, level.level_num);
        z->step_speed = CS_VEL(rand_float(0.5f, max_speed));
        whack_spawn_zombie_from_grave(z, g);
    }
}

/*Ice gate for bobsled spawns: the row has a live zomboni trail right now
(repvz Board::PickRowForZombie rejects BOBSLED when mIceTimer[row] == 0).*/
static bool row_has_live_ice(int row)
{
    return row >= 0 && row < (int)level.ice_timer.size() &&
           level.ice_timer[row] > 0 && row < (int)level.ice_min_x.size() &&
           level.ice_min_x[row] >= 0;
}

/*Per-mille lane weight after a mower fired on the row (repvz Board.cpp:
2710-2725, PickRowForNewZombie's mWaveRowGotLawnMowered rule): a just-mowed
lane is near-barren for the next wave (x0.01), half weight the wave after,
and normal again from wave +3.*/
static int mower_relief_weight(int row)
{
    if (row < 0 || row >= (int)level.row_mowed_wave.size())
        return 1000;
    int mow = level.row_mowed_wave[row];
    if (mow < 0)
        return 1000;
    int diff = level.cur_wave - mow;
    if (diff <= 1)
        return 10;
    if (diff <= 2)
        return 500;
    return 1000;
}

/*Wave zombies pre-roll their lanes at level load; the mower-relief rule needs
the mow state at spawn time, so a zombie standing on a dampened lane re-rolls
its lane here (weighted by mower_relief_weight; bobsled riders pile onto
their lead's new lane, and a bobsled only lands on live ice).*/
static void relieve_mowed_lanes()
{
    int bobsled_row = -1; // lead lane of the bobsled team currently passing
    for (const auto &zombie : current_wave.zombies)
    {
        if (zombie->type == BOBSLED_TYPE && bobsled_row >= 0)
        {
            zombie->row = bobsled_row;
            zombie->y_location = cells[zombie->row][0].y1 - 50;
            continue;
        }
        if (zombie->type != BOBSLED_TYPE)
            bobsled_row = -1;
        if (mower_relief_weight(zombie->row) >= 1000)
        {
            if (zombie->type == BOBSLED_TYPE)
                bobsled_row = zombie->row;
            continue;
        }
        vector<int> lanes = lanes_for_zombie(zombie->type, level.cur_wave);
        if (zombie->type == BOBSLED_TYPE)
        {
            vector<int> iced;
            for (int r : lanes)
                if (row_has_live_ice(r))
                    iced.push_back(r);
            if (!iced.empty())
                lanes = iced;
        }
        int wsum = 0;
        for (int r : lanes)
            wsum += mower_relief_weight(r);
        int roll = rand(0, wsum - 1);
        for (int r : lanes)
        {
            if (roll < mower_relief_weight(r))
            {
                zombie->row = r;
                zombie->y_location = cells[r][0].y1 - 50;
                break;
            }
            roll -= mower_relief_weight(r);
        }
        if (zombie->type == BOBSLED_TYPE)
            bobsled_row = zombie->row;
    }
}

void add_zombies_from_level_data()
{
    // First wave of zombie: sound effect "The zombies are coming".
    if (level.zombie_has_coming == false)
    {
        play_sound_effect(ZOMBIE_COMING_MUSIC_DIRECTORY);
        level.zombie_has_coming = true;
    }

    // Mower-lane spawn relief: re-roll lanes off recently mowed rows
    // (Board.cpp:2710-2725) before the wave lands.
    relieve_mowed_lanes();

    // Add zombies from level data to game characters.
    // Tag them with their wave and remember the wave's spawn health budget
    // (used by the early-trigger rule below).
    level.wave_spawn_health = 0;
    for (size_t zi = 0; zi < current_wave.zombies.size(); zi++)
    {
        Zombie *zombie = current_wave.zombies[zi];
        // Bobsled teams ride live ice. PickRowForZombie only lands a sled
        // pick on an iced row (repvz Board.cpp:2654), and the spawn-time
        // fallback swaps a team for FOUR plain walkers when no row carries
        // a trail anymore (Board.cpp:4927-4933 via CanAddBobSled,
        // Board.cpp:2597-2607 -- ANY row's ice counts, not just the pick's
        // own lane; the old delete dropped 4 zombies per team instead).
        if (zombie->type == BOBSLED_TYPE && !row_has_live_ice(zombie->row))
        {
            size_t ze = zi; // the team spawns as one contiguous run (leader + mates)
            while (ze < current_wave.zombies.size() &&
                   current_wave.zombies[ze]->type == BOBSLED_TYPE)
                ze++;
            int iced = -1;
            for (int r = 0; r < (int)level.ice_timer.size(); r++)
                if (row_has_live_ice(r))
                {
                    iced = r;
                    break;
                }
            if (iced >= 0)
            {
                // Another lane still carries a trail: the whole team moves
                // there (the iced-row pick it should have been).
                for (size_t k = zi; k < ze; k++)
                {
                    current_wave.zombies[k]->row = iced;
                    current_wave.zombies[k]->y_location = cells[iced][0].y1 - 50;
                }
            }
            else
            {
                // No ice anywhere: four plain walkers take the team's
                // place (Board.cpp:4927-4933).
                for (size_t k = zi; k < ze; k++)
                {
                    Zombie *b = current_wave.zombies[k];
                    Zombie *n = init_zombie(NORMAL_TYPE, level.level_num, 0,
                                            level.cur_wave);
                    n->row = b->row;
                    n->y_location = cells[n->row][0].y1 - 50;
                    delete b;
                    current_wave.zombies[k] = n;
                    level.count_zombie_type[BOBSLED_TYPE]--;
                    level.count_zombie_type[NORMAL_TYPE]++;
                }
            }
            zombie = current_wave.zombies[zi]; // may have been replaced
        }
        zombie->wave_id = level.cur_wave;
        level.wave_spawn_health += zombie->get_health();
        game_characters.zombies.push_back(zombie);
    }

    // Number of zombies for this wave.
    level.used_zombie_count += current_wave.zombies.size();

    // Arm the next wave's early-trigger threshold (PvZ-Portable
    // Board.cpp:5350-5376, running after SpawnZombieWave()'s mCurrentWave++
    // so the checks look at the wave coming NEXT): a random 50-65% of this
    // wave's spawn health -- but the wait into a flag wave gets 0 (full
    // clear only) alongside its 45 s breather, and its countdown is
    // ZOMBIE_COUNTDOWN_BEFORE_FLAG (level_data.cpp materialize). Bowling
    // (1-5) is excluded from both the flag wait and the 0-health rule
    // (Board.cpp:5360): its final wave fires on the normal timer once the
    // lawn is chewed to the roll.
    if (level.cur_wave + 1 < level.wave_count)
        level.wave_trigger_health = level.waves[level.cur_wave + 1].has_flag &&
                                             level.seed_mode != "bowling"
                                         ? 0.0f
                                         : rand_float(0.5f, 0.65f) * level.wave_spawn_health;

    // Final wave: arm the ambush beat (PvZ-Portable Board.cpp:4941-4943 sets
    // mRiseFromGraveCounter = 200 when the last wave spawns; it fires 2.0 s
    // later -- 120 ticks at 60 tps). update_final_wave_ambush() picks the
    // map's variant: roof bungee drop, pool rise, or the night grave rise --
    // graves open on the final wave ONLY, not every flag wave. (Adventure
    // flag waves never sprout NEW stones -- that stone-per-flag-round rule
    // is Survival-Night only, repvz Challenge.cpp:3026-3033.)
    if (level.cur_wave == level.final_wave)
        level.final_ambush_clk = 120;
}

void go_to_next_wave()
{    // Next wave
    if (level.cur_wave + 1 < level.wave_count)
    {
        level.cur_wave++;
        level.next_wave_clk = level.waves[level.cur_wave].delay_time;
        level.next_wave_clk_start = level.next_wave_clk; // armed countdown (early-trigger floor)
        level.zombie_delay_state = NO_DELAY;
        level.announcer.state = ANNOUNCED;
    }
    // waves finished
    else
        level.waves_finished = true;
}

/*Spawn the current wave when its countdown ends.
    + Flag waves and the final wave go through the announcer first.
    + The countdown to the next wave is cut short when the current wave dies out
      or its remaining health drops below the threshold rolled at its spawn
      (a random 50-65% of spawn health; flag-wave waits clear fully).
*/
void create_new_zombies()
{
    // Get zombies for current wave.
    current_wave = level.waves[level.cur_wave];
    // Whack runs a fixed 20 s wave clock (WhackAZombieSpawning); no early
    // trigger, no empty-lawn chaining -- the next wave is always on time.
    if (level.cur_wave > 0 && !level.is_special(LevelSpecial::Whack))
    {
        // Early trigger (PvZ-Portable Board.cpp:5335 = repvz
        // Board.cpp:5589): cut the remaining countdown to 2 s once the
        // wave currently on the lawn is chewed down to the threshold
        // rolled when it spawned (mZombieHealthToNextWave: a random
        // 50-65% of its spawn health, 0 = full clear only into flag
        // waves) -- but only after at least 400 cs (240 ticks) of this
        // wave's wait have elapsed:
        // mZombieCountDown > 200 &&
        // mZombieCountDownStart - mZombieCountDown > 400 &&
        // TotalZombiesHealthInWave(mCurrentWave - 1) <= mZombieHealthToNextWave.
        // The empty lawn is the health-0 case of the same rule (the old
        // separate instant chain let waves land back-to-back in under a
        // second on overkill -- the original never skips the 200 cs floor).
        int rest_health = 0;
        for (const auto &zombie : game_characters.zombies)
            if (zombie->wave_id == level.cur_wave - 1)
                rest_health += zombie->get_health();
        if (level.wave_spawn_health > 0 &&
            (float)rest_health <= level.wave_trigger_health &&
            level.next_wave_clk > 120 &&
            level.next_wave_clk_start - level.next_wave_clk > 240)
            level.next_wave_clk = 120;
    }
    if (level.next_wave_clk > 0)
    {
        level.next_wave_clk--;
        // 2-5 whack: the fixed 20 s wave clock's milestone beats, all on
        // mZombieCountDown (repvz Challenge::WhackAZombieSpawning +
        // Board::NextWaveComing):
        // + ==100: the graveyard tops up with max(1, 5 - standing stones)
        //   new stones (Challenge.cpp:2540-2544) -- it GROWS by at least one
        //   stone every wave (9 start -> ~16 by the final pour), and the
        //   pre-wave clock never tops up (mCurrentWave > 0);
        // + ==5: NextWaveComing (Board.cpp:5214-5232) -- the opening awooga
        //   horn on the pre-wave clock, and the FINAL WAVE banner + siren
        //   five ticks before the last wave's window opens; the wave clock
        //   itself never pauses for the banner.
        if (level.is_special(LevelSpecial::Whack))
        {
            if (level.next_wave_clk == 100 && level.cur_wave > 0)
                whack_top_up_graves();
            if (level.next_wave_clk == 5)
            {
                if (level.cur_wave == 0)
                    play_sound_effect(AWOOGA_MUSIC_DIRECTORY);
                else if (level.cur_wave == level.final_wave)
                {
                    level.announcer = Announcer(FINAL_WAVE_DIRECTORY, FINAL_WAVE_MUSIC_DIRECTORY);
                    play_sound_effect(SIREN_MUSIC_DIRECTORY);
                }
            }
        }
        return;
    }
    if (level.is_special(LevelSpecial::Whack))
    {
        // 2-5: each wave bursts out of the graves on its own clock (repvz
        // Challenge::WhackAZombieSpawning) -- the fixed 20 s wave timer
        // runs in the regular countdown while whack_update() raises bursts
        // during the first 17 s; the final wave pours 3 s in and ends the
        // level's wave list from whack_update(). Whack NEVER enters the
        // flag/final announcer intercepts below: the original's last wave
        // lands straight on the 20 s beat, banner already shown at ==5
        // (mChallengeStateCounter = 300 is the only final-wave delay).
        if (whack_drain_wave != level.cur_wave) // arm once per wave
        {
            level.wave_spawn_health = 0;
            whack_drain_wave = level.cur_wave;
            whack_burst_clk = 1; // counter = 1 (Challenge.cpp:2554): first
                                 // burst on the window's first tick
            if (level.cur_wave == level.final_wave)
                whack_window_clk = 180;  // 300 cs pour delay
            else
            {
                whack_window_clk = 1020; // 1700 cs burst window (mZombieCountDown >= 300 cs)
                go_to_next_wave();       // roll the 20 s clock onto the NEXT wave
            }
        }
        return;
    }
    // Huge/final waves spawn right on schedule once announced: the original
    // never waits for the lawn to thin out -- the wave lands exactly when the
    // 7.5 s announce countdown (mHugeWaveCountDown = 750, repvz
    // Board.cpp:5490-5492,5568-5570) expires.
    if (level.zombie_delay_state == FINAL_WAVE)
    {
        if (level.huge_wave_clk > 0)
            level.huge_wave_clk--;
        if (level.announcer.state == ANNOUNCED)
        {
            // One banner only, like the original's single
            // [ADVICE_HUGE_WAVE]/siren beat for the last flag wave
            // (Board.cpp:5562-5570).
            level.announcer = Announcer(FINAL_WAVE_DIRECTORY, FINAL_WAVE_MUSIC_DIRECTORY);
            play_sound_effect(SIREN_MUSIC_DIRECTORY);
        }
        // The banner runs 300 ticks; the wave spawns once both the banner and
        // the 7.5 s announce countdown (450 ticks) are through -- i.e. ~7.5 s
        // after the banner pops up.
        if (level.announcer.state == ANNOUNCING && level.announcer.is_finished() &&
            level.huge_wave_clk <= 0)
        {
            level.zombie_delay_state = AFTER_ANNOUNCE;
            level.announcer.state = ANNOUNCED;
        }
        else
            return;
    }

    if (level.zombie_delay_state == HUGE_WAVE)
    {
        if (level.huge_wave_clk > 0)
            level.huge_wave_clk--;
        if (level.announcer.state == ANNOUNCED)
        {
            level.announcer = Announcer(HUGE_WAVE_DIRECTORY, HUGE_WAVE_MUSIC_DIRECTORY);
        }
        // The banner runs 300 ticks; the wave spawns once both the banner and
        // the 7.5 s announce countdown (450 ticks) are through.
        if (level.announcer.state == ANNOUNCING && level.announcer.is_finished() &&
            level.huge_wave_clk <= 0)
        {
            level.zombie_delay_state = AFTER_ANNOUNCE;
            level.announcer.state = ANNOUNCED;
        }
        else
            return;
    }

    // Final wave
    if (level.zombie_delay_state == NO_DELAY)
    {
        if (level.cur_wave == level.final_wave)
        {
            level.zombie_delay_state = FINAL_WAVE;
            level.huge_wave_clk = 450; // 7.5 s (750 cs) announce, Board.cpp:5568
            return;
        }
        // Huge wave
        else if (current_wave.has_flag)
        {
            level.zombie_delay_state = HUGE_WAVE;
            level.huge_wave_clk = 450; // 7.5 s (750 cs) announce, Board.cpp:5568
            return;
        }
    }

    add_zombies_from_level_data();
    go_to_next_wave();
}

/*Create new wave of zombies.
If level has finised and that's time to create new wave:
        + first wave
        + there's no zombie
        + long time no seen new zombies
*/
void update_new_wave_zombies()
{
    if (level.waves_finished)
        return;
    // Vasebreaker ships no waves at all: its zombies come out of the pots.
    if (level.waves.empty())
        return;
    // Zomboss fight (5-10): no regular waves; the boss controller drops the
    // reinforcements itself. (waves_finished is also set on spawn; this guard
    // only covers the very first ticks.)
    if (level.is_special(LevelSpecial::Boss))
        return;

    if (level.is_special(LevelSpecial::Whack))
        whack_update(); // grave upkeep + burst spawning of the current wave

    create_new_zombies();
    return;
}

/*Handle all movement is happening: zombies, suns, peas.*/
void handle_movements()
{
    move_zombies(game_characters.zombies);
    move_suns(game_characters.suns);
    move_bullets(game_characters.bullets);
}

/*Update icons remainning time. Sun count blink*/
void update_plant_seeds_remaining_time()
{
    // Update icons remainning time
    for (auto &seed_packet : player.seed_packets)
    {
        if (seed_packet.remaining_time)
        {
            seed_packet.remaining_time--;
        }
    }
    // Sun count blink
    if (player.sun_count_change_color_times)
    {
        if (player.sun_count_change_color_time)
        {
            player.sun_count_change_color_time--;
        }
        else
        {
            player.sun_count_change_color_time = SUN_CHANGE_COLOR_CLK_COUNT;
            player.sun_count_change_color_times--;
        }
    }
}