#include "rolling_nut.hpp"
#include "draw/ImageType.hpp"
#include "draw/explosion_fx.hpp"
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/conveyor/conveyor.hpp"
#include "elements/elements.hpp"
#include "game_param.hpp"
#include "music/music.hpp"
#include "support.hpp"
#include <cmath>

extern Level level;
extern Elements game_characters;
extern Map cells;
extern Window win;

vector<RollingNut> rolling_nuts;

const float NUT_VX = 2.5f;  // px/tick rolling speed
const float NUT_VY = 3.0f;  // px/tick lane-drift speed after a ricochet
const int NUT_HALF = 25;    // off-edge despawn margin past the right window edge
const int NUT_SIZE = 66;    // rendered height (~0.8x a planted Wall-nut's 84px)
// Bowling damage (original Plant::UpdateBowling, 20 orig hp = 1 clone hp):
// an unarmored zombie (and a screen door, while the nut rolls) takes the
// full 1800 insta (= 90); a helm soaks 900 (= 45) and a shield (newspaper,
// ladder) 400 (= 20) -- armor-only overflow is lost (TakeHelmDamage /
// TakeShieldDamage never spill into the body), so cone/pail/football die in
// 2/3/3 hits, paper/ladder in 2/3, everything else in 1.
const int NUT_KILL_DAMAGE = 90;   // 1800 orig
const int NUT_HELM_DAMAGE = 45;   // 900 orig
const int NUT_SHIELD_DAMAGE = 20; // 400 orig
const float NUT_BODY_FLOOR = 13.5f; // armor soak stops at the body (270 orig)
const int NUT_SPIN = 5;     // degrees/tick: arc speed matches NUT_VX over the ~33px radius

// Blast visual: the Pow.png comic flash + the POWIE caption.
// (The original paints a custom particle burst + screen jiggle; the planted
// cherry bomb's full explosion sheet is NOT reused here.)
const int NUT_BOOM_CLK = FPS * 3 / 4;      // ~0.75 s of blast visuals

static float row_center(int row)
{
    return (cells[row][0].y1 + cells[row][0].y2) / 2.f;
}

void reset_rolling_nuts()
{
    rolling_nuts.clear();
}

/*Drop the nut where the player clicked; it starts rolling right from there.*/
void launch_rolling_nut(const int &plant, const int &row, const int &x)
{
    if (plant < 0 || row < 0)
        return;
    RollingNut nut;
    nut.plant = plant;
    nut.row = row;
    nut.x = (float)x;
    nut.y = row_center(row);
    rolling_nuts.push_back(nut);
    play_sound_effect(BOWLING_MUSIC_DIRECTORY);
}

/*Explode-o-nut blast: burnt death for every zombie in the 3x3 area
(same range math as the planted Cherry Bomb).*/
static void explode_nut(RollingNut &nut)
{
    play_sound_effect(CHERRYBOMB_MUSIC_DIRECTORY);
    nut.boom_row = nut.row;
    nut.boom_col = max(0, min(HORIZ_BLOCK_COUNT - 1, get_block_col((int)nut.x)));
    int right_limit = cells[nut.row][nut.boom_col].x2 + BLOCK_WIDTH;
    int left_limit = cells[nut.row][nut.boom_col].x1 - BLOCK_WIDTH - 50;
    auto &zombies = game_characters.zombies;
    for (int j = (int)zombies.size() - 1; j >= 0; j--)
        if (is_in(nut.row - 1, zombies[j]->row, nut.row + 1) &&
            is_in(left_limit, zombies[j]->x_location + ZOMBIE_EXACT_LOCATION + 15, right_limit))
        {
            push_burned_corpse(game_characters.zombie_parts, zombies[j]);
            delete zombies[j];
            zombies.erase(zombies.begin() + j);
        }
    nut.explode_clk = NUT_BOOM_CLK;
}

