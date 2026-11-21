// Game class hooks for the PvZ2 PAM plant renderer. Kept separate from
// pam_plants.cpp so that file stays linkable without the whole game (used by
// demo/pam_demo), mirroring reanim_game_hooks.cpp.

#include "pam_plants.hpp"

#include "draw/rsdl.hpp"
#include "elements/Level/Level.hpp" // roof_tile_dy
#include "elements/Map/Map.hpp"
#include "elements/plants/plants.hpp"
#include "game_param.hpp" // MAX_TIME_BLINK

extern int game_state;
extern Map cells;
extern Window win;

bool Plants::display_pam(const PlantAssets &assets)
{
    if (USE_PAM_PLANT_SKIN == 0)
        return false; // PAM skins disabled -> caller uses the PvZ1 sprite sheets
    if (!pamplant::ready())
    {
        pamplant::init(win.sprite_renderer());
        if (!pamplant::ready())
            return false; // assets missing -> caller falls back to sprite sheets
    }
    // pam clock restarts when the sheet side enters a new state.
    if (pam_dir != directory_num)
    {
        pam_dir = directory_num;
        pam_clock = 0.0f;
    }
    // feet at the sprite sheet's bottom edge (roof: per-column slope step of
    // repvz GridToPixelY so plants hug the shingle lines)
    int footY = cells[row][col].y1 + assets.y_alias + assets.game_height +
                roof_tile_dy(col);
    int nSheet = all_img[directory_num].n_sheet;
    // Flash fades out over the blink budget, same ramp as the reanim hook.
    const float fade = attacked_time > 0 ? (float)attacked_time / MAX_TIME_BLINK : 1.0f;
    if (!pamplant::drawPlant(type, directory_num,
                             (float)frame / (float)assets.frame, nSheet,
                             pam_clock, cells[row][col].x1, cells[row][col].x2,
                             footY, assets.game_height, attacked_time > 0, fade))
        return false;
    // pam_clock advances per logic tick, not per render (auto-FPS)
    if (check_status(game_state, IS_PAUSED) == false)
        pam_clock += pamplant::frameRate(type) / (float)FPS * logic_ticked;
    return true;
}
