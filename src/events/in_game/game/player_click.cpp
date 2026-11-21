#include "player_click.hpp"
#include "draw/rsdl.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/Player/Player.hpp"
#include "elements/bowling/rolling_nut.hpp"
#include "elements/button/button.hpp"
#include "elements/conveyor/conveyor.hpp"
#include "elements/elements.hpp"
#include "elements/plants/support_plant.hpp"
#include "elements/plants/coffeebean/coffeebean.hpp"
#include "elements/vasebreaker/vasebreaker.hpp"
#include "events/mouse_cursor/mouse_cursor.hpp"
#include "game_param.hpp"
#include "music/music.hpp"
#include "reanim/one_shot_fx.hpp"
#include "support.hpp"
#include <algorithm>

using std::max;
using std::min;

extern Level level;
extern Elements game_characters;

extern Player player;
extern Map cells;
extern Window win;

/*Hammer damage of a single click in 2-5 Whack-a-Zombie (original
Challenge::MouseDownWhackAZombie: TakeHelmDamage 900 while a helmet is on,
instant kill on the bare body). Mapped onto the clone's health model
(normal body 10, cone helm 18, bucket helm 55): a helm hit strips at most
WHACK_DAMAGE of helm, the spared part keeps the helmet on. Resulting hit
counts match the original: normal 1, conehead 2 (helm + body), buckethead 3.*/
#define WHACK_DAMAGE 45

// Chance a whacked zombie drops a 75 sun (three 25-coins in the original,
// Board::CheckForGameOver-free loot roll in Zombie.cpp::DieNoLoot path).
#define WHACK_SUN_DROP_CHANCE 5 // %

/*Handle all user click:
    If player click on sun: handle sun click, then return;
    Else if player click on shovel: player is shoveling.
    Else if player is choosing a plant and click on a tile:
        specify plant type and create a new plant there.
*/
void handle_user_click(const int &mouse_x, const int &mouse_y)
{
    // If player click on sun: handle sun click, then return;
    bool money_pickup = false;
    if (pick_sun_if_clicked_on(mouse_x, mouse_y, &money_pickup))
    {
        // Marigold coin / yeti diamond pickups chime with the coin sound.
        play_sound_effect(money_pickup ? COIN_MUSIC_DIRECTORY : SUN_CLICK_MUSIC_DIRECTORY);
        return;
    }

    // 2-5 Whack-a-Zombie: the mallet swings on every yard click, hit or miss
    // (original Challenge::MouseDownWhackAZombie: the cursor reanim restarts
    // and FOLEY_SWING plays even when nothing is under it; picking a seed
    // packet keeps the hand cursor, so no swing there). A click landing on a
    // zombie then applies the helm-aware hammer blow below.
    if (level.is_special(LevelSpecial::Whack))
    {
        if (!is_a_plant_seed_clicked_on(mouse_x, mouse_y))
        {
            mouse_cursor().start_swing();
            play_sound_effect(SWING_MUSIC_DIRECTORY);
        }
        if (whack_zombie_if_clicked(mouse_x, mouse_y))
            return;
    }

    // 4-5 Vasebreaker: clicks smash the vases; there is no other input.
    if (level.is_special(LevelSpecial::Vasebreaker))
    {
        vasebreaker::break_vase_if_clicked(mouse_x, mouse_y);
        return;
    }

    // Else if player is shoveling: handle remove plant.
    if (player.is_shoveling)
    {
        if (click_is_in_frontyard(mouse_x, mouse_y))
        {
            remove_plant_if_clicked_on(game_characters.plants, mouse_x, mouse_y);
        }
        else
        {
            play_sound_effect(FLOOP_MUSIC_DIRECTORY); // cancel shovel, like the original
        }
        player.is_shoveling = false;
        return;
    }

    // Else if player click on shovel: player is shoveling.
    if (level.has_shovel && Shovel_bar.is_mouse_in(mouse_x, mouse_y))
    {
        play_sound_effect(SHOVEL_MUSIC_DIRECTORY);
        player.is_shoveling = true;
        remove_chosen_plant();
        return;
    }

    // Conveyor / bowling levels: the belt is the only source of packets.
    if (belt.active)
    {
        handle_belt_click(mouse_x, mouse_y);
        return;
    }

    // Else if player is choosing a plant and click on a tile:
    //     specify plant type and create a new plant there.
    if (is_a_plant_seed_clicked_on(mouse_x, mouse_y))
    {
        which_plant_is_chosen(mouse_y);
        return;
    }
    if (player.is_choosing_a_plant())
    {
        if (click_is_in_frontyard(mouse_x, mouse_y))
        {
            create_new_plant(mouse_x, mouse_y);
            return;
        }
        else
        {
            remove_chosen_plant();
            return;
        }
    }
}

