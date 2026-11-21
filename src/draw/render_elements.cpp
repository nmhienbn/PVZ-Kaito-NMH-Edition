#include "draw/render_elements.hpp"
#include "draw/fog_overlay.hpp"
#include "draw/plant_icon_dir.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/Player/Player.hpp"
#include "elements/boss/boss_fight.hpp"
#include "elements/bowling/rolling_nut.hpp"
#include "elements/button/button.hpp"
#include "elements/conveyor/conveyor.hpp"
#include "elements/elements.hpp"
#include "elements/elements_actions.hpp"
#include "elements/mower/mower.hpp"
#include "elements/plants/jalapeno/jalapeno_fire.hpp"
#include "elements/progress_bar/progress_bar.hpp"
#include "elements/storm/storm.hpp"
#include "elements/vasebreaker/vasebreaker.hpp"
#include "elements/zombies/zombie.hpp"
#include "reanim/one_shot_fx.hpp"
#include <deque>
#include <cmath>
using namespace std;
const int SUN_BAR_WIDTH = 180;   // drawn width; sun_bar.png is 167x58 -> 62 tall
const int SUN_BAR_X = 5, SUN_BAR_Y = 5;

extern Level level;
extern Elements game_characters;

extern Player player;
extern Map cells;
extern Window win;
extern deque<int> shown_level;
extern int clk;

/*Conveyor-belt seed bank (repvz SeedBank::Draw: IMAGE_CONVEYORBELT_BACKDROP at
(83, 0), the scrolling IMAGE_CONVEYORBELT surface, then the packets of
conveyor.cpp BELT_X1..BELT_XR; 2Pak ConveyorBelt_backdrop.png (516x86) +
ConveyorBelt.png tiles). The strip ends flush with the turbo icon's left
edge, never under it or the menu icon.*/
const int BELT_ART_X1 = 79; // backdrop's left frame, original draws it at x = 83
const int BELT_ART_X2 = BELT_XR + 10; // == 955 == TURBO_ICON.x1 (button.cpp): a 10-px end rail past the packets, no underlap
const int BELT_ART_H = 88;            // backdrop drawn height
const int BELT_ART_TILE_W = 502, BELT_ART_TILE_H = 96;
/*The 9-arg Window::draw_png re-derives the dst height as sh*dw/sw (the dst dh
is advisory only, rsdl_img.cpp), so the drawn tile height follows the SAMPLED
rows: 86 of the tile's 96 keeps the surface inside the 88-tall backdrop
instead of bleeding 10 px past its bottom frame onto the lawn.*/
const int BELT_ART_TILE_CLIP_H = 86;

/*Backdrop under the whole strip + the belt surface sliding left (1 px per
tick, the belt's own packet drift speed). Drawn beneath the packet row so
packets read as riding the belt; static art + scroll, no packet changes.*/
void draw_conveyor_belt_art()
{
    win.draw_png(CONVEYORBELT_BACKDROP_DIRECTORY, BELT_ART_X1, 0,
                 BELT_ART_X2 - BELT_ART_X1, BELT_ART_H);
    int scroll = clk % BELT_ART_TILE_W;
    for (int tile = BELT_ART_X1 - scroll - BELT_ART_TILE_W; tile < BELT_ART_X2;
         tile += BELT_ART_TILE_W)
    {
        int sx = max(0, BELT_ART_X1 + 6 - tile); // clip the tile to the strip
        int dx = max(tile, BELT_ART_X1 + 6);
        int w = min(tile + BELT_ART_TILE_W, BELT_ART_X2) - dx;
        if (w > 0)
            win.draw_png(CONVEYORBELT_TILE_DIRECTORY, sx, 0, w, BELT_ART_TILE_CLIP_H,
                         dx, 0, w, BELT_ART_TILE_CLIP_H);
    }
}

