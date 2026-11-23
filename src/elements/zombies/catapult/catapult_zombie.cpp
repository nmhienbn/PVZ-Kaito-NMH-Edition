#include "catapult_zombie.hpp"
#include "elements/Level/Level.hpp" // roof_slope_offset
#include "../roof/roof_shared.hpp"
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/plants/umbrellaleaf/umbrellaleaf.hpp" // ball deflect check
#include "game_param.hpp"
#include "music/music.hpp"
#include "reanim/reanim_zombie.hpp"

extern int game_state;
extern Window win;

namespace
{
// One ball every ~3 s, 20 balls per cart (original figures, 60 tps).
const int CATAPULT_AMMO = 20;
const int CATAPULT_SHOT_CLK = 180;
// Ball flight ~0.6 s; the lob arc peaks under the roof line.
const int BALL_TICKS = 36;
// Original basketball: 75 dmg vs a 300-hp plant = kill in 4 balls
// (repvz Projectile.cpp:25). On the clone's bite-unit scale (plain plant
// 6 hp) that is 1.5 units/ball: the impact alternates 2/1, 2/1... so the
// 4th ball takes a plain plant down.
// Parking line: the cart lobs once mPosX <= 650 of the original board
// (repvz UpdateZombieCatapult, Zombie.cpp:1588) = column 7. The clone's
// board is the original art shifted by BOARD_OFFSET (220 px), so the line
// sits mid-tile of column 7.
const int PARK_COL = 7;
const int PARK_LINE_DX = 50; // cells[PARK_COL].x1 + 50 == 650 + 220
} // namespace

CatapultZombie::~CatapultZombie()
{
}

void CatapultZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = *ZOMBIE_HEALTH_LIMIT[CATAPULT_TYPE].rbegin();
    directory_num = CATAPULT_WALK_DIRECTORY;
    ammo = CATAPULT_AMMO;
    shot_clk = CATAPULT_SHOT_CLK;
}

void CatapultZombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - dir_height);
    directory_num = CATAPULT_WALK_DIRECTORY;
    state = DRIVING;
}

/*Catapult target (repvz FindCatapultTarget, Zombie.cpp:1562-1578): the plant
of my row whose tile sits >= 100 px left of the cart's nose, skipping spiky
tiles (spikeweed pops the tires instead -- lobbing at it is wasted ammo);
the left-most column wins. -1 when nothing qualifies.*/
int CatapultZombie::target_col() const
{
    int best = -1;
    for (auto *p : game_characters.plants)
    {
        if (p->get_row() != row || p->get_type() == SPIKEWEED_TYPE)
            continue;
        int col = p->get_col();
        if (cells[row][col].x1 + 100 > x_location)
            continue; // too close to the nose: the arc cannot drop on it
        if (best < 0 || col < best)
            best = col;
    }
    return best;
}

bool CatapultZombie::on_move_tick()
{
    switch (state)
    {
    case DRIVING:
        // repvz UpdateZombieCatapult's PHASE_ZOMBIE_NORMAL: a rolling vehicle
        // (reaching plants crush under it, see on_reach_plant); it parks and
        // lobs only once past the column-7 line with a live target and ammo.
        if (ammo > 0 && x_location <= cells[row][PARK_COL].x1 + PARK_LINE_DX &&
            target_col() >= 0)
        {
            state = SHOOTING;
            is_moving = false;
            shot_clk = CATAPULT_SHOT_CLK;
            return true;
        }
        return false; // regular drive-in movement
    case SHOOTING:
    {
        if (shoot_anim > 0)
            shoot_anim--;
        if (ball_ticks >= 0)
        {
            // Ball in flight: impact at the end of the arc.
            if (--ball_ticks == 0)
            {
                // Umbrella leaf cover: the ball bounces off, no damage.
                if (!umbrella_protects(ball_row, ball_col))
                {
                    int p = plant_index_at(ball_row, ball_col);
                    if (p >= 0)
                    {
                        game_characters.plants[p]->set_attacked_time(MAX_TIME_BLINK);
                        for (int d = 0; d < ball_damage && p < (int)game_characters.plants.size(); d++)
                        {
                            game_characters.plants[p]->decrease_health();
                            if (game_characters.plants[p]->is_died())
                            {
                                delete_plant(game_characters.plants, p);
                                break;
                            }
                        }
                    }
                }
                ball_ticks = -1;
            }
        }
        if (--shot_clk > 0)
            return true;
        shot_clk = CATAPULT_SHOT_CLK;
        if (ammo <= 0)
        {
            // Magazine dry (repvz mSummonCounter == 0): back to the rolling
            // phase, crushing the lane like a zomboni.
            state = CRUSHING;
            is_moving = true;
            change_zombie_eating_status();
            return true;
        }
        int col = target_col();
        if (col < 0)
        {
            // Target gone mid-siege (repvz PHASE_CATAPULT_RELOADING with no
            // FindCatapultTarget): drive on and crush instead of idling.
            state = DRIVING;
            is_moving = true;
            change_zombie_eating_status();
            return true;
        }
        ammo--;
        shoot_anim = 30;
        ball_ticks = BALL_TICKS;
        ball_total = BALL_TICKS;
        ball_damage = (ball_fires++ % 2 == 0) ? 2 : 1; // 75-dmg balls: 4/plant
        ball_from_x = x_location + 60;
        ball_row = row;
        ball_col = col;
        ball_to_x = (cells[row][col].x1 + cells[row][col].x2) / 2;
        play_sound_effect(BASKETBALL_MUSIC_DIRECTORY);
        return true;
    }
    case CRUSHING:
        return false; // drive on; on_reach_plant crushes
    }
    return false;
}

