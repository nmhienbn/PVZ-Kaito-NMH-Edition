#include "lobbed.hpp"
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Map/Map.hpp"
#include "elements/elements.hpp"
#include "music/music.hpp"
#include "reanim/one_shot_fx.hpp"
#include <algorithm>

extern Map cells;
extern Window win;
extern Elements game_characters;

// Arc physics (repvz Projectile::UpdateLobbedMotion, Projectile.cpp:505-515;
// launch params repvz Plant::Fire, Plant.cpp:4786-4791): MOTION_LOBBED runs a
// fixed 120-centisecond flight at any range (mVelX = aRangeX/120) with mPosZ
// integrated under mVelZ = aRangeY/120 - 7 px/cs and gravity mAccZ = 0.115
// px/cs^2; screen Y = mPosY + mPosZ, mPosY stepping linearly. 120 cs = 72
// ticks at 60 tps; converting to px/tick (1 tick = 5/3 cs) gives vz0 below
// (apex vz0^2/(2g) ~= 213 px at tick ~36.5, arc residual -12 px at
// touchdown). The aRangeY/120 linear climb is carried by the baseline lerp
// in move(), so z() below is pure gravity.
static const int LOBBED_FLIGHT_TICKS = 72;
static const float LOBBED_VZ0 = -7.0f * 5.0f / 3.0f;        // px/tick
static const float LOBBED_GRAVITY = 0.115f * 25.0f / 9.0f;  // px/tick^2
// Butter stun: repvz Zombie::ApplyButter sets mButteredCounter = 400 cs
// (Zombie.cpp:8614) = 240 ticks at 60 tps; then the zombie walks on,
// unchilled.
static const int BUTTER_STUN_TICKS = 240;
// Hit window around the landing point for the direct hit.
static const int LOBBED_HIT_WINDOW = 50;

/*Pick the nearest zombie ahead of the launch point on this lane (the ground
rules: tunneling diggers and submerged snorkels can't be targeted; flying
balloons and hanging bungees can - lobbed shots reach the sky).*/
static int nearest_ahead_x(const int &row, const int &from_x)
{
    int best = -1;
    for (auto *z : game_characters.zombies)
    {
        if (z->row != row || z->is_charmed || z->is_underground() || z->is_submerged())
            continue;
        int zx = z->x_location + 60; // roughly the torso
        if (zx <= from_x)
            continue;
        if (best < 0 || zx < best)
            best = zx;
    }
    return best;
}

Lobbed::Lobbed(const int &_kind, const int &_row, const int &_x)
    : Bullet(LOBBED, _row, _x)
{
    kind = _kind;
    from_x = x_location;
    from_y = cells[row][0].y1 - 10; // leaves the basket just above the plant
    y_location = from_y;
    int tx = nearest_ahead_x(row, from_x);
    // Bare lane: the original lobs at board x 700 (repvz Plant::Fire,
    // Plant.cpp:4780: aRangeX = 700 - aOriginX, clamped to 40 px min at
    // Plant.cpp:4783-4785) -> clone x 920, never closer than 40 px out.
    target_x = tx >= 0 ? tx : std::max(920, from_x + 40);
    t_total = LOBBED_FLIGHT_TICKS;
    switch (kind)
    {
    case LOBBED_BUTTER:
        directory_num = BUTTER_BULLET_DIRECTORY;
        // repvz Projectile::Draw scales the butter image x0.8 (Projectile.cpp:
        // 1022-1023); the retail png is fully opaque, so the retail draw is
        // 49x50 x0.8 = 39x40 of visible pat. The 2Pak png keeps the same
        // 49x50 canvas but its opaque pat is only 41x42 (4-px rim), which
        // 38x40 squashed to 32x34 visible; 47x48 lands the pat at 39x40.
        width = 47;
        height = 48;
        dmg = 2; // 40 original damage
        break;
    case LOBBED_MELON:
        directory_num = MELON_BULLET_DIRECTORY;
        width = 54;
        height = 46; // Melonpult_melon.png opaque 54x46 (scale_audit §8.3)
        dmg = 4; // 80 original damage
        break;
    case LOBBED_CORN:
        directory_num = CORN_BULLET_DIRECTORY;
        width = 16;
        height = 16; // Cornpult_kernal.png opaque 17x17 x0.95 (scale_audit §8.3)
        dmg = 1; // 20 original damage
        break;
    default: // LOBBED_CABBAGE
        directory_num = CABBAGE_BULLET_DIRECTORY;
        width = 30;
        height = 27; // Cabbagepult_cabbage.png opaque 30x27 (scale_audit §8.3)
        dmg = 2; // 40 original damage (repvz gProjectileDefinition, Projectile.cpp:17)
        break;
    }
}