/*Display game layout:
    + Background (playground).
    + Sun and Player's sun count.
    + Shovel. (and key to choose)
    + Plant seeds (Chosen or not).
    + Menu icon.
    + Game progress bar
*/
void display_game_layout()
{
    // Background
    win.draw_bg(BLACK_SCREEN_DIRECTORY);
    win.draw_bg(level.background_directory);

    // Sun bar and player's sun count (no sun economy on belt levels)
    if (!belt.active)
    {
        win.draw_png_height_scaled(SUN_BAR_DIRECTORY, SUN_BAR_X, SUN_BAR_Y, SUN_BAR_WIDTH);
        win.set_style(PVZUI_TTF, 26, TTF_STYLE_BOLD);
        win.show_text(to_string(player.sun_count), 100, 17,
                      (player.sun_count_change_color_times & 1 ? RED : BLACK),
                      PVZUI_TTF, 30);
    }

    // Shovel (available once unlocked)
    if (level.has_shovel)
    {
        Shovel_bar.display(SHOVEL_BAR_DIRECTORY);
        Shovel_bar.blink();
        win.show_text_outlined("S", Shovel_bar.x2 - 13, Shovel_bar.y1, WHITE, BRIANNE_TTF);
    }

    // Plant seed
    display_seed_packets_bar();
    if (player.is_choosing_a_plant() || player.is_shoveling)
    {
        blink_row_and_col();
    }
    display_game_elements();

    // Fog bank (area 4 night-pool): drawn after the zombies so they sink into
    // it; Plantern bubbles and a fresh Blover punch it open (fog_overlay.cpp).
    fogov::display();

    // Suns sit above the fog: they are UI pickups, not lawn entities, so the
    // fog bank must never cover them (same split as the original DrawFog).
    display_suns(game_characters.suns);

    // Menu icon (to show pause menu)
    PAUSE_ICON.display(PAUSE_ICON_DIRECTORY);
    TURBO_ICON.display(TURBO_ICON_DIRECTORY);

    // Progress bar
    display_progress_bar();

    // 4-10 storm: pitch-black sheet over the whole frame outside lightning
    // flashes (no-op on other levels). Fog overlay (area 4) is drawn by the
    // fog agent's own layer above the lawn, not here.
    storm::draw_overlay();

    // 5-10 Zomboss: RV landing-zone warning patch, topmost like the old
    // tick-time draw (handle_changes ran after the layout in the pre-
    // auto-FPS loop; the redraw-after-ticks order would wipe it otherwise).
    // No-op while no RV is in the air.
    bossfight::draw_rv_warning();
}

/*Display the plants' seed:
    + Player hasn't unlocked yet: no display.
    + Display plant seed bar base on the number of plant.
    + Dim if player doesn't have enough sun or it is chosen.
    + Render a black screen to display remaining time for it to refresh.
    + Normal if player have enough sun.
    + Key to choose
*/
void display_seed_packets_bar()
{
    // Conveyor/bowling levels: the belt replaces the packet column.
    if (belt.active)
    {
        draw_conveyor_belt_art();
        belt.display();
        return;
    }
    // Count number of unlocked plant.
    int num_plants = player.seed_packets.size();

    // Plant seed packets
    for (int i = 0; i < num_plants; i++)
    {
        player.seed_packets[i].display(plant_seed[i].x1, plant_seed[i].y1, player.sun_count);
        win.show_text_outlined(to_string(i + 1), plant_seed[i].x1 + ICON_WIDTH - 13, plant_seed[i].y1, WHITE, BRIANNE_TTF);
        player.seed_packets[i].blink(plant_seed[i].x1, plant_seed[i].y1);
    }
}

/*Display shadow under: plants, zombies, bullets*/
void display_shadow()
{
    for (int row = 0; row < (int)cells.size(); row++)
    {
        for (int col = 0; col < HORIZ_BLOCK_COUNT; col++)
        {
            if (cells[row][col].is_planted)
            {
                // Roof: the shadow follows the tile's slope step like the
                // plant standing on it (repvz GridToPixelY per-column offset).
                // World knob grows the blob about its own center.
                const int sw = world_px(86), sh = world_px(36);
                win.draw_png(SHADOW_DIRECTORY, cells[row][col].x1 - 5 + (86 - sw) / 2,
                             cells[row][col].y2 - 40 + roof_tile_dy(col) + (36 - sh) / 2, sw, sh);
            }
        }
    }
    for (const auto &zombie : game_characters.zombies)
    {
        // Zombie shadows ride the same slope ramp the zombie render uses
        // (GetPosYBasedOnRow) so the blob stays under its feet. Big Trouble
        // Little Zombie (3-5) minis draw at half reanim scale -- the blob
        // shrinks with them, center anchored like the original's
        // TodDrawImageCenterScaledF(g, IMAGE_PLANTSHADOW, ..., mScaleZombie)
        // (repvz Zombie::DrawShadow, Zombie.cpp:9123-9310, mScaleZombie=0.5
        // for Little Trouble at Zombie.cpp:792).
        float k = zombie->shadow_scale();
        int sw = max(1, (int)lround(96 * k * WORLD_SCALE));
        int sh = max(1, (int)lround(40 * k * WORLD_SCALE));
        win.draw_png(SHADOW_DIRECTORY,
                     zombie->x_location + 85 + (96 - sw) / 2,
                     cells[zombie->row][0].y2 - 40 + (40 - sh) / 2 +
                         roof_slope_offset(zombie->x_location),
                     sw, sh);
    }
    for (auto &bullet : game_characters.bullets)
    {
        bullet->display_shadow();
    }
}

