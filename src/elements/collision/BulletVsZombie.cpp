#include "BulletVsZombie.hpp"
#include "elements/bullets/fume/fume.hpp"
#include "elements/zombies/newspaper/newspaper_zombie.hpp"
#include "game_param.hpp"
#include "music/music.hpp"
#include "reanim/reanim_zombie.hpp"
#include <algorithm>
using namespace std;

/*repvz GetRectOverlap(aRect1, aRect2) = min(rights) - max(lefts) on the
horizontal axis; a hit needs overlap > 0 (impact) / >= 0 (splash window).*/

/*Straight-shot hit test (repvz Projectile::FindCollisionTarget,
Projectile.cpp:202-238 + GetProjectileRect 1172-1181): collision is a RECT
OVERLAP between the projectile rect and the zombie body rect (zrect_*), not a
left-edge probe. Peas carry an extended rect [x-15, x+mWidth+15]; puffs /
spikes ride their plain [x, x+mWidth]. Overlap > 0 either way around the body,
so a zombie that walked past a slow bullet still catches it.*/
static bool rect_hit(const Bullet &bullet, const Zombie &zombie)
{
    int b_left = bullet.x_location;
    int b_right = bullet.x_location + bullet.width;
    if (bullet.type == PEA || bullet.type == SNOWZPEA)
    {
        b_left -= 15;  // repvz Rect(mX - 15, mY, mWidth + 15, mHeight)
        b_right += 15;
    }
    int overlap = min(b_right, zombie.zrect_right()) - max(b_left, zombie.zrect_left());
    return overlap > 0;
}

/*
@return 'true' if pea reach the zombie
*/
bool has_bullet_reached_zombie(const Bullet &bullet, const Zombie &zombie)
{
    if (bullet.row != zombie.row)
        return false;
    // Lobbed shots fly overhead on their arc and score by splash on touchdown
    // (the shot resolves its own hit in Lobbed::land, on the landing tick).
    if (bullet.type == LOBBED)
        return false;
    // Tunneling diggers are underground: no shot reaches them.
    if (zombie.is_underground())
        return false;
    // Submerged snorkel (swimming under water): shots pass over it; it only
    // surfaces to eat.
    if (zombie.is_submerged())
        return false;
    // Cactus spike is the balloon counter: it may hit a flying balloon zombie
    // (regular ground shots are filtered out by the balloon zombie itself).
    if (bullet.type == SPIKE && zombie.type == BALLOON_TYPE)
        return bullet.x_location > zombie.x_location + 70;
    // Every other ground shot passes under a flying balloon.
    if (zombie.is_flying())
        return false;
    // Left-flying shots (Split Pea's rear head, the star's back-star) cross
    // the zombie's face: hit on bbox overlap with its front arc.
    if (bullet.type == STAR)
        return bullet.x_location + bullet.width > zombie.x_location + 40 &&
               bullet.x_location < zombie.x_location + 110;
    return rect_hit(bullet, zombie);
}

/*repvz Zombie::IsFireResistant (Zombie.cpp:8545-8552, pathified):
catapult / zomboni / SCREEN-DOOR & LADDER carriers shrug off fireball splash.
(Newspapers burn.)*/
static bool fire_resistant(const Zombie &z)
{
    return z.type == CATAPULT_TYPE || z.type == ZOMBONI_TYPE ||
           z.type == DOOR_TYPE || z.type == LADDER_TYPE;
}