// Pumpkin shell stacking helpers (defined near create_new_plant below).
static bool cell_has_pumpkin(const int &row, const int &col);
// Shared plant-on-tile rules for banked and conveyor packets (below).
static bool try_plant_type_on_tile(int type, const int &row, const int &col);

/*Belt levels (conveyor / bowling):
    + Click on a queued packet picks it up.
    + With a packet in hand, click on the lawn: conveyor plants it for free
      (single-use); bowling throws the nut rolling down that lane.
    + Click anywhere else: the packet goes back to the belt.
*/
void handle_belt_click(const int &mouse_x, const int &mouse_y)
{
    if (belt.try_pick(mouse_x, mouse_y))
        return;
    if (!player.is_choosing_a_plant())
        return;
    if (click_is_in_frontyard(mouse_x, mouse_y))
    {
        if (belt.bowling)
        {
            // 1-5 Bowling rule (original Challenge::CanPlantAt ->
            // PLANTING_NOT_PASSED_LINE): nuts launch only behind the red
            // line, i.e. in columns 0..2. Past the line the throw is refused.
            if (get_block_col(mouse_x) > 2)
            {
                win.show_announcer_text("YOU CANNOT PLANT PAST THE LINE!");
                return;
            }
            launch_rolling_nut(belt.take_pick(), get_block_row(mouse_x, mouse_y), mouse_x);
        }
        else
        {
            int row, col;
            determine_row_and_col_chosen_by_second_click(mouse_x, mouse_y, row, col);
            int ptype = belt.take_pick();
            if (ptype < 0)
                return;
            // Same tile rules as banked packets (try_plant_type_on_tile): a
            // conveyor packet still rides a pre-placed flower pot (the 5-10
            // boss board), shells a plant as a pumpkin, etc.
            if (!try_plant_type_on_tile(ptype, row, col))
                belt.push(ptype);
        }
    }
    else
        belt.cancel_pick();
}

/*
Check if the mouse is in ICON BAR (the bar that contains plant seed).
Only the slots that actually hold a seed packet count.
*/
bool is_a_plant_seed_clicked_on(const int &mouse_x, const int &mouse_y)
{
    if (player.seed_packets.empty())
        return false;
    // Now, just y2 of the last plant seed is not known.
    // x1, x2, y1 are known.
    if (!is_in(plant_seed[0].x1, mouse_x, plant_seed[0].x2) ||
        mouse_y < plant_seed[0].y1)
        return false;
    return (mouse_y <= plant_seed[player.seed_packets.size() - 1].y2);
}

/*
Change is_a_plant_chosen status and chosen plant
    Show announcement when player has not enough sun or plant has not refresh.
    Otherwise, determine which plant is chosen, play sound.
        Note that double choose on a plant seed will cancel the selection.
*/
void change_chosen_status(const int &i)
{
    if (i < 0 || i >= PLANT_COUNT)
        return;
    // The plant must have a seed packet this level.
    auto packet = player.seed_packets.end();
    for (auto it = player.seed_packets.begin(); it != player.seed_packets.end(); ++it)
        if (it->plant_name == i)
        {
            packet = it;
            break;
        }
    if (packet == player.seed_packets.end())
        return;
    if (player.sun_count < PLANT_SUN_COST[i])
    {
        // not enough sun
        remove_chosen_plant();
        player.sun_count_change_color_times = 4;
        play_sound_effect(BUZZER_MUSIC_DIRECTORY);
        win.show_announcer_text("YOU DO NOT HAVE ENOUGH SUNS!");
    }
    else if (packet->remaining_time)
    {
        // plant has not refresh
        remove_chosen_plant();
        win.show_announcer_text("YOU MUST WAIT FOR PLANT SEED TO REFRESH!");
    }
    else
    {
        if (SeedPacket::chosen_plant == i)
        {
            // double choose: remove chosen plant
            SeedPacket::chosen_plant = PLANT_COUNT;
        }
        else
        {
            // specify chosen plant
            SeedPacket::chosen_plant = (PlantType)i;
            play_sound_effect(SEED_LIFT_MUSIC_DIRECTORY);
            player.is_shoveling = false;
        }
    }
}