/*Doom-shroom crater art (port of GridItem::DrawCrater): water tiles use the
floating day/night water craters, roof tiles the tall roof craters (slope
half = "left" art past column 5), the plain yard keeps crater.png (cel 0 =
day, cel 1 = night) and flips to the "fading" art over the last half of the
heal timer. Every png ships 2 cels side by side; draw the picked cel.
Placements bottom-anchor the art to the tile like the untimed old render.*/
static void display_crater(int row, int col)
{
    const Block &b = cells[row][col];
    bool fading = b.crater_clk > 0 && b.crater_clk < CRATER_FADING_TICKS;
    int img = CRATER_DIRECTORY, cel_w = 90, cel_h = 61;
    int cel = level.is_night() ? 1 : 0;
    int dx = b.x1 + 5, dy = b.y1 + 55, dw = 80, dh = 50;
    if (level.map_type == 6 || level.map_type == 7)
    {
        img = col < 5 ? CRATER_ROOF_LEFT_DIRECTORY : CRATER_ROOF_CENTER_DIRECTORY;
        cel_w = 75;
        cel_h = col < 5 ? 87 : 77;
        cel = fading ? 1 : 0;
        dx = b.x1 + (col < 5 ? 8 : 10);
        dw = cel_w;
        dh = cel_h;
        dy = b.y1 + 105 - dh;
    }
    else if (b.is_water)
    {
        img = level.is_night() ? CRATER_WATER_NIGHT_DIRECTORY : CRATER_WATER_DAY_DIRECTORY;
        cel_w = 85;
        cel_h = 53;
        cel = fading ? 1 : 0;
        dx = b.x1 + 3;
        dw = cel_w;
        dh = cel_h;
        dy = b.y1 + 105 - dh;
    }
    else if (fading)
    {
        img = CRATER_FADING_DIRECTORY;
    }
    win.draw_png(img, cel * cel_w, 0, cel_w, cel_h, dx, dy, dw, dh);
}

/*One grave stone at grow fraction g (0..1), shifted `pan` px left (intro-pan
camera; 0 in gameplay). repvz GridItem::DrawGraveStone (GridItem.cpp:141-187):
the stone stands bottom-anchored on its tile with the board's per-cell look
jitter (mGridCelOffset = Rand(10)-5 on both axes, Board.cpp:78-79): the hash
below is the deterministic stand-in for that random offset. The grow-in is
the original's clip reveal, rendered here as a scale-in over
GRAVE_GROW_TICKS.*/
static void draw_grave_stone(int row, int col, float g, int pan)
{
    if (g <= 0.02f)
        return; // still underground (stagger delay)
    // World knob folds into the grow scale (grow-in already scales the stone).
    int gw = max(1, (int)lround(88 * g * WORLD_SCALE));
    int gh = max(1, (int)lround(100 * g * WORLD_SCALE));
    unsigned h = (unsigned)(row * 31 + col * 7 + 13);
    int jx = (int)(h % 10u) - 5;
    int jy = (int)((h * 17u) % 10u) - 5;
    win.draw_png(GRAVE_DIRECTORY,
                 cells[row][col].x1 + (BLOCK_WIDTH - gw) / 2 + jx - pan,
                 cells[row][col].y1 + BLOCK_HEIGHT - 12 - gh + jy,
                 gw, gh);
}