bool CatapultZombie::on_reach_plant(const int &col)
{
    if (state == SHOOTING)
        return true; // parked: ignore plants, balls do the work
    // DRIVING or CRUSHING: the cart flattens whatever it rolls over (a
    // vehicle never parks on contact in the original).
    if (crush_plants_at(row, col))
        play_sound_effect(GARGANTUAR_THUMP_MUSIC_DIRECTORY);
    return true; // keeps rolling
}

bool CatapultZombie::can_eat_plant() const
{
    return false; // a vehicle never bites
}

void CatapultZombie::determine_appearance(vector<ZombiePart> &)
{
    // No degrade visuals: the attack flash is the damage cue.
}

void CatapultZombie::add_zombie_die(vector<ZombiePart> &zombie_parts)
{
    if (reanimz::ready())
    {
        zombie_parts.push_back(ZombiePart(reanimz::DEATH_PART_IMG, 1, row, x_location,
                                          ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                          reanimz::SKIN_CATAPULT, false, head_dropped));
        return;
    }
    Zombie::add_zombie_die(zombie_parts);
}

void CatapultZombie::render_zombie()
{
    if (state != anim_state)
    {
        anim_t = 0.0f;
        anim_state = state;
    }
    const char *seg = "anim_walk";
    bool loop = true;
    if (state == SHOOTING)
    {
        if (shoot_anim > 0)
        {
            seg = "anim_shoot";
            loop = false;
        }
        else
        {
            seg = "anim_idle";
        }
    }
    if (!reanimz::drawSkinSegment(reanimz::SKIN_CATAPULT, seg, anim_t,
                                  x_location, y_location + roof_slope_offset(x_location),
                                  cold_time > 0, attacked_time > 0, 0, 0, WORLD_SCALE, loop))
        Zombie::render_zombie();
    // Ball in flight: simple parabola between the basket and the target,
    // drawn with the retail basketball sprite (30x30) centered on the arc.
    if (ball_ticks >= 0)
    {
        int t = ball_total - ball_ticks;
        float f = (float)t / (float)ball_total;
        int bx = ball_from_x + (int)((ball_to_x - ball_from_x) * f);
        int by = cells[ball_row][0].y1 - 30 - (int)(160.0f * 4.0f * f * (1.0f - f));
        const int bw = world_px(28);
        win.draw_png(BASKETBALL_DIRECTORY, bx - bw / 2, by - bw / 2, bw, bw);
    }
    // anim advances per logic tick, not per render (auto-FPS); the wrap (and
    // the one-shot completion that resets shoot_anim) runs once per tick.
    for (int _lt = logic_ticked; _lt > 0 && !check_status(game_state, IS_PAUSED); _lt--)
    {
        anim_t += 0.2f;
        float len = reanimz::skinSegmentLength(reanimz::SKIN_CATAPULT, seg);
        if (len > 0 && anim_t >= len * (loop ? 1.0f : 0.98f))
        {
            // one-shot done: back to idle
            if (!loop)
            {
                shoot_anim = 0;
                anim_t = 0.0f;
            }
            else
            {
                anim_t = 0.0f;
            }
        }
    }
}
