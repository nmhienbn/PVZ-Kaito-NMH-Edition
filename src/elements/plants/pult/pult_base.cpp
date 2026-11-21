#include "pult_base.hpp"
#include "draw/world_scale.hpp"
#include "draw/rsdl.hpp"
#include "elements/Map/Map.hpp"
#include "elements/collision/AttackPlantVsZombie.hpp"
#include "elements/elements.hpp"
#include "music/music.hpp"
#include "elements/Level/Level.hpp" // roof_tile_dy

namespace reanimplant
{
// Kernel-pult loaded-ammo swap, posted before each pult draw (same per-draw
// convention as reanimplant::setPumpkinHealth): true only while a butter lob
// is armed in the basket, so the skin can swap the kernal/butter tracks the
// way the original's render groups do.
void setKernelPultButterArmed(bool armed);
} // namespace reanimplant

extern int game_state;
extern Map cells;
extern Window win;
extern Elements game_characters;

PultBase::PultBase(const int &_row, const int &_col) : Plants(_row, _col)
{
    sec_for_prepare = 1;
    attack_range = ONE_ROW_AHEAD;
}

PultBase::~PultBase()
{
}

/*
Lob the basket content when the whip lands: the release tick differs per pult
(their anim_shooting windows hide the ammo sprite at different frames). The
lob flies the ammo rolled at the loop head, and the basket is then empty
until the shooting segment's reload pose draws it back.
*/
void PultBase::fire_lobbed(vector<Bullet *> &bullets)
{
    if (!attacking() || frame != fire_tick())
        return;
    play_sound_effect(type == KERNELPULT_TYPE ? KERNELPULT_SND_MUSIC_DIRECTORY
                                              : THROW_SND_MUSIC_DIRECTORY);
    bullets.push_back(new Lobbed(armed_ammo >= 0 ? armed_ammo : lobbed_kind(),
                                 row, cells[row][col].x2 - 10));
    armed_ammo = -1; // basket empty until the anim's reload pose
}

/*
Pults shoot while any target is ahead on their lane; the reel only flips at
a loop boundary (frame == 0) so the whip animation is never cut mid-swing.
Loop head = new windup: roll the ammo for the lob (kernel-pult may load
butter, repvz Plant.cpp:784-792 assigns the butter render group at exactly
this point); leaving the attack state unloads the basket.
*/
void PultBase::determine_appearance(bool zombie_ahead)
{
    if (frame != 0)
        return;
    if (zombie_ahead && directory_num == idle_dir())
        directory_num = attack_dir();
    else if (!zombie_ahead && attacking())
        directory_num = idle_dir();
    armed_ammo = attacking() ? pick_ammo() : -1;
}

void PultBase::action()
{
    fire_lobbed(game_characters.bullets);
    determine_appearance(check_in_attack_range(*this, game_characters.zombies));
}

/*
Attack loop = one lob per 180 ticks: the original mLaunchRate for pults is
300 cs (Plant.cpp gPlantDefs) = 180 ticks at 60 fps; wrap the attack reel
there instead of riding the full sheet.
*/
void PultBase::next_frame(const PlantAssets &assets)
{
    next_frame_capped(assets, attack_dir(), 180);
}

void PultBase::display(const int &_row)
{
    if (row != _row)
        return;
    const PlantAssets &assets = attacking() ? attack_assets() : idle_assets();
    reanimplant::setKernelPultButterArmed(attacking() && armed_ammo == LOBBED_BUTTER);
    if (display_reanim(assets))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        next_frame(assets);
        return;
    }
    // Legacy fallback: the flat seed-packet icon (no sprite sheets ship for
    // the roof set; art is reanim-only).
    win.draw_png_width_scaled(idle_dir(), cells[row][col].x1 + 5,
                              (cells[row][col].y1 + roof_tile_dy(col)) + 20, world_px(ELEMENT_WIDTH - 20));
    next_frame(assets);
}