/*Damage of one rolling Wall-nut hit (original Plant::UpdateBowling; see the
constants above). The armor layers sop up a capped amount with the overflow
lost, so armored zombies take several hits; bare ones are one-shotted.*/
static int nut_damage_for(Zombie *z)
{
    // Health is float on the 20:1 scale (270 -> 13.5); the per-hit loop chews
    // whole points, so the armor soak rounds up to still reach the floor.
    const float hp = z->get_health();
    switch (z->type)
    {
    case CONE_TYPE:
    case BUCKET_TYPE:
    case FOOTBALL_TYPE:
        if (hp > NUT_BODY_FLOOR)
            return (int)ceilf(min((float)NUT_HELM_DAMAGE, hp - NUT_BODY_FLOOR));
        return NUT_KILL_DAMAGE;
    case NEWSPAPER_TYPE:
    case LADDER_TYPE:
        if (hp > NUT_BODY_FLOOR)
            return (int)ceilf(min((float)NUT_SHIELD_DAMAGE, hp - NUT_BODY_FLOOR));
        return NUT_KILL_DAMAGE;
    default:
        return NUT_KILL_DAMAGE;
    }
}

/*Wall-nut hit: armor-aware damage (see nut_damage_for), then ricochet into
an adjacent playable lane.*/
static void bash_zombie(RollingNut &nut, vector<Zombie *> &zombies, int zidx)
{
    play_sound_effect(BOWLING_IMPACT_MUSIC_DIRECTORY);
    const int dmg = nut_damage_for(zombies[zidx]);
    bool died = false;
    for (int i = 0; i < dmg && !died; i++)
        died = zombies[zidx]->decrease_health(game_characters.zombie_parts);
    if (died)
    {
        play_sound_effect(ZOMBIE_FALLING_MUSIC_DIRECTORY);
        zombies[zidx]->add_zombie_die(game_characters.zombie_parts);
        delete zombies[zidx];
        zombies.erase(zombies.begin() + zidx);
    }
    // Ricochet (original Plant::BowlingHitZombie): the collision lane switches
    // ON THE SPOT at the hit (mRow--/++ there); y then slides over visually.
    // Finish any in-flight slide first (its target lane is already nut.row).
    if (nut.vy != 0.0f)
    {
        nut.y = row_center(nut.row);
        nut.vy = 0.0f;
    }
    // The vertical direction REVERSES after every hit; a nut already in an
    // edge lane is forced back inward. Only the first hit picks a side at
    // random (same branch order as the original).
    int row0 = level.playable_rows.front();
    int rown = level.playable_rows.back();
    int dir;
    if (nut.row == rown || nut.vdir > 0)
        dir = -1;
    else if (nut.row == row0 || nut.vdir < 0)
        dir = 1;
    else
        dir = rand(0, 1) ? 1 : -1; // first hit, mid-lane: random side
    if (row0 != rown)
    {
        nut.vdir = dir;
        nut.row += dir;
        nut.vy = dir * NUT_VY;
    }
}

