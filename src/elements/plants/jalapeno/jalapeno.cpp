#include "jalapeno.hpp"
#include "draw/world_scale.hpp"
#include "jalapeno_fire.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/boss/boss_fight.hpp"
#include "elements/elements.hpp"

// Reanim-only art (2Pak Jalapeno.reanim: idle 0..5, explode 6..13 held on the
// last pose while the fuse runs). 5 tpf.
static PlantAssets JALAPENO_ASSET(5, 85, 81, 85, 81, 5, 5);
#define JALAPENO_FUSE_TICKS 60 // GOTY mDoSpecialCountdown = 100 cs = 1.0 s

extern int game_state;
extern Map cells;
extern Window win;
extern Elements game_characters;
extern Level level;

/*
Jalapeno constructor: planted already furious (GOTY starts in anim_explode
with the fuse running).
*/
Jalapeno::Jalapeno(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = JALAPENO_TYPE;
    health = PLANT_HEALTH_LIMIT[JALAPENO_TYPE];
    sec_for_prepare = 1;
    directory_num = JALAPENO_EXPLODE_DIRECTORY;
    attack_range = NO_ATTACK; // the whole row burns manually
    fuse = JALAPENO_FUSE_TICKS;
}

Jalapeno::~Jalapeno()
{
}

/*
Burn the whole row: every zombie in the lane takes the full burn damage and
dies charred (GOTY BurnRow = 1800 raw damage, i.e. 90 peas here -- kills
everything up to gargantuar tier). Zombies that cannot be hit right now
(boulder balls; boss head raised = invulnerable) are untouched; a vulnerable
boss still soaks 90, so roughly two jalapenos per vulnerability window.
The flames also melt the zomboni's ice trail in the row: the live ice is
what a bobsled team needs to ride, so a burned lane goes back to plain lawn
(the trail would otherwise sit there until its melt timer expires).
*/
void Jalapeno::burn_row()
{
    play_sound_effect(JALAPENO_MUSIC_DIRECTORY);
    play_sound_effect(IGNITE_SND_MUSIC_DIRECTORY); // FOLEY_JALAPENO_IGNITE
    // The row-wide flame streak (repvz DoSpecial -> Board::DoFwoosh,
    // Plant.cpp:4402-4414): twelve staggered stations of looping fire.
    jalapeno_fx::spawn_lane_fire(row);
    // Tell the Zomboss controller this row burned: it shatters any ICE
    // boulder rolling here (orig Plant::BurnRow, Plant.cpp:4313-4318).
    bossfight::on_jalapeno_burn(row);
    // Melt the row's ice trail ahead of the flames.
    if (row >= 0 && row < (int)level.ice_min_x.size())
        level.ice_min_x[row] = -1;
    if (row >= 0 && row < (int)level.ice_timer.size())
        level.ice_timer[row] = 0;
    auto &zombies = game_characters.zombies;
    for (int j = 0; j < (int)zombies.size(); j++)
    {
        Zombie *z = zombies[j];
        // repvz GetDamageRangeFlags(JALAPENO) = 127 (Plant.cpp:587-621): the
        // burn reaches a TUNNELING digger (DAMAGAGES_UNDERGROUND bit);
        // BoulderZombie/boss merely borrow is_underground() as an immunity
        // flag and keep it.
        if (z->row != row || z->is_charmed ||
            (z->is_underground() && z->type != DIGGER_TYPE))
            continue;
        if (!z->take_hit(90, game_characters.zombie_parts))
            continue; // survived the burn (armored / boss) -- leave it alive
        push_burned_corpse(game_characters.zombie_parts, z);
        delete z;
        zombies.erase(zombies.begin() + j);
        j--;
    }
}

/*
Jalapeno's action: fuse runs down, then the row goes up in flames.
*/
void Jalapeno::action()
{
    if (fuse <= 0)
        return;
    if (--fuse == 0)
    {
        burn_row();
        health = 0;
    }
}

/*
Display jalapenos in row
*/
void Jalapeno::display(const int &_row)
{
    if (row != _row)
        return;
    if (display_reanim(JALAPENO_ASSET))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        Plants::next_frame(JALAPENO_ASSET);
        return;
    }
    win.draw_png_width_scaled(JALAPENO_DIRECTORY, cells[row][col].x1 + 15,
                              (cells[row][col].y2 + roof_tile_dy(col)) - 80, world_px(ELEMENT_WIDTH - 30));
    Plants::next_frame(JALAPENO_ASSET);
}