/*Torchwood fire-pea splash (repvz Projectile::DoSplashDamage,
Projectile.cpp:443-489 + IsZombieHitBySplash 409-441): the fireball's rect
gets its width forced to 100 and everything it touches IN THE SAME ROW burns
for mDamage/3 apiece; the total stays capped at the fireball's own damage
(aMaxSplashDamageAmount = aOriginalDamage for PROJECTILE_FIREBALL), so deeper
crowds split a thinner share (floor: 0.05 clone hp = 1 original point).
Victims take DAMAGE_HITS_SHIELD_AND_BODY (GetDamageFlags: IsSplashDamage is
true for them) -- shield and body each take the splash.*/
static void fire_pea_splash(Bullet &bullet, Zombie &target, vector<Zombie *> &zombies,
                            vector<ZombiePart> &zombie_parts)
{
    // repvz rect: [bullet.x, bullet.x + 100] (fireball rect, width forced 100)
    const int s_left = bullet.x_location, s_right = bullet.x_location + 100;
    vector<Zombie *> victims;
    for (auto *z : zombies)
    {
        if (z == &target || z->is_charmed)
            continue;
        if (z->row != bullet.row) // fireball splash: same row only (aRowDeviation != 0 -> out)
            continue;
        if (fire_resistant(*z))
            continue; // doors/ladders/vehicles shrug fire off entirely
        // fireball flags = GROUND (1): tunneling/submerged/flying stay safe
        if (z->is_underground() || z->is_submerged() || z->is_flying())
            continue;
        if (z->get_health() <= 0) // already slain this sweep
            continue;
        int overlap = min(s_right, z->zrect_right()) - max(s_left, z->zrect_left());
        if (overlap < 0) // splash check is >= 0 (repvz line 440)
            continue;
        victims.push_back(z);
    }
    if (victims.empty())
        return;
    float splash_dmg = bullet.dmg / 3.0f;                 // aProjectileDef.mDamage / 3
    const float max_total = (float)bullet.dmg;            // fireball: cap = original damage
    if (victims.size() * splash_dmg > max_total)
    {
        splash_dmg = bullet.dmg * max_total / (victims.size() * splash_dmg * 3.0f);
        splash_dmg = max(splash_dmg, 0.05f); // aSplashDamage = max(..., 1) on the 20:1 scale
    }
    for (auto *z : victims)
    {
        if (z->take_shield_and_body_hit(splash_dmg, zombie_parts))
        {
            play_sound_effect(ZOMBIE_FALLING_MUSIC_DIRECTORY);
            z->add_zombie_die(zombie_parts);
            // dead zombies are reaped by the caller's health sweep
        }
    }
}

/*
Fume-shroom cloud: pierces, every zombie in its row takes one soak. The soak
hits a carried shield AND the body (repvz DAMAGE_HITS_SHIELD_AND_BODY,
Plant.cpp:4548): shields chip but soak nothing -- the fume counter-play
against screen doors/newspapers is restored.
*/
static void handle_fume_sweep(Bullet &cloud, vector<Zombie *> &zombies, vector<ZombiePart> &zombie_parts)
{
    Fume &fume = static_cast<Fume &>(cloud);
    for (int j = 0; j < (int)zombies.size(); j++)
    {
        Zombie *z = zombies[j];
        // Fumes are ground-level: tunneling diggers pass under the cloud,
        // and a submerged snorkel sits below it.
        if (z->is_underground() || z->is_submerged())
            continue;
        if (!fume.try_hit(*z))
            continue;
        fume.crash_effect();
        if (z->take_shield_and_body_hit(1, zombie_parts))
        {
            play_sound_effect(ZOMBIE_FALLING_MUSIC_DIRECTORY);
            z->add_zombie_die(zombie_parts);
            delete z;
            zombies.erase(zombies.begin() + j);
            j--;
        }
    }
}

/*
For all pea and all zombie to check their collision.
One pea hit only one zombies at a time.
*/
void handle_bullet_encounter_zombie(vector<Bullet *> &bullets, vector<Zombie *> &zombies, vector<ZombiePart> &zombie_parts)
{
    // sort zombies -> just find the first zombie to attack
    // hypnotized zombies are left alone (they fight on the player's side)
    vector<Zombie *> tmp;
    for (auto &z : zombies)
        if (!z->is_charmed)
            tmp.push_back(z);
    stable_sort(tmp.begin(), tmp.end(), [](const Zombie *a, const Zombie *b) -> bool
                { return a->x_location == b->x_location ? a->cold_time < b->cold_time : a->x_location < b->x_location; });

    // sort bullets -> farthest pea will hit first
    sort(bullets.begin(), bullets.end());
    for (int i = 0; i < (int)bullets.size(); i++)
    {
        // Remove disappeared pea
        if (bullets[i]->is_disappeared())
        {
            bullets.erase(bullets.begin() + i);
            i--;
            continue;
        }
        if (bullets[i]->has_exploded())
            continue;
        // Fume-shroom cloud soaks the whole row it passes through.
        if (bullets[i]->type == FUME)
        {
            handle_fume_sweep(*bullets[i], zombies, zombie_parts);
            continue;
        }
        // Apply pea hit zombie & delete died zombies
        for (int j = 0; j < (int)tmp.size(); j++)
            if (apply_bullet_hit_zombie(*bullets[i], *tmp[j], zombies, zombie_parts))
            {
                tmp.erase(tmp.begin() + j);
                break;
            }
        // Reap every zombie felled this sweep (a fire-pea splash can drop
        // several at once).
        for (int j = 0; j < (int)zombies.size(); j++)
            if (zombies[j]->get_health() <= 0)
            {
                delete zombies[j];
                zombies.erase(zombies.begin() + j);
                j--;
            }
    }
}