void update_rolling_nuts()
{
    auto &zombies = game_characters.zombies;
    for (int i = (int)rolling_nuts.size() - 1; i >= 0; i--)
    {
        RollingNut &nut = rolling_nuts[i];

        // Explosion visual playing out
        if (nut.explode_clk > 0)
        {
            if (--nut.explode_clk <= 0)
                rolling_nuts.erase(rolling_nuts.begin() + i);
            continue;
        }

        // Lane-drift visual after a ricochet: the collision lane (nut.row)
        // already switched at the hit; y slides toward its center.
        if (nut.vy != 0.0f)
        {
            nut.y += nut.vy;
            if ((nut.vy > 0.0f && nut.y >= row_center(nut.row)) ||
                (nut.vy < 0.0f && nut.y <= row_center(nut.row)))
            {
                nut.y = row_center(nut.row);
                nut.vy = 0.0f;
            }
        }

        // Roll right
        nut.x += nut.vx;
        nut.angle = (nut.angle + NUT_SPIN) % 360;
        if (nut.x > WINDOW_WIDTH + NUT_HALF) // off the right edge
        {
            rolling_nuts.erase(rolling_nuts.begin() + i);
            continue;
        }

        // Vertical settle gate (original Plant::UpdateBowling,
        // Plant.cpp:2385-2387): while the ricochet slide is more than 2 px
        // off the collision lane's center the nut tests NO zombie contact
        // (aDistToGrid = GridToPixelY(0, mRow) - mY).
        const int dist_to_grid = (int)row_center(nut.row) - (int)nut.y;
        if (dist_to_grid < -2 || dist_to_grid > 2)
            continue;

        // Zombie contact: the leftmost zombie in the nut's lane whose body
        // rect overlaps the nut's attack rect (original Plant::UpdateBowling
        // -> FindTargetZombie, Plant.cpp:2399 + 4770). On a bowling level
        // GetPlantAttackRect is Rect(mX, mY, mWidth - 20, mHeight)
        // (Plant.cpp:5163-5166) -- the 80-px plant box (mWidth = 80,
        // Plant.cpp:149) shrunk 20 px from the right. nut.x is that box's
        // center (mX + mWidth / 2, the aPosX of Plant.cpp:2402), so the
        // attack interval is [nut.x - 40, nut.x + 20]. The zombie's box is
        // its per-type body rect (Zombie::GetZombieRect, Zombie.cpp:8298;
        // clone zrect_left/zrect_right), and the test is the pure horizontal
        // interval overlap of GetRectOverlap (Board.cpp:8939-8943):
        // min(right edges) - max(left edges) >= 0. Among overlapping
        // zombies the original takes the left-most body rect (aWeight =
        // -aZombieRect.mX, Plant.cpp:4886).
        const int atk_l = (int)nut.x - 40;
        const int atk_r = atk_l + 60;
        int hit = -1;
        int hit_left = INF;
        for (int j = 0; j < (int)zombies.size(); j++)
        {
            if (zombies[j]->row != nut.row)
                continue;
            const int zl = zombies[j]->zrect_left();
            const int zr = zombies[j]->zrect_right();
            if (min(atk_r, zr) - max(atk_l, zl) >= 0 && zl < hit_left)
            {
                hit = j;
                hit_left = zl;
            }
        }
        if (hit == -1)
            continue;
        if (nut.plant == CHERRYBOMB_TYPE)
            explode_nut(nut);
        else
            bash_zombie(nut, zombies, hit);
    }
}

void display_rolling_nuts()
{
    // Bowling launch line (original: IMAGE_WALLNUT_BOWLINGSTRIPE at the right
    // edge of column 2, Challenge::DrawBackdrop Challenge.cpp:2416): nuts may
    // only be launched behind it. The 57x502 sheet carries the stripe in its
    // left 17 px (rest transparent); crop and stretch it over the 4 px line.
    if (belt.bowling)
    {
        int line_x = cells[0][2].x2; // right edge of column 2
        int y1 = cells[level.playable_rows.front()][0].y1;
        int y2 = cells[level.playable_rows.back()][0].y2;
        win.draw_png(BOWLING_STRIPE_DIRECTORY, 0, 0, 17, 502,
                     line_x - 2, y1, 4, y2 - y1);
    }
    for (const auto &nut : rolling_nuts)
    {
        if (nut.explode_clk > 0)
        {
            int elapsed = NUT_BOOM_CLK - nut.explode_clk;
            // Pow.png blast flash centered on the 3x3 torn area, ~200 px wide
            // (keeps the 136x123 aspect), shown for the whole caption time.
            int fx = cells[nut.boom_row][nut.boom_col].x1 - BLOCK_WIDTH;
            int fw = BLOCK_WIDTH * 3;
            int top_row = max(0, nut.boom_row - 1);
            int bot_row = min((int)cells.size() - 1, nut.boom_row + 1);
            int fy = cells[top_row][0].y1;
            int fh = cells[bot_row][0].y2 - fy;
            const int pw = world_px(200), ph = world_px(181);
            win.draw_png(POW_FX_DIRECTORY,
                         fx + fw / 2 - pw / 2, fy + fh / 2 - ph / 2, pw, ph);
            // Comic blast caption ("POWIE!") over the blast.
            draw_boom_caption(fx + fw / 2, fy + fh * 2 / 5, elapsed);
            continue;
        }
        int nut_img = nut.plant == CHERRYBOMB_TYPE ? EXPLODEONUT_BOWLING_DIRECTORY : WALLNUT_BOWLING_DIRECTORY;
        const int ns = world_px(NUT_SIZE);
        win.draw_png_width_scaled(nut_img,
                                  (int)nut.x - ns / 2, (int)nut.y - ns / 2,
                                  ns, nut.angle);
    }
}