/*
Touchdown: the direct hit lands on the zombie closest to the aim point on
this lane (within the hit window); the melon then splashes its 3x3
neighbourhood. Butter pastes its target to the ground instead of slowing it.
*/
void Lobbed::land()
{
    Zombie *direct = nullptr;
    int best_d = LOBBED_HIT_WINDOW;
    for (auto *z : game_characters.zombies)
    {
        if (z->row != row || z->is_charmed || z->is_underground() || z->is_submerged())
            continue;
        int d = abs(z->x_location + 60 - target_x);
        if (d < best_d)
        {
            best_d = d;
            direct = z;
        }
    }
    if (kind == LOBBED_BUTTER && direct)
    {
        play_sound_effect(BUTTER_MUSIC_DIRECTORY);
        // repvz Zombie::ApplyButter (Zombie.cpp:8606-8615) pastes only
        // coherent ground zombies: no zamboni, no boss, no bobsled team
        // still on its sled, nothing airborne. The pat sound still plays and
        // the 2 damage below lands regardless (the original damages through
        // ApplyButter's early-out too).
        const bool immune = direct->type == ZOMBONI_TYPE ||
                            direct->type == ZOMBOSS_TYPE ||
                            (direct->type == BOBSLED_TYPE && direct->is_riding_vehicle()) ||
                            direct->is_flying();
        if (!immune)
        {
            direct->butter_time = BUTTER_STUN_TICKS;
            direct->cold_time = 0; // butter replaces any chill on the victim
            // impact burst on the crown (the persistent pat draw stays in
            // Zombie::display)
            reanimfx::spawn_butter_splat(direct);
        }
    }
    vector<Zombie *> hits;
    int n_splash = 0;
    for (auto *z : game_characters.zombies)
    {
        if (z->is_charmed || z->is_underground() || z->is_submerged())
            continue;
        if (z == direct)
        {
            hits.push_back(z);
            continue;
        }
        if (kind != LOBBED_MELON)
            continue;
        // Melon splash (repvz DoSplashDamage/IsZombieHitBySplash,
        // Projectile.cpp:443-489): rect-overlap between the melon's 60-px
        // splash rect and the zombie body rect, on this row or the
        // neighbouring ones.
        if (abs(z->row - row) > 1)
            continue;
        // repvz splash rect: GetProjectileRect for MELON = Rect(mX+20, mY, 60,
        // h). At the touchdown tick the melon just tips the victim's body
        // (mX+80 ~= rect left), so the window sits behind the aim point; in
        // clone terms (target_x = the victim's torso probe) that band is
        // [target_x - 35, target_x + 25] -- 60 px wide, hitting bodies from
        // ~110 px behind to ~10 px ahead of the aim.
        int overlap = min(target_x + 25, z->zrect_right()) - max(target_x - 35, z->zrect_left());
        if (overlap < 0) // splash check is >= 0 (repvz line 440)
            continue;
        hits.push_back(z);
        n_splash++;
    }
    if (kind == LOBBED_MELON)
        play_sound_effect(MELON_IMPACT_SND_MUSIC_DIRECTORY);
    else if (kind != LOBBED_BUTTER)
        play_sound_effect(PEA_CRASH_MUSIC_DIRECTORY);
    if (kind == LOBBED_MELON) reanimfx::spawn(reanimfx::FX_MELON_IMPACT, x_location + width / 2, y_location + height / 2, row); // ground burst strip at the touchdown point
    // Splash damage: aProjectileDef.mDamage / 3 per victim (about 26-27 orig
    // for a melon), capped at 7 victims' worth of damage -- deeper crowds
    // split a thinner per-victim share (floor 0.05 == 1 original point).
    float splash_dmg = dmg / 3.0f;
    if (kind == LOBBED_MELON && n_splash * splash_dmg > splash_dmg * 7)
    {
        splash_dmg = dmg * (dmg / 3.0f * 7) / (n_splash * (dmg / 3.0f) * 3.0f);
        splash_dmg = max(splash_dmg, 0.05f);
    }
    for (auto *z : hits)
    {
        float dealt = (z == direct) ? (float)dmg : splash_dmg;
        // The original flags lobbed impacts DAMAGE_BYPASSES_SHIELD
        // (GetDamageFlags, MOTION_LOBBED): the shot arcs over a carried
        // screen door / newspaper / ladder and lands on the body (helms
        // still soak). Melons carry DAMAGE_HITS_SHIELD_AND_BODY instead:
        // shield and body each take the full hit.
        bool dead;
        if (kind == LOBBED_MELON && z->carries_shield())
        {
            dead = z->take_hit(dealt, game_characters.zombie_parts); // shield's share
            if (!dead)
                dead = z->take_body_hit(dealt, game_characters.zombie_parts); // body's share
        }
        else if (kind == LOBBED_MELON)
            dead = z->take_hit(dealt, game_characters.zombie_parts);
        else
            dead = z->take_body_hit(dealt, game_characters.zombie_parts);
        if (dead)
        {
            play_sound_effect(ZOMBIE_FALLING_MUSIC_DIRECTORY);
            z->add_zombie_die(game_characters.zombie_parts);
            // erase after the loop: the dead zombie's health is already zeroed
        }
    }
    for (int j = 0; j < (int)game_characters.zombies.size(); j++)
        if (game_characters.zombies[j]->get_health() <= 0)
        {
            delete game_characters.zombies[j];
            game_characters.zombies.erase(game_characters.zombies.begin() + j);
            j--;
        }
}

void Lobbed::move()
{
    if (has_exploded())
        return;
    t++;
    float f = (float)t / (float)t_total;
    if (f >= 1.0f)
    {
        x_location = target_x;
        explode = 1; // spent at touchdown; is_disappeared reaps it next sweep
        land();
        return;
    }
    x_location = from_x + (int)((target_x - from_x) * f);
    float base = from_y + (cells[row][0].y1 + 25 - from_y) * f;
    // Gravity arc on top of the climbing baseline: z(t) = vz0*t + g*t^2/2.
    y_location = (int)(base + LOBBED_VZ0 * t + 0.5f * LOBBED_GRAVITY * t * t);
}

void Lobbed::display()
{
    if (has_exploded())
        return;
    // Sprite grows about the logical-rect center; width/height stay
    // collision-only (retail butter 47x48 lands probed at the same center).
    const int dw = world_px(width), dh = world_px(height);
    win.draw_png(directory_num, x_location + (width - dw) / 2,
                 y_location + (height - dh) / 2, dw, dh);
    // no display_shadow() here: the shared shadow pass
    // (render_elements.cpp) already draws one shadow per bullet.
}