/*The board's grave stones for the intro credit pans (seed chooser's
draw_credit_scene, credit.cpp's review_playground): same draw as gameplay
but panned with the camera, and without the Grave Buster chew scaling (no
busters exist before gameplay). The pan host ticks the grow clocks
(tick_grave_grows) so the stones rise while the camera travels.*/
void display_credit_graves(int pan)
{
    for (int row = 0; row < (int)cells.size(); row++)
        for (int col = 0; col < HORIZ_BLOCK_COUNT; col++)
            if (cells[row][col].has_grave)
                draw_grave_stone(row, col, level.grave_grow_progress(row, col), pan);
}

/*Display the game element: plants, zombies and the suns.*/
void display_game_elements()
{
    // Shadows
    display_shadow();
    display_mowers(game_characters.mowers);

    for (int row = 0; row < (int)cells.size(); row++)
    {
        // Night grid items sit under everything else on their tile: the
        // crater is a floor scar, the grave stone stands behind the plants.
        for (int col = 0; col < HORIZ_BLOCK_COUNT; col++)
        {
            if (cells[row][col].has_crater)
                display_crater(row, col);
            if (cells[row][col].has_grave)
            {
                // The stone scales in over GRAVE_GROW_TICKS with a random
                // stagger (a mid-wave sprout; the initial stones, which start
                // at mGridItemCounter = -Rand(50), have already risen during
                // the intro pan -- see display_credit_graves).
                float g = level.grave_grow_progress(row, col);
                // Grave Buster chew: the stone scales down with the chew
                // timer and is gone the tick the buster finishes.
                g *= 1.0f - grave_eaten_fraction(row, col);
                draw_grave_stone(row, col, g, 0);
            }
        }

        // Zomboni ice trail (retail Board::DrawIce, Board.cpp:6061): ice.png
        // tiled from the ice front back to the street edge, the front capped
        // by ice_cap.png; bobsled teams slide on it.
        if (row < (int)level.ice_min_x.size() && level.ice_min_x[row] >= 0)
        {
            const int ice_front = level.ice_min_x[row];
            const int ice_y = cells[row][0].y1 + 30;
            const int ice_h = GRID_BLOCK_H - 45;
            // Tiles start 13px past the front so the 25px cap overlaps the
            // first tile (no seam); the last tile is clipped at the edge.
            for (int x = ice_front + 13; x < WINDOW_WIDTH; x += 150)
            {
                int tw = min(150, WINDOW_WIDTH - x);
                win.draw_png(ICE_TRAIL_DIRECTORY, 0, 0, tw, 72, x, ice_y, tw, ice_h);
            }
            win.draw_png(ICE_CAP_DIRECTORY, ice_front, ice_y, 25, ice_h);
        }

        // Plants
        for (auto &plant : game_characters.plants)
            plant->display(row);

        // Ladder zombie's placed ladder leans over the defensive plant on its
        // tile (Zombie_ladder_1.png, 77x121) so zombies visibly climb it.
        for (int col = 0; col < HORIZ_BLOCK_COUNT; col++)
            if (cells[row][col].has_ladder)
            {
                const int lw = world_px(77), lh = world_px(121);
                win.draw_png(PLACED_LADDER_DIRECTORY,
                             cells[row][col].x1 + (cells[row][col].x2 -
                                                   cells[row][col].x1 - lw) / 2,
                             cells[row][col].y2 - world_px(117), lw, lh);
            }

        // 4-5 vasebreaker: standing vases on this row cover the tile and its
        // content until smashed (no zombies/plants on a live vase's tile).
        vasebreaker::display_row(row);

        // Zombies
        display_zombies(game_characters.zombies, row);
        display_zombie_parts(game_characters.zombie_parts, row);
        // Pending grave risers in their last GRAVE_RISE_EMERGE_TICKS slide up
        // through the ground clip (repvz UpdateZombieRiseFromGrave) -- drawn
        // with the row's zombies (particle layer above in the original, hence
        // before the dirt-burst FX below keeps the mound over their feet).
        for (const auto &g : level.grave_rises)
            if (g.row == row && g.riser != nullptr)
                g.riser->display_grave_rise(g.clk);
        // Transient reanim effects (water splashes, grave dirt bursts) top
        // off the row.
        reanimfx::display(row);
        // The jalapeno's lane fire burns over everything in its row
        // (repvz RENDER_LAYER_PARTICLE order).
        jalapeno_fx::display_row(row);
    }
    // Others
    for (auto &bullet : game_characters.bullets)
        bullet->display();
    display_rolling_nuts();
    // Suns are drawn after fogov::display() in display_game_layout so the fog
    // bank never covers them (see the fog hook there).
}