/*Find which plant is chosen.
For all plant seeds' coordinate.
 */
void which_plant_is_chosen(int mouse_y)
{
    for (int i = (int)player.seed_packets.size() - 1; i >= 0; i--)
        if (plant_seed[i].y1 < mouse_y && mouse_y < plant_seed[i].y2)
        {
            change_chosen_status(player.seed_packets[i].plant_name);
            break;
        }
}

/*
Check if mouse is in frontyard or not.
Rows are limited to the level's playable lanes.
On the roof the sloped columns' tiles hang roof_tile_dy px below the flat
lane grid (down to cells[...].y2 + 100 = 595 on the bottom row); bounding by
the flat grid alone refuses every click on the lower half of the roof.
*/
bool click_is_in_frontyard(const int &mouse_x, const int &mouse_y)
{
    int right_bound = cells[0][8].x2;
    int left_bound = cells[0][0].x1;
    int upper_bound = cells[level.playable_rows.front()][0].y1;
    int lower_bound = cells[level.playable_rows.back()][0].y2;
    if (GRID_ROOF)
        lower_bound += roof_tile_dy(0); // deepest slope step (cols 0..4 side)
    if (mouse_x > left_bound && mouse_x < right_bound &&
        mouse_y > upper_bound && mouse_y < lower_bound)
        return true;
    return false;
}

/*
Find which row and column is chosen by click
Optimize to O(1) =))
*/
void determine_row_and_col_chosen_by_second_click(const int &mouse_x, const int &mouse_y, int &row, int &col)
{
    row = get_block_row(mouse_x, mouse_y);
    col = get_block_col(mouse_x);
}

/*
Restore the status that player is not choosing a plant.
A packet picked off the belt goes back to the belt.
*/
void remove_chosen_plant()
{
    if (belt.active)
        belt.cancel_pick();
    SeedPacket::chosen_plant = PLANT_COUNT;
}

/*Check planting rules tied to terrain: craters are permanent holes, graves
only accept Grave Buster (which itself requires a grave), and water tiles
(pool/fog) accept aquatic plants plus any regular plant RIDING on a lily pad
(the support-plant rule of support_plant.hpp). Aquatic-only plants refuse
dry land.*/
bool tile_accepts_plant(int type, const int &row, const int &col)
{
    if (cells[row][col].has_crater)
    {
        win.show_announcer_text("YOU CANNOT PLANT ON A CRATER!");
        return false;
    }
    if (type == GRAVEBUSTER_TYPE)
    {
        if (!cells[row][col].has_grave)
        {
            win.show_announcer_text("GRAVE BUSTER EATS ONLY GRAVES!");
            return false;
        }
        return true;
    }
    if (cells[row][col].has_grave)
    {
        win.show_announcer_text("YOU CANNOT PLANT ON A GRAVE!");
        return false;
    }
    // Coffee Bean: needs a day-sleeping mushroom host on its tile, and only
    // one bean per tile (the first bean's wake-up is still playing out). The
    // bean check runs in its own pass: the host mushroom sits earlier in the
    // plant list, so a host-first early return would hide a brewing bean.
    if (type == COFFEEBEAN_TYPE)
    {
        for (auto &p : game_characters.plants)
            if (p->get_row() == row && p->get_col() == col && p->get_type() == COFFEEBEAN_TYPE)
            {
                win.show_announcer_text("A COFFEE BEAN IS ALREADY BREWING!");
                return false;
            }
        for (auto &p : game_characters.plants)
        {
            if (p->get_row() != row || p->get_col() != col)
                continue;
            if (is_day_sleepy_mushroom(p->get_type()) && !p->coffee_awake)
                return true;
        }
        win.show_announcer_text("COFFEE BEANS WAKE DOZING MUSHROOMS!");
        return false;
    }
    // Roof stages (background_map 6 = roof, 7 = roof_night): the bare roof
    // tile takes ONLY a flower pot; every other plant must ride on a pot that
    // is already down (the support-plant rule then handles the stacking).
    if ((level.map_type == 6 || level.map_type == 7) && type != FLOWERPOT_TYPE &&
        !cell_has_support(game_characters.plants, row, col))
    {
        win.show_announcer_text("PLANT A FLOWER POT FIRST!");
        return false;
    }
    bool aquatic = type >= 0 && type < (int)PLANT_IS_AQUATIC.size() && PLANT_IS_AQUATIC[type];
    if (cells[row][col].is_water && !aquatic &&
        !cell_has_support(game_characters.plants, row, col))
    {
        win.show_announcer_text("YOU NEED A LILY PAD FIRST!");
        return false;
    }
    if (!cells[row][col].is_water && aquatic)
    {
        win.show_announcer_text("AQUATIC PLANTS GO ONLY IN THE POOL!");
        return false;
    }
    return true;
}

