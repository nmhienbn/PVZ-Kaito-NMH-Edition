// Game class hooks for the PopCap reanim plant renderer. Kept separate from
// reanim_plant.cpp so that file stays linkable without the whole game,
// mirroring pam_plant_hooks.cpp / reanim_game_hooks.cpp.

#include "reanim_plant.hpp"

#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp" // roof_tile_dy
#include "elements/Map/Map.hpp"
#include "elements/elements.hpp"
#include "elements/plants/plants.hpp"
#include "game_param.hpp" // MAX_TIME_BLINK

#include <cmath>

extern int game_state;
extern Map cells;
extern Elements game_characters;
extern Level level;
extern Window win;

namespace {

// Flower-pot rider offset (repvz PlantFlowerPotHeightOffset, scale 1.0):
// a plant standing in a pot sits higher on the pot rim.
float flower_pot_height_offset(int type)
{
    float height = -5.0f;
    float fix = 0.0f;
    switch (type)
    {
    case CHOMPER_TYPE:
    case PLANTERN_TYPE:
        height -= 5.0f;
        break;
    case SCAREDYSHROOM_TYPE:
        height += 5.0f;
        fix -= 8.0f;
        break;
    case SUNSHROOM_TYPE:
    case PUFFSHROOM_TYPE:
        fix -= 4.0f;
        break;
    case HYPNOSHROOM_TYPE:
    case MAGNETSHROOM_TYPE:
    case PEASHOOTER_TYPE:
    case REPEATER_TYPE:
    case SNOWPEA_TYPE:
    case THREEPEATER_TYPE:
    case SUNFLOWER_TYPE:
    case MARIGOLD_TYPE:
    case CABBAGEPULT_TYPE:
    case MELONPULT_TYPE:
    case TANGLEKELP_TYPE:
    case BLOVER_TYPE:
    case SPIKEWEED_TYPE:
        fix -= 8.0f;
        break;
    case SEASHROOM_TYPE:
    case POTATOMINE_TYPE:
        fix -= 4.0f;
        break;
    case LILYPAD_TYPE:
        fix -= 16.0f;
        break;
    case COFFEEBEAN_TYPE:
        fix -= 20.0f;
        break;
    default:
        break;
    }
    return height;
}

// Port of repvz PlantDrawHeightOffset (Plant.cpp:3537-3669) for the plants
// this clone supports; mX,mY == our cell (x1,y1).
float plant_draw_height_offset(int type, int row, int col)
{
    float height = 0.0f;
    const Block &cell = cells[row][col];
    const bool pool = cell.is_water || level.row_has_water(row);

    // Pool floating bob (2 px sine). In-game the board is always present, so
    // this is exactly the original's IsPoolSquare branch.
    if (pool)
    {
        const float counter = (float)(SDL_GetTicks() / 10 % 200);
        const float pos = (float)row * 3.14159265f + (float)col * 0.25f * 3.14159265f;
        const float t = counter * (2.0f * 3.14159265f / 200.0f);
        height += sinf(pos + t) * 2.0f;
    }

    bool has_pot = false;
    if (type != FLOWERPOT_TYPE)
        for (Plants *p : game_characters.plants)
            if (p != nullptr && p->get_row() == row && p->get_col() == col &&
                p->get_type() == FLOWERPOT_TYPE)
            {
                has_pot = true;
                break;
            }
    if (has_pot)
        height += flower_pot_height_offset(type);

    if (type == FLOWERPOT_TYPE)
        height += 26.0f;
    else if (type == LILYPAD_TYPE)
        height += 25.0f;
    else if (type == STARFRUIT_TYPE)
        height += 10.0f;
    else if (type == TANGLEKELP_TYPE)
        height += 24.0f;
    else if (type == SEASHROOM_TYPE)
        height += 28.0f;
    else if (type == COFFEEBEAN_TYPE)
        height -= 20.0f;
    else if (type == CACTUS_TYPE)
        return height;
    else if (type == PUMPKIN_TYPE)
        height += 15.0f;
    else if (type == PUFFSHROOM_TYPE)
        height += 5.0f;
    else if (type == SCAREDYSHROOM_TYPE)
        height -= 14.0f;
    else if (type == GRAVEBUSTER_TYPE)
        height -= 40.0f;
    else if (type == SPIKEWEED_TYPE)
    {
        const int bottom_row = (int)cells.size() - 1;
        if (has_pot)
            height += 5.0f;
        else if (level.is_roof())
            height += 15.0f;
        else if (pool)
            height += 0.0f;
        else if (row == bottom_row && col >= 7 && (int)cells.size() >= 6)
            height += 1.0f;
        else if (row == bottom_row && col < 7)
            height += 12.0f;
        else
            height += 15.0f;
    }
    return height;
}

} // namespace

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
    // Retail placement: the body reanim doc is authored for scale 1.0 with
    // its origin at the cell top-left, plus PlantDrawHeightOffset (roof: the
    // per-column slope step). The F2 layout editor's transient offsets shift
    // this anchor (0 in normal gameplay).
    reanimplant::PlantPlacement placement;
    placement.x = (float)(cells[row][col].x1 + layout_dx);
    placement.y = (float)(cells[row][col].y1 + layout_dy +
                          roof_tile_dy(col) +
                          (int)lroundf(plant_draw_height_offset(type, row, col)));
    placement.scale = rsdl_debug_scale(this) * WORLD_SCALE;
    int nSheet = all_img[directory_num].n_sheet;
    // Flash fades out over the blink budget, same ramp as the zombie hook.
    const float fade = attacked_time > 0 ? (float)attacked_time / MAX_TIME_BLINK : 1.0f;
    if (!reanimplant::drawPlant(type, directory_num,
                                (float)frame / (float)assets.frame, nSheet,
                                reanim_clock, cells[row][col].x1 + layout_dx,
                                cells[row][col].x2 + layout_dx,
                                cells[row][col].y1, assets.game_height,
                                attacked_time > 0,
                                nullptr, nullptr, nullptr, nullptr, fade,
                                rsdl_debug_scale(this) * WORLD_SCALE,
                                &placement))
        return false;
    // anim advances per logic tick, not per render (auto-FPS)
    if (check_status(game_state, IS_PAUSED) == false)
        reanim_clock += reanimplant::frameRate(type) / (float)FPS * logic_ticked;
    return true;
}
