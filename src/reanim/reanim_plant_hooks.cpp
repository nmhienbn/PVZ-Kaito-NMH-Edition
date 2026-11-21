// Game class hooks for the PopCap reanim plant renderer. Kept separate from
// reanim_plant.cpp so that file stays linkable without the whole game,
// mirroring pam_plant_hooks.cpp / reanim_game_hooks.cpp.

#include "reanim_plant.hpp"

#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp" // roof_tile_dy
#include "elements/Map/Map.hpp"
#include "elements/plants/plants.hpp"
#include "game_param.hpp" // MAX_TIME_BLINK

extern int game_state;
extern Map cells;
extern Window win;

bool Plants::display_reanim(const PlantAssets &assets)
{
    if (USE_REANIM_PLANT_SKIN == 0)
        return false; // reanim skins disabled -> caller falls through
    if (!reanimplant::ready())
    {
        reanimplant::init(win.sprite_renderer());
        if (!reanimplant::ready())
            return false; // assets missing -> caller falls back
    }
    // reanim clock restarts when the sheet side enters a new state.
    if (reanim_dir != directory_num)
    {
        reanim_dir = directory_num;
        reanim_clock = 0.0f;
    }
    // feet at the sprite sheet's bottom edge (roof: per-column slope step of
    // repvz GridToPixelY so plants hug the shingle lines); the F2 layout
    // editor's transient offsets shift the anchor (0 in normal gameplay).
    int footY = cells[row][col].y1 + assets.y_alias + assets.game_height +
                roof_tile_dy(col) + layout_dy;
    int nSheet = all_img[directory_num].n_sheet;
    // Flash fades out over the blink budget, same ramp as the zombie hook.
    const float fade = attacked_time > 0 ? (float)attacked_time / MAX_TIME_BLINK : 1.0f;
    if (!reanimplant::drawPlant(type, directory_num,
                                (float)frame / (float)assets.frame, nSheet,
                                reanim_clock, cells[row][col].x1 + layout_dx,
                                cells[row][col].x2 + layout_dx,
                                footY, assets.game_height, attacked_time > 0,
                                nullptr, nullptr, nullptr, nullptr, fade,
                                rsdl_debug_scale(this) * WORLD_SCALE))
        return false;
    // anim advances per logic tick, not per render (auto-FPS)
    if (check_status(game_state, IS_PAUSED) == false)
        reanim_clock += reanimplant::frameRate(type) / (float)FPS * logic_ticked;
    return true;
}