/*Plant the plant in the given row and col.
Reset player_choosing_plant status.
Increase plant's remaining time.
Decrease player's sun count.
Tick that the tile is planted.
Play sound effect.
@return false when the grave/crater rules reject the tile.
*/
bool plant_new_plant(int type, const int &row, const int &col)
{
    if (type < 0 || type >= PLANT_COUNT)
        return false;
    if (!tile_accepts_plant(type, row, col))
    {
        remove_chosen_plant();
        return false;
    }
    remove_chosen_plant();
    // Conveyor-belt packets cost no sun and have no recharge.
    bool belt_seed = (level.seed_mode == "conveyor");
    if (!belt_seed)
    {
        for (auto &packet : player.seed_packets)
            if (packet.plant_name == type)
            {
                packet.reset_remaining_time();
                break;
            }
        player.sun_count -= PLANT_SUN_COST[type];
    }
    game_characters.plants.push_back(init_plant(type, row, col));
    cells[row][col].is_planted = true;
    // Spike Weed lies flat in its tile: zombies walk over it, only wheels
    // (zomboni/bobsled) and smashes interact -- handled by the plant itself.
    if (type != SPIKEWEED_TYPE)
        cells[row][col].is_block_zombie = true;
    play_sound_effect(PLANT_PLANT_MUSIC_DIRECTORY);
    reanimfx::spawn_planted_dirt(row, col); // planting burst: dirt ring on soil, splash on water
    return true;
}

/*Shell helper: true when the tile already wears a pumpkin shell (only one
shell per tile, per the original).*/
static bool cell_has_pumpkin(const int &row, const int &col)
{
    for (auto &p : game_characters.plants)
        if (p->get_type() == PUMPKIN_TYPE && p->get_row() == row && p->get_col() == col)
            return true;
    return false;
}

/*True when the tile holds a pumpkin shell and nothing else (the plant inside
died): a new main plant may be tucked back in.*/
static bool cell_only_pumpkin(const int &row, const int &col)
{
    bool pumpkin = false;
    for (auto &p : game_characters.plants)
        if (p->get_row() == row && p->get_col() == col)
        {
            if (p->get_type() == PUMPKIN_TYPE)
                pumpkin = true;
            else
                return false;
        }
    return pumpkin;
}

/*Check whether a stacked tile still has a free main slot: the chosen plant is
not a support, the tile holds a support (lily pad / flower pot) and no main
plant yet. A bare pumpkin shell does not fill the slot (see support_plant.hpp),
so a plant can be tucked into a shell that only rides a pad/pot.
Aquatic plants (tangle kelp, sea-shroom) may NOT ride a lily pad — retail
refuses them; they go straight into water instead.
See support_plant.hpp.*/
static bool stackable_support_tile(const int &type, const int &row, const int &col)
{
    return !is_support_plant(type) &&
           !(type >= 0 && type < (int)PLANT_IS_AQUATIC.size() && PLANT_IS_AQUATIC[type]) &&
           cell_has_support(game_characters.plants, row, col) &&
           !cell_has_inner(game_characters.plants, row, col);
}