/*Display the game when paused.*/
void display_game_paused_elements()
{
    display_game_layout();
    display_game_elements();
    PAUSE_ICON.display(PAUSE_ICON_PRESS_DIRECTORY);
    win.draw_bg(BLACK_SCREEN_DIRECTORY);
}

/*If any plant seed is chosen: render it (transparent) at the mouse position.*/
void display_chosen_plant()
{
    int _x = 0, _y = 0;
    SDL_GetMouseState(&_x, &_y);
    if (player.is_shoveling)
    {
        // Held shovel at retail size: Shovel.png is 81x88 drawn 1:1 in the
        // original; Shovel2.png (90x97) width-scaled from h=88 lands ~82 px.
        const int SHOVEL_HEIGHT = 88;
        win.draw_png_width_scaled(SHOVEL_DIRECTORY, _x, _y - SHOVEL_HEIGHT, SHOVEL_HEIGHT);
        return;
    }
    if (PEASHOOTER_TYPE <= SeedPacket::chosen_plant && SeedPacket::chosen_plant < PLANT_COUNT)
    {
        _x -= ICON_WIDTH >> 2;
        _y -= ICON_HEIGHT >> 1;
        if (belt.active && belt.bowling)
        {
            // Bowling level: the packet in hand is the nut itself.
            int nut_img = SeedPacket::chosen_plant == CHERRYBOMB_TYPE ? EXPLODEONUT_BOWLING_DIRECTORY : WALLNUT_BOWLING_DIRECTORY;
            win.draw_png_width_scaled(nut_img, _x, _y, ICON_HEIGHT);
            return;
        }
        win.draw_png_width_scaled(PLANT_ICON_DIR[SeedPacket::chosen_plant], _x, _y, ICON_HEIGHT);
    }
}

/*
Make row when column blink when player is
about to plant or shovel in that row/column.

Highlight bands are drawn runtime (translucent white tile rects) instead of the
retired yard_blink/Frontyard{cr}.png overlays: the baked set only knew the old
5-row x 100 px frontyard grid, so a 6-row pool lane or a sloped roof band
could never line up. Drawing from the live grid keeps every board exact (pool
6 x 85, roof 5 x 85 with the per-column slope step from roof_tile_dy).
*/
void blink_row_and_col_at(int mouse_x, int mouse_y)
{
    int row = get_block_row(mouse_x, mouse_y);
    int col = get_block_col(mouse_x);
    if (row < 0 || col < 0)
        return;
    // Blink is limited to the level's playable lanes.
    if (row < level.playable_rows.front() || row > level.playable_rows.back())
        return;
    const int R0 = level.playable_rows.front();
    const int R1 = level.playable_rows.back();
    const int A = 100; // same strength the baked PNGs applied (Image alpha 100)
    // Row band: one rect per column so the band steps with the roof slope.
    for (int c = 0; c < HORIZ_BLOCK_COUNT; c++)
        win.sprite_batch().fillRect((float)cells[row][c].x1,
                                    (float)(cells[row][c].y1 + roof_tile_dy(c)),
                                    (float)BLOCK_WIDTH, (float)GRID_BLOCK_H,
                                    255, 255, 255, A);
    // Column band: from the first to the last playable row of this column.
    win.sprite_batch().fillRect((float)cells[R0][col].x1,
                                (float)(cells[R0][col].y1 + roof_tile_dy(col)),
                                (float)BLOCK_WIDTH,
                                (float)((R1 - R0 + 1) * GRID_BLOCK_H),
                                255, 255, 255, A);
}

void blink_row_and_col()
{
    int _x, _y;
    SDL_GetMouseState(&_x, &_y);
    blink_row_and_col_at(_x, _y);
}