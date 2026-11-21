#include "blover.hpp"
#include "draw/world_scale.hpp"
#include "draw/fog_overlay.hpp"
#include "elements/Map/Map.hpp"
#include "elements/elements.hpp"
#include "elements/zombies/zombie_struct.hpp"
#include "elements/Level/Level.hpp" // roof_tile_dy

// Blover.reanim (fps 12 == 5 ticks/frame): idle 0..28, blow 53..81.
#define BLOVER_ASSET_FRAMES 5
// Wind blast fires ~a third into the blow clip.
#define BLOVER_BLOW_FRAME 10
// The plant withers when the blow clip has fully spun down.
#define BLOVER_LIFETIME (29 * BLOVER_ASSET_FRAMES)

static PlantAssets BLOVER_ASSET(BLOVER_ASSET_FRAMES, 76, 76, 76, 76, 5, 8);

extern int game_state;
extern Map cells;
extern Window win;
extern Elements game_characters;

Blover::Blover(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = BLOVER_TYPE;
    health = PLANT_HEALTH_LIMIT[BLOVER_TYPE];
    sec_for_prepare = 1;
    directory_num = BLOVER_WIND_DIRECTORY;
    attack_range = NO_ATTACK;
}

Blover::~Blover()
{
}

/*
Wind blast: blow all the fog away for ~20 s (it drifts back afterwards) and
pop every flying balloon zombie on the lawn.
*/
void Blover::blow(vector<Zombie *> &zombies, vector<ZombiePart> &zombie_parts)
{
    play_sound_effect(BLOVER_MUSIC_DIRECTORY);
    fogov::blow_away(fogov::FOG_BLOWN_TICKS);
    bool popped_any = false;
    for (int j = 0; j < (int)zombies.size(); j++)
    {
        Zombie *z = zombies[j];
        if (z->type != BALLOON_TYPE)
            continue;
        if (!popped_any)
            play_sound_effect(BALLOON_POP_MUSIC_DIRECTORY);
        popped_any = true;
        z->add_zombie_die(zombie_parts);
        delete z;
        zombies.erase(zombies.begin() + j);
        j--;
    }
}

void Blover::action()
{
    if (!blown && frame >= BLOVER_BLOW_FRAME * BLOVER_ASSET_FRAMES)
    {
        blown = true;
        blow(game_characters.zombies, game_characters.zombie_parts);
    }
    if (frame >= BLOVER_LIFETIME - 1)
        health = 0; // spent: update_plants_status reaps it
}

void Blover::display(const int &_row)
{
    if (row != _row)
        return;
    if (display_reanim(BLOVER_ASSET))
    {
        // anim advances per logic tick, not per render (auto-FPS)
        for (int _lt = logic_ticked; _lt > 0 && attacked_time > 0 &&
             check_status(game_state, IS_PAUSED) == false; _lt--)
            attacked_time--;
        next_frame(BLOVER_ASSET);
        return;
    }
    win.draw_png_width_scaled(BLOVER_DIRECTORY, cells[row][col].x1 + 10,
                              (cells[row][col].y1 + roof_tile_dy(col)) + 20, world_px(ELEMENT_WIDTH - 30));
    next_frame(BLOVER_ASSET);
}