/*Plant `type` at (row, col) honoring every stacking rule of a chosen packet:
a pumpkin shell goes over ANY occupied tile (unless it already wears one), a
refill slips into a hollow shell, a Coffee Bean stacks onto its mushroom host,
any regular plant rides a support (lily pad / flower pot) with a free main
slot. Returns false (with the announcer line shown) when the tile refuses the
plant; removal of the cursor pick is left to the caller / plant_new_plant.
Shared by create_new_plant (banked packets) and handle_belt_click (conveyor
packets) so the 5-10 pre-placed pots take plants from the belt exactly like
they take them from the seed bank (repvz Board::CanPlantAt ->
GRIDITEM_NONE on a pot without a plant).*/
static bool try_plant_type_on_tile(int type, const int &row, const int &col)
{
    if (row < 0 || col < 0)
        return false;
    if (cells[row][col].is_planted && type == PUMPKIN_TYPE && !cell_has_pumpkin(row, col))
    {
        // Pumpkin shell stacks over ANY occupied tile (main or support plant).
        return plant_new_plant(PUMPKIN_TYPE, row, col);
    }
    if (cells[row][col].is_planted && type != PUMPKIN_TYPE &&
        !is_support_plant(type) && cell_only_pumpkin(row, col))
    {
        // Re-filling a hollow shell: a pumpkin without a plant inside still
        // takes a new main plant.
        return plant_new_plant(type, row, col);
    }
    if (cells[row][col].is_planted && type == COFFEEBEAN_TYPE)
    {
        // Coffee Bean wakes a day-sleeping mushroom: allow stacking onto that
        // mushroom's tile (the bean withers away right after the wake-up);
        // tile_accepts_plant validates that a valid host is there.
        if (find_main_plant(game_characters.plants, row, col))
            return plant_new_plant(COFFEEBEAN_TYPE, row, col);
    }
    if (cells[row][col].is_planted && !stackable_support_tile(type, row, col))
    {
        win.show_announcer_text("YOU CANNOT PLANT ON A PLANTED TILE!");
        return false;
    }
    return plant_new_plant(type, row, col);
}

/*Find row and column then plant the plant.
    Cannot plant if the tile has planted (except a rider on a support plant).
    Second click to cancel choosing plant.
    Show notification if the tile has planted.
*/
void create_new_plant(const int &mouse_x, const int &mouse_y)
{
    int row, col;
    determine_row_and_col_chosen_by_second_click(mouse_x, mouse_y, row, col);
    if (!try_plant_type_on_tile(SeedPacket::chosen_plant, row, col))
        remove_chosen_plant();
}

/*
Remove plant if player is shoveling and has clicked on its tile.
A stacked tile (lily pad + rider) loses the RIDER first: scan the plant list
from the newest back (the rider was planted on top).
*/
void remove_plant_if_clicked_on(vector<Plants *> &plants, const int &mouse_x, const int &mouse_y)
{
    for (int i = (int)plants.size() - 1; i >= 0; i--)
    {
        if (is_click_made_in_element_block(plants[i]->get_row(), plants[i]->get_col(), mouse_x, mouse_y))
        {
            play_sound_effect(PLANT_PLANT_MUSIC_DIRECTORY);
            delete_plant(plants, i);
            return;
        }
    }
}

