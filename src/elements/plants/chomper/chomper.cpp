#include "chomper.hpp"
#include "draw/world_scale.hpp"
#include "elements/Map/Map.hpp"
#include "elements/elements.hpp"
#include "support.hpp"
#include <algorithm>
#include "elements/Level/Level.hpp" // roof_tile_dy
using namespace std;

// Sheet-frame pacing for the reanim segments (reanim fps 12 == 5 ticks/frame):
//   idle 0..99, anim_bite 100..122, anim_chew 123..139, anim_swallow 140..186.
#define CHOMPER_ASSET_FRAMES 5
#define CHOMPER_BITE_NFRAMES 23
#define CHOMPER_SWALLOW_NFRAMES 47
// Jaw-closing frame inside anim_bite (~0.7 s in, like the GOTY 70-tick wind-up).
#define CHOMPER_BITE_HIT_FRAME 8
#define CHOMPER_CHEW_TICKS 2400

static PlantAssets CHOMPER_ASSET(CHOMPER_ASSET_FRAMES, 100, 96, 100, 96, 0, 0);

extern int game_state;
extern Map cells;
extern Window win;
extern Elements game_characters;

/*
Chomper constructor
*/
Chomper::Chomper(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = CHOMPER_TYPE;
    health = PLANT_HEALTH_LIMIT[CHOMPER_TYPE];
    sec_for_prepare = 1;
    directory_num = CHOMPER_SHEET_DIRECTORY;
    attack_range = NO_ATTACK; // targeting is custom (front tile only)
    state = CHOMPER_READY;
    chew_clock = 0;
}

/*
Chomper destructor
*/
Chomper::~Chomper()
{
}

/*
Nearest zombie in the bite zone. GOTY attack rect is mX+80..mX+120
(Plant.cpp:5260 SEED_CHOMPER: Rect(mX+80, mY, 40, mHeight)): with mX on the
tile centre the clone equivalent is the front tile, plus one tile behind for
whatever is already gnawing the plant. Hypnotized runners are allies --
the original's damage-range gate (EffectedByDamage sans ONLY_MINDCONTROLLED)
keeps them off the menu.
*/
Zombie *Chomper::find_target()
{
    int left_limit = cells[row][col].x1 - 40;
    int right_limit = cells[row][col].x2 + BLOCK_WIDTH / 2; // mX+80..mX+120 == x2..x2+40
    Zombie *best = nullptr;
    for (const auto &zombie : game_characters.zombies)
    {
        if (zombie->row != row || zombie->is_charmed)
            continue;
        int zx = zombie->x_location + ZOMBIE_EXACT_LOCATION + 15;
        if (!is_in(left_limit, zx, right_limit))
            continue;
        if (best == nullptr || zombie->x_location < best->x_location)
            best = zombie;
    }
    return best;
}

/*
Chomper's action: bite a zombie whole when one steps in front of the mouth,
chew it for ~40 s, swallow, then hunt again.
*/
void Chomper::action()
{
    switch (state)
    {
    case CHOMPER_READY:
        if (find_target() != nullptr)
        {
            state = CHOMPER_BITING;
            directory_num = CHOMPER_ATTACK_DIRECTORY;
            frame = 0;
        }
        break;
    case CHOMPER_BITING:
        if (frame == CHOMPER_BITE_HIT_FRAME * CHOMPER_ASSET_FRAMES)
        {
            Zombie *zombie = find_target();
            if (zombie && (zombie->type == GARGANTUAR_TYPE || zombie->type == ZOMBOSS_TYPE))
            {
                /*repvz Plant.cpp:1788-1813 (doBite): gargantuar / redeye /
                zomboss are never swallowed -- aZombie->TakeDamage(40, 0U)
                (40 raw = 2 pooled points on the clone's 20:1 scale), then
                STATE_CHOMPER_BITING_MISSED: the bite anim plays out and the
                chomper recovers straight to idle (next_frame's lunge
                recovery), no 40-s chew.
                */
                play_sound_effect(BIGCHOMP_MUSIC_DIRECTORY);
                play_sound_effect(CHOMP_MUSIC_DIRECTORY);
                if (zombie->take_hit(2, game_characters.zombie_parts))
                {
                    zombie->add_zombie_die(game_characters.zombie_parts);
                    auto &zs = game_characters.zombies;
                    zs.erase(find(zs.begin(), zs.end(), zombie));
                    delete zombie;
                }
                break;
            }
            if (zombie)
            {
                play_sound_effect(BIGCHOMP_MUSIC_DIRECTORY);
                // swallowed whole: no ash, no loot drop
                auto &zombies = game_characters.zombies;
                zombies.erase(find(zombies.begin(), zombies.end(), zombie));
                delete zombie;
                play_sound_effect(GULP_MUSIC_DIRECTORY); // gulp after swallowing (like the original)
                play_sound_effect(CHOMPSOFT_MUSIC_DIRECTORY);
            }
            else
                play_sound_effect(CHOMP_MUSIC_DIRECTORY); // bit thin air
            state = CHOMPER_CHEWING;
            directory_num = CHOMPER_CHEW_DIRECTORY;
            frame = 0;
            chew_clock = CHOMPER_CHEW_TICKS;
        }
        break;
    case CHOMPER_CHEWING:
        // chew_clock runs down in next_frame (paused-safe, logic-tick-gated).
        break;
    case CHOMPER_SWALLOWING:
        break;
    }
}

void Chomper::next_frame(const PlantAssets &assets)
{
    // anim advances per logic tick, not per render (auto-FPS); each iteration
    // is one logic tick of the bite/chew/swallow state chain
    for (int _lt = logic_ticked; _lt > 0 && check_status(game_state, IS_PAUSED) == false; _lt--)
    {
        ++frame;
        if (state == CHOMPER_BITING && frame >= assets.frame * CHOMPER_BITE_NFRAMES)
        {
            // bite lunge without prey (target walked off): recover to idle
            state = CHOMPER_READY;
            directory_num = CHOMPER_SHEET_DIRECTORY;
            frame = 0;
        }
        else if (state == CHOMPER_CHEWING && --chew_clock <= 0)
        {
            state = CHOMPER_SWALLOWING;
            directory_num = CHOMPER_SWALLOW_DIRECTORY;
            frame = 0;
        }
        else if (state == CHOMPER_SWALLOWING && frame >= assets.frame * CHOMPER_SWALLOW_NFRAMES)
        {
            state = CHOMPER_READY;
            directory_num = CHOMPER_SHEET_DIRECTORY;
            frame = 0;
        }
        else if (state == CHOMPER_READY && frame >= assets.frame * all_img[CHOMPER_SHEET_DIRECTORY].n_sheet)
        {
            frame = 0;
        }
    }
}

/*
Display chompers in row. Reanim-only art (2Pak Chomper.reanim); falls back to
the flat packet icon if the reanim parts are missing.
*/
void Chomper::display(const int &_row)
{
    if (row != _row)
        return;
    if (display_reanim(CHOMPER_ASSET))
    {
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        next_frame(CHOMPER_ASSET);
        return;
    }
    win.draw_png_width_scaled(CHOMPER_DIRECTORY, cells[row][col].x1 + 2,
                              (cells[row][col].y1 + roof_tile_dy(col)) + 15, world_px(ELEMENT_WIDTH - 8));
    next_frame(CHOMPER_ASSET);
}