void snow_pea_effect(Bullet &bullet, Zombie &zombie)
{
    if (bullet.type == SNOWZPEA)
    {
        // repvz: DAMAGE_FREEZE is flagged independently of the shield
        // (GetDamageFlags Projectile.cpp:399-402 + TakeDamage chill order) --
        // a screen-door carrier still catches the chill (waves the door
        // around at half pace, same as any walker).
        if (!zombie.cold_time)
        {
            zombie.bite_time *= 2;
            zombie.next_step_time *= 2;
        }
        zombie.cold_time = MAX_COLD_TIME;
    }
}

/*
If a pea collide with a zombie: apply it to hit the zombie:
    Pea -> crashed pea.
    Snow pea will slow zombies at once.
    Zombie decrease health & blink.
    Add zombies' death.
*/
bool apply_bullet_hit_zombie(Bullet &bullet, Zombie &zombie, vector<Zombie *> &zombies,
                             vector<ZombiePart> &zombie_parts)
{
    if (has_bullet_reached_zombie(bullet, zombie))
    {
        // Cactus spike mid-air: pops the balloon instead of plain damage
        // (only spikes reach a flying balloon; the pop drops it to walking).
        if (zombie.is_flying())
        {
            bullet.crash_effect();
            bullet.make_explode();
            zombie.pop_balloon(zombie_parts);
            return true;
        }
        // Sound effects: armor/zombie shields clank instead of splatting;
        // the newspaper shield clanks only while the paper still holds.
        if (zombie.type == BUCKET_TYPE || zombie.type == DOOR_TYPE ||
            (zombie.type == NEWSPAPER_TYPE &&
             zombie.get_health() > NEWSPAPER_PAPER_BREAK_HEALTH * zombie.hp_scale()))
            play_sound_effect(SHIELD_HIT_MUSIC_DIRECTORY);
        else
            bullet.crash_effect();

        // Snowz peas effects
        snow_pea_effect(bullet, zombie);

        // Torchwood fire pea (dmg 2): melts the target's chill off.
        if (bullet.dmg > 1 && zombie.cold_time)
            zombie.cold_time = 0;

        // Bullet explode
        bullet.make_explode();

        // Shield bypass (repvz Projectile::GetDamageFlags 385-398): the
        // starfruit star fired backwards (MOTION_STAR && mVelX < 0) and Split
        // Pea's rear head (MOTION_BACKWARDS) fly at the zombie's BACK -- the
        // screen door it carries up front soaks nothing; damage lands on the
        // body straight away.
        const bool bypass_shield = bullet.dir < 0 &&
                                   (bullet.type == STAR || bullet.type == PEA);
        bool dead;
        // Torchwood fire pea: hits like the original fireball (splash victim
        // rules, Projectile::GetDamageFlags 387-389) -- shield AND body each
        // soak for non-fire-resistant targets; door/ladder carriers (and
        // vehicles) take it on the shield like any straight shot.
        if (bullet.dmg > 1 && (bullet.type == PEA) && !fire_resistant(zombie))
            dead = zombie.take_shield_and_body_hit(bullet.dmg, zombie_parts);
        else
            dead = bypass_shield ? zombie.take_body_hit(bullet.dmg, zombie_parts)
                                 : zombie.take_hit(bullet.dmg, zombie_parts);

        // Fire-pea AoE on impact (DoSplashDamage); a fire-resistant target
        // produces no splash at all (DoImpact's else branch, Projectile.cpp:822).
        if (bullet.dmg > 1 && bullet.type == PEA && !fire_resistant(zombie))
            fire_pea_splash(bullet, zombie, zombies, zombie_parts);

        if (dead)
        {
            play_sound_effect(ZOMBIE_FALLING_MUSIC_DIRECTORY);
            zombie.add_zombie_die(zombie_parts);
        }
        return true;
    }
    return false;
}