/*
2-5 Whack-a-Zombie hammer: hit the topmost (most recently spawned) zombie
under the cursor for a helm-aware blow. A kill has a small chance (5%) to
drop a 75 sun where the zombie stood.
*/
bool whack_zombie_if_clicked(const int &mouse_x, const int &mouse_y)
{
    vector<Zombie *> &zombies = game_characters.zombies;
    for (int i = (int)zombies.size() - 1; i >= 0; i--)
    {
        Zombie *z = zombies[i];
        if (z->is_charmed || z->freeze_time)
            continue;
        if (mouse_x < z->x_location || mouse_x > z->x_location + ZOMBIE_WIDTH ||
            mouse_y < z->y_location || mouse_y > z->y_location + ZOMBIE_HEIGHT)
            continue;
        const float body_hp = *ZOMBIE_HEALTH_LIMIT[NORMAL_TYPE].rbegin() * z->hp_scale();
        int hp = z->get_health();
        // Mallet impact sound by what it lands on (original: FOLEY_PLASTIC_HIT
        // on a cone helm, shield hit on a bucket helm, FOLEY_BONK on a bare
        // body).
        if ((float)hp > body_hp)
            play_sound_effect(z->type == BUCKET_TYPE ? SHIELD_HIT_MUSIC_DIRECTORY
                                                     : PLASTIC_HIT_SND_MUSIC_DIRECTORY);
        else
            play_sound_effect(BONK_MUSIC_DIRECTORY);
        // Helm-aware blow: a helmet soaks at most WHACK_DAMAGE per click, the
        // rest of the blow never bleeds into the body (original whack rule).
        // (manual ceil: hp is held as an int-counted float, body_hp may be x.375)
        int dmg = (float)hp > body_hp ? min(WHACK_DAMAGE, (int)(hp - body_hp + 0.999f)) : WHACK_DAMAGE;
        bool died = false;
        // Per-hp loop (same as the bowling nut) so every crossed degrade
        // threshold updates the armor visuals exactly once.
        for (int i = 0; i < dmg && !died; i++)
            died = z->decrease_health(game_characters.zombie_parts);
        if (died)
        {
            play_sound_effect(ZOMBIE_FALLING_MUSIC_DIRECTORY);
            z->add_zombie_die(game_characters.zombie_parts);
            // A whacked zombie sometimes drops a 75 sun (original drops three
            // 25 suns; one full-size icon carries the value here).
            if (rand(0, 99) < WHACK_SUN_DROP_CHANCE)
            {
                Sun drop{};
                drop.final_row = z->get_row();
                drop.final_col = max(0, min(8, get_block_col(z->x_location)));
                drop.y_location = cells[drop.final_row][0].y1 - 30;
                drop.value = 3 * NORMAL_SUN_VALUE;
                game_characters.suns.push_back(drop);
            }
            delete z;
            zombies.erase(zombies.begin() + i);
        }
        return true;
    }
    return false;
}

/*
Check if player click in the tile in 'row' and 'col'
*/
bool is_click_made_in_element_block(const int &row, const int &col, const int &mouse_x, const int &mouse_y)
{
    if (mouse_x > cells[row][col].x1 && mouse_x < cells[row][col].x2 &&
        mouse_y > cells[row][col].y1 && mouse_y < cells[row][col].y2)
        return true;
    return false;
}

/*
For all the suns to check if any sun is clicked on.
@return 'true' if sun is clicked on
*/
bool pick_sun_if_clicked_on(const int &mouse_x, const int &mouse_y, bool *is_money)
{
    for (auto &sun : game_characters.suns)
    {
        int row = sun.final_row;
        int col = sun.final_col;
        int right_bound = cells[row][col].x2;
        int left_bound = cells[row][col].x1;
        int upper_bound = sun.y_location;
        int lower_bound = sun.y_location + ELEMENT_HEIGHT;
        if (mouse_x > left_bound && mouse_x < right_bound &&
            mouse_y > upper_bound && mouse_y < lower_bound)
        {
            sun.is_clicked = true;
            sun.x_location = left_bound;
            if (sun.money_kind == MONEY_SEED_PACKET)
            {
                // Vasebreaker plant vase (retail COIN_USABLE_SEED_PACKET):
                // collecting hands the packet onto the cursor for
                // click-to-plant -- no sun, and the coin chime below plays.
                SeedPacket::chosen_plant = (PlantType)sun.seed_plant;
            }
            else
                player.sun_count += sun.value;
            if (is_money)
                *is_money = sun.money_kind != MONEY_NONE;
            return true;
        }
    }
    return false;
}
