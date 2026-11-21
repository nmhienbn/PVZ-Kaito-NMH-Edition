#include "seed_chooser.hpp"

#include "draw/ImageType.hpp"
#include "draw/atlas_font.hpp"
#include "draw/render_elements.hpp" // display_credit_graves
#include "draw/rsdl.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Player/Player.hpp"
#include "elements/SeedPacket/SeedPacket.hpp"
#include "elements/elements_actions.hpp" // tick_grave_grows
#include "elements/zombies/zombie_type.hpp"
#include "events/almanac/almanac.hpp"
#include "events/dave_scene/dave_scene.hpp"
#include "events/in_game/credit/credit.hpp"
#include "game_param.hpp"
#include "level/debut_data.hpp" // plant_packet_bg_dir
#include "level/level_data.hpp"
#include "music/music.hpp"

#include <algorithm>

extern bool quit;
extern Level level;
extern Player player;
extern Window win;

namespace
{/*Native 2Pak SeedChooser panel art (window is 1050x600): the panel
(SeedChooser_Background.png 465x513) docks right of the seed bank at
PANEL_X0 (the bank column's right edge + the original 9px gap, face ending
at PANEL_X0 + 444), and everything on it (title, catalog, ROCK, scroll
arrows) is panel-local: absolute = PANEL_X0 + panel_x + local. The
chosen-seeds strip is the in-game seed bank verbatim at its absolute
in-game spot (plant_seed column, render_elements.cpp) - it does NOT ride
the panel, it floats over the background like the original's bank floats
over the lawn.*/
const int PANEL_W = 465, PANEL_H = 513;
const int PANEL_Y0 = 87;
const int PANEL_X0 = ICON_BAR_X1 + ICON_WIDTH + 9; // == 134
const int PANEL_TRAVEL = PANEL_W + PANEL_X0;       // fully off-screen left -> docked
const int SLIDE_IN_TICKS = 19;  // travel grew by PANEL_X0; 19 keeps ~31 px/tick
const int SLIDE_OUT_TICKS = 15; // was 12 for -PANEL_W..0; 15 keeps ~39 px/tick

/*Credit pan (rework 2): the background is 1400 px wide, so the street-side
view stops at 1400 - WINDOW_WIDTH; the pan runs at credit.cpp's 10 px/tick.*/
const int PAN_MAX = 1400 - WINDOW_WIDTH;
const int PAN_STEP = 10;

/*Scrolling packet list (PvZ2-style): cells are the exact in-game packet size
(ICON_WIDTH x ICON_HEIGHT) and TOUCH like the seed bank / almanac grid
(CELL_DX == CELL_W, almanac ALMANAC_PLANT_DX == 105), 4 columns x
VISIBLE_ROWS of the pool on screen, the rest reached by arrows / drag /
wheel / the slim rail scrollbar. GRID_X0/GRID_Y0 are PANEL-LOCAL: four
touching 105px columns are a 420px block; +21 parks it on the panel's brown
face (art-local x 20..442) with an even 1px margin on both sides.*/
const int GRID_COLS = 4;
const int GRID_X0 = 21; // panel-local: centers the 4*105 block on the face
const int GRID_Y0 = PANEL_Y0 + 96;
const int CELL_W = 105; // == ICON_WIDTH (in-game packet width)
const int CELL_H = 65;  // == ICON_HEIGHT
const int CELL_DX = CELL_W; // cells touch like the in-game seed bank
const int CELL_DY = 68;
const int VISIBLE_ROWS = 5;
const int DRAG_THRESHOLD = 12;

/*Slim scrollbar docked on the panel's right rail, panel-local x
BAR_LOCAL_X..(almanac idiom, almanac_draw_plants_scrollbar: thin groove
track + square thumb). Span: the visible grid rows.*/
const int BAR_LOCAL_X = GRID_X0 + GRID_COLS * CELL_DX + 5; // 446
const int BAR_W = 10;
const int BAR_THUMB = 12; // thumb height, square-ish on the 10px track

const Button ROCK_RECT(285, 285 + 156, 548, 548 + 42);
const Button EYE_RECT(900, 900 + 111, 560, 560 + 28);
/*MENU / VIEW ALMANAC ride the window's top-right corner like the original's
mMenuButton / mAlmanacButton (repvz Board.cpp:3385, CutScene.cpp:682).*/
const Button MENU_RECT(912, 912 + 128, 8, 8 + 30);
const Button ALMANAC_RECT(912, 912 + 128, 44, 44 + 30);
const Button SCROLL_UP_RECT(16, 16 + 52, 548, 548 + 36);
const Button SCROLL_DOWN_RECT(76, 76 + 52, 548, 548 + 36);

/*Packet (world back + plant + sun tag + price) at any width; ICON_WIDTH is
the reference scale (the in-game bank size). The back is the plant's DEBUT
world (plant_packet_bg_dir), so a night plant keeps its dark art even on a
day level. The plant art shares the in-game packet's world-height sizing
(draw_plant_icon_world at k = sc): tall plants tower, small ones stay small,
oversize arts clamp into the art box.*/
void draw_packet_scaled(int plant_type, int x, int y, int w)
{
    float sc = w / (float)ICON_WIDTH;
    int h = (int)(ICON_HEIGHT * sc + 0.5f);
    win.draw_png(plant_packet_bg_dir(plant_type), x, y, w, h);

    // Common art box: full-bleed under the border, but outside the sun tag
    // and the price line (the tag covers x+58..x+99, y+25..y+66 unscaled).
    draw_plant_icon_world(plant_type,
                          x + (int)((PACKET_ICON_BOX_X + PACKET_ICON_BOX_W / 2.0f) * sc),
                          y + (int)((PACKET_ICON_BOX_Y + PACKET_ICON_BOX_H) * sc),
                          (int)(PACKET_ICON_BOX_W * sc), (int)(PACKET_ICON_BOX_H * sc),
                          sc);
    win.draw_png_height_scaled(SUN_TAG_DIRECTORY, x + (int)(58 * sc), y + (int)(25 * sc),
                               (int)(41 * sc));
    win.draw_png_height_scaled(SEED_PACKET_BOT_BORDER_DIRECTORY, x, y + (int)(60 * sc), w);
    int cost = PLANT_SUN_COST[plant_type];
    int fsize = std::max(10, (int)(30 * sc));
    int tw, th;
    TTF_SizeText(win.get_font(PVZUI_TTF, fsize), to_string(cost).c_str(), &tw, &th);
    tw = x + w - (int)(4 * sc) - tw;
    th = y + h - th;
    win.show_text_outlined(to_string(cost), tw, th, WHITE, PVZUI_TTF, fsize);
}

/*The credit frame: the intro pan background (previous stage when the sod
roll is about to play, like credit.cpp's review_playground) with the level's
credit zombies marching. pan is the camera offset (0 = house,
PAN_MAX = street).*/
void draw_credit_scene(vector<Zombie *> &roster, int pan)
{
    win.draw_bg(sod_prev_background(), pan, 0);
    display_credit_graves(pan); // night stones rise while the camera travels
    for (auto zombie : roster)
        zombie->display_credited(pan);
}

/*Chosen-seeds strip: the in-game seed bank verbatim (render_elements.cpp
display_game_layout): sun bar at (5,5) width 180 with the level's initial sun
at (100,17), and the picks as ICON_WIDTH x ICON_HEIGHT packets in the
plant_seed column (ICON_BAR_X1, ICON_BAR_Y1+1 + i*ICON_HEIGHT), unchosen slots
as silhouettes. Absolute: the catalog no longer touches the bank (plan B) -
it floats over the background at the exact in-game spot, docked or sliding,
like the original's bank floats over the lawn. Only while the panel is in
play (the original hides the seed bank above the screen during the intro
pan).*/
void draw_bank(const SeedChooser &sc)
{
    win.draw_png_height_scaled(SUN_BAR_DIRECTORY, 5, 5, 180);
    win.set_style(PVZUI_TTF, 26, TTF_STYLE_BOLD);
    win.show_text(to_string(player.sun_count), 100, 17, BLACK, PVZUI_TTF, 30);
    for (int i = 0; i < SEED_SLOT_COUNT; i++)
    {
        int x = plant_seed[i].x1, y = plant_seed[i].y1;
        if (i < (int)sc.chosen.size())
        {
            draw_packet_scaled(sc.chosen[i], x, y, ICON_WIDTH);
        }
        else
        {
            // Empty slot: full slot footprint like the seeded packets next
            // to it, not an aspect-kept thumbnail (user report: the empty
            // column read much narrower than the packets).
            SeedPacket::display_empty(x, y);
        }
    }
}

void draw_eye_button(SeedChooser::State state)
{
    EYE_RECT.display(SEEDCHOOSER_BUTTON2_DIRECTORY);
    // label: READABLE yellow TTF (user request: buttons back to English --
    // the pre-atlas yellow fill + shadow), centered like the dwarven atlas
    // it replaces (16 pt at 0.9x atlas scale).
    const char *label = state == SeedChooser::VIEW_LAWN ? "BACK" : "VIEW LAWN";
    int tw, th;
    TTF_SizeText(win.get_font(DWARVESC_TTF, 16), label, &tw, &th);
    win.show_text_shadowed(label, (EYE_RECT.x1 + EYE_RECT.x2 - tw) / 2,
                          (EYE_RECT.y1 + EYE_RECT.y2 - th) / 2 - 4,
                          RGB(255, 216, 64), DWARVESC_TTF, 16, 0, 1, RGB(96, 64, 0));
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    if (EYE_RECT.is_mouse_in(mx, my))
        EYE_RECT.display(SEEDCHOOSER_BUTTON2_GLOW_DIRECTORY);
}

void draw_win_button(const Button &rect, const char *label)
{
    rect.display(SEEDCHOOSER_BUTTON2_DIRECTORY);
    // READABLE yellow TTF (pre-atlas look), 15 pt at the old 0.85x scale.
    int tw, th;
    TTF_SizeText(win.get_font(DWARVESC_TTF, 15), label, &tw, &th);
    win.show_text_shadowed(label, (rect.x1 + rect.x2 - tw) / 2,
                          (rect.y1 + rect.y2 - th) / 2 - 3,
                          RGB(255, 216, 64), DWARVESC_TTF, 15, 0, 1, RGB(96, 64, 0));
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    if (rect.is_mouse_in(mx, my))
        rect.display(SEEDCHOOSER_BUTTON2_GLOW_DIRECTORY);
}

bool rock_armed(const SeedChooser &sc)
{
    return (int)sc.chosen.size() == sc.needed;
}

void draw_rock_button(const SeedChooser &sc, int anchor_x)
{
    Button rock(anchor_x + ROCK_RECT.x1, anchor_x + ROCK_RECT.x2,
                ROCK_RECT.y1, ROCK_RECT.y2);
    if (rock_armed(sc))
    {
        rock.display(SEEDCHOOSER_BUTTON_DIRECTORY);
        int mx, my;
        SDL_GetMouseState(&mx, &my);
        if (rock.is_mouse_in(mx, my))
            rock.display(SEEDCHOOSER_BUTTON_GLOW_DIRECTORY);
        AtlasFont::yellow().draw_centered(win, "LET'S ROCK!", (rock.x1 + rock.x2) / 2,
                                          (rock.y1 + rock.y2) / 2 - 4, 1.15f);
    }
    else
    {
        rock.display(SEEDCHOOSER_BUTTON_DISABLED_DIRECTORY);
        AtlasFont::yellow().draw_centered(win, "LET'S ROCK!", (rock.x1 + rock.x2) / 2,
                                          (rock.y1 + rock.y2) / 2 - 4, 1.15f, RGB(140, 140, 140));
    }
}

/*Slim scrollbar on the panel's right rail (rides the slide like the grid
cells): the almanac's bar recipe (almanac_draw_plants_scrollbar) at 10px --
the 2Pak slider groove rotated 90 for the track, the slider knob's 24x24
opaque core cropped square for the thumb.*/
void draw_scrollbar(const SeedChooser &sc, int slide_x)
{
    SDL_Rect track, thumb;
    if (!seed_chooser_scrollbar_geom(sc, track, thumb))
        return;
    int cx = slide_x + track.x + track.w / 2, cy = track.y + track.h / 2;
    win.draw_png(ALMANAC_SLIDERSLOT_DIRECTORY, cx - track.h / 2, cy - track.w / 2,
                 track.h, track.w, 90);
    win.draw_png(SLIDER_DIRECTORY, 0, 2, 24, 24,
                 slide_x + thumb.x, thumb.y, thumb.w, thumb.h);
}

/*Panel: 2Pak chooser background (the CHOOSE YOUR SEEDS plaque is its top
band) docked at PANEL_X0, the scrolled window of packets over its wood body,
the rail scrollbar beside them, scroll arrows and LET'S ROCK on the bottom
shelf. slide_x is the slide offset (<0 while sliding; base = PANEL_X0 +
slide_x). The packet backs that sit on it are debut-world keyed
(plant_packet_bg_dir).*/
void draw_panel(const SeedChooser &sc, int slide_x)
{
    const int base = PANEL_X0 + slide_x;
    win.draw_png(SEEDCHOOSER_BACKGROUND_DIRECTORY, base, PANEL_Y0, PANEL_W, PANEL_H);
    // Title centered ON the plaque: the real DwarvenTodcraft18Yellow atlas
    // glyphs at native size (repvz SeedChooserScreen [CHOOSE_YOUR_PLANTS] =
    // FONT_DWARVENTODCRAFT18YELLOW). The green band is art-local y ~2..31 of
    // SeedChooser_Background.png, so the ink centers on it; any lower and it
    // hangs onto the wood body.
    const char *title = "CHOOSE YOUR SEEDS";
    AtlasFont::yellow().draw_centered(win, title, base + PANEL_W / 2, PANEL_Y0 + 2 + 14);

    int mx, my;
    SDL_GetMouseState(&mx, &my);
    const int row_lo = sc.scroll_row, row_hi = sc.scroll_row + VISIBLE_ROWS;
    for (int i = 0; i < (int)sc.pool.size(); i++)
    {
        int row = i / GRID_COLS;
        if (row < row_lo || row >= row_hi)
            continue;
        Button cell = seed_chooser_grid_cell(i, sc.scroll_row);
        cell.x1 += slide_x;
        cell.x2 += slide_x;
        bool picked = find(sc.chosen.begin(), sc.chosen.end(), sc.pool[i]) != sc.chosen.end();
        draw_packet_scaled(sc.pool[i], cell.x1, cell.y1, CELL_W);
        if (picked)
            win.draw_png(BLACK_SCREEN_DIRECTORY, cell.x1, cell.y1, CELL_W, CELL_H);
        else if (cell.is_mouse_in(mx, my))
            win.draw_png(WHITE_SCREEN_DIRECTORY, cell.x1, cell.y1, CELL_W, CELL_H);
    }

    // Scroll arrows + window indicator, on the shelf next to LET'S ROCK.
    if (sc.max_scroll_row() > 0)
    {
        Button up(base + SCROLL_UP_RECT.x1, base + SCROLL_UP_RECT.x2,
                  SCROLL_UP_RECT.y1, SCROLL_UP_RECT.y2);
        Button down(base + SCROLL_DOWN_RECT.x1, base + SCROLL_DOWN_RECT.x2,
                    SCROLL_DOWN_RECT.y1, SCROLL_DOWN_RECT.y2);
        up.display(BACK_BUTTON_DIRECTORY, 90);
        down.display(BACK_BUTTON_DIRECTORY, -90);
        if (sc.scroll_row == 0)
            up.display(BLACK_SCREEN_DIRECTORY); // dim the dead arrow
        if (sc.scroll_row == sc.max_scroll_row())
            down.display(BLACK_SCREEN_DIRECTORY);
        char range[24];
        snprintf(range, sizeof(range), "%d-%d / %d",
                 sc.scroll_row * GRID_COLS + 1,
                 std::min((sc.scroll_row + VISIBLE_ROWS) * GRID_COLS, (int)sc.pool.size()),
                 (int)sc.pool.size());
        Button tag(base + 136, base + 276, 548, 548 + 36);
        tag.show_text(range, 0, 2, 15, RGB(231, 222, 100), RGB(43, 44, 58));
        draw_scrollbar(sc, slide_x);
    }
    draw_rock_button(sc, base);
}

/*Where the camera wants to be per state: street end for the credit pauses and
the chooser (repvz: the chooser slides on after the pan-right completes),
rewound to the lawn for VIEW_LAWN and the ROCK hand-off.*/
int camera_target(SeedChooser::State state)
{
    switch (state)
    {
    case SeedChooser::VIEW_LAWN:
    case SeedChooser::CREDIT_OUT:
        return 0;
    default:
        return PAN_MAX;
    }
}

int clampi(int v, int lo, int hi)
{
    return v < lo ? lo : v > hi ? hi
                                : v;
}

} // namespace

Button seed_chooser_grid_cell(int idx, int scroll_row)
{
    int x = PANEL_X0 + GRID_X0 + (idx % GRID_COLS) * CELL_DX;
    int y = GRID_Y0 + (idx / GRID_COLS - scroll_row) * CELL_DY;
    return Button(x, x + CELL_W, y, y + CELL_H);
}

Button seed_chooser_rock_rect()
{
    return Button(PANEL_X0 + ROCK_RECT.x1, PANEL_X0 + ROCK_RECT.x2,
                  ROCK_RECT.y1, ROCK_RECT.y2);
}

Button seed_chooser_eye_rect()
{
    return EYE_RECT;
}

Button seed_chooser_menu_rect()
{
    return MENU_RECT;
}

Button seed_chooser_almanac_rect()
{
    return ALMANAC_RECT;
}

Button seed_chooser_scroll_up_rect()
{
    return Button(PANEL_X0 + SCROLL_UP_RECT.x1, PANEL_X0 + SCROLL_UP_RECT.x2,
                  SCROLL_UP_RECT.y1, SCROLL_UP_RECT.y2);
}

Button seed_chooser_scroll_down_rect()
{
    return Button(PANEL_X0 + SCROLL_DOWN_RECT.x1, PANEL_X0 + SCROLL_DOWN_RECT.x2,
                  SCROLL_DOWN_RECT.y1, SCROLL_DOWN_RECT.y2);
}

Button seed_chooser_chosen_slot(int i)
{
    return plant_seed[i];
}

int seed_chooser_grid_cols()
{
    return GRID_COLS;
}

int seed_chooser_visible_rows()
{
    return VISIBLE_ROWS;
}

int seed_chooser_cell_w()
{
    return CELL_W;
}

int seed_chooser_cell_h()
{
    return CELL_H;
}

/*Docked scrollbar geometry (harness anchor; at run time the drawn bar also
rides the slide offset): the 10px rail right of the 4-column grid, spanning
the visible rows; thumb top = track.y + travel * scroll_row / max_row.
false when the pool fits on one screen (no bar drawn, no drag).*/
bool seed_chooser_scrollbar_geom(const SeedChooser &sc, SDL_Rect &track, SDL_Rect &thumb)
{
    const int max_row = sc.max_scroll_row();
    if (max_row <= 0)
        return false;
    track = {PANEL_X0 + BAR_LOCAL_X, GRID_Y0, BAR_W,
             VISIBLE_ROWS * CELL_DY - (CELL_DY - CELL_H)}; // row0 top .. row4 bottom
    thumb = {track.x, track.y + (track.h - BAR_THUMB) * sc.scroll_row / max_row,
             BAR_W, BAR_THUMB};
    return true;
}

/*Row the slim-bar drag maps a thumb top to (mirror of the almanac's
almanac_scroll_row_at).*/
static int scroll_row_for_thumb(const SeedChooser &sc, int thumb_top)
{
    SDL_Rect track, thumb;
    if (!seed_chooser_scrollbar_geom(sc, track, thumb))
        return 0;
    return clampi((int)lroundf((thumb_top - track.y) * sc.max_scroll_row() /
                               (float)(track.h - thumb.h)),
                  0, sc.max_scroll_row());
}

int seed_chooser_panel_x0()
{
    return PANEL_X0;
}

int seed_chooser_panel_w()
{
    return PANEL_W;
}

int SeedChooser::max_scroll_row() const
{
    int rows = ((int)pool.size() + GRID_COLS - 1) / GRID_COLS;
    return std::max(0, rows - VISIBLE_ROWS);
}

int SeedChooser::cell_at(int x, int y) const
{
    const int GX = PANEL_X0 + GRID_X0; // docked absolute grid origin
    if (y < GRID_Y0 || y >= GRID_Y0 + VISIBLE_ROWS * CELL_DY)
        return -1;
    int col = (x - GX) / CELL_DX;
    int row = (y - GRID_Y0) / CELL_DY;
    if (col < 0 || col >= GRID_COLS || row < 0 || row >= VISIBLE_ROWS)
        return -1;
    // inside the row/column pitch but past the packet edge: no cell
    if ((x - GX) % CELL_DX >= CELL_W || (y - GRID_Y0) % CELL_DY >= CELL_H)
        return -1;
    int idx = (scroll_row + row) * GRID_COLS + col;
    return idx < (int)pool.size() ? idx : -1;
}

void SeedChooser::toggle_pick(int pool_idx)
{
    auto it = find(chosen.begin(), chosen.end(), pool[pool_idx]);
    if (it != chosen.end())
    {
        chosen.erase(it);
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
    }
    else if ((int)chosen.size() < SEED_SLOT_COUNT)
    {
        chosen.push_back(pool[pool_idx]);
        play_sound_effect(SEED_LIFT_MUSIC_DIRECTORY);
    }
}

/*Bank-strip click (panel docked): (x, y) inside the packet of chosen slot i
sends that seed back to the pool list. Only filled slots answer; silhouette
slots stay inert like the original's empty bank.*/
bool SeedChooser::unpick_chosen_at(int x, int y)
{
    for (int i = 0; i < (int)chosen.size(); i++)
        if (seed_chooser_chosen_slot(i).is_mouse_in(x, y))
        {
            chosen.erase(chosen.begin() + i);
            play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
            return true;
        }
    return false;
}

void SeedChooser::init()
{
    pool = unlocked_plant_types();

    // Pre-fill from a previous pick of this level (restart reload or replay
    // puts it back into level.seed_plants), filtered to still-unlocked seeds.
    for (int type : level.seed_plants)
    {
        if ((int)chosen.size() >= SEED_SLOT_COUNT)
            break;
        if (find(pool.begin(), pool.end(), type) != pool.end() &&
            find(chosen.begin(), chosen.end(), type) == chosen.end())
            chosen.push_back(type);
    }
    needed = min(SEED_SLOT_COUNT, (int)pool.size());
    // No packet may look "held" while browsing the chooser.
    SeedPacket::chosen_plant = PLANT_COUNT;

    /*Credit zombies of the level, one per roster type (JSON roster first,
    generated-wave census for legacy levels), left on their make_credit lawn
    spots like credit.cpp's pan.*/
    vector<int> roster_types = get_level_roster_types();
    if (roster_types.empty())
    {
        for (int i = NORMAL_TYPE; i < COUNT_ZOMBIE_TYPE; i++)
        {
            if (i == FLAG_TYPE || i >= (int)level.count_zombie_type.size() ||
                level.count_zombie_type[i] <= 0)
                continue;
            roster_types.push_back(i);
        }
    }
    for (int type : roster_types)
    {
        Zombie *zombie = init_zombie(type, level.level_num);
        zombie->make_credit();
        roster.push_back(zombie);
    }

    // Night stones sprout over the coming credit pan (tick_grave_grows in
    // tick, display_credit_graves in the scene): herald them with the same
    // rumble foley the mid-wave sprout uses (FOLEY_GRAVESTONE_RUMBLE,
    // repvz Zombie.cpp:8341).
    if (!level.grave_grows.empty())
        play_sound_effect(GRAVESTONE_RUMBLE_MUSIC_DIRECTORY);
}

void SeedChooser::render()
{
    // Blocking scene pacing itself at the 60 fps cap: force the auto-FPS
    // gate so the gated callees below (show_announcer_text's countdown
    // over the credit pan, display_credited walk frames) keep advancing
    // per frame -- logic_ticked would otherwise hold a stale value from
    // the last gameplay frame.
    logic_ticked = 1;
    win.clear_renderer();
    draw_credit_scene(roster, camera);

    bool panel_states = state == SLIDE_IN || state == CHOOSING || state == SLIDE_OUT;
    if (panel_states)
    {
        draw_panel(*this, panel_x);
        draw_bank(*this); // absolute in-game spot, does not ride the slide
    }
    else
    {
        win.show_announcer_text(); // "<PLAYER>'S TRIP" over the credit pan
    }

    if (state == VIEW_LAWN || state == CHOOSING)
        draw_eye_button(state);

    if (state == CHOOSING)
    {
        draw_win_button(seed_chooser_menu_rect(), "MENU");
        if (player.unlocked_level >= ALMANAC_UNLOCK_LEVEL)
            draw_win_button(seed_chooser_almanac_rect(), "VIEW ALMANAC");
    }
}

bool SeedChooser::tick()
{
    // Night stones rise during the chooser's pan (repvz sprouts the initial
    // graves in the level-intro cutscene, not after READY-SET-PLANT); one
    // tick per frame, the clocks are spent long before the panel docks.
    tick_grave_grows();
    render();
    win.update_screen();

    HANDLE_SDL_EVENT(
        QUIT();
        LCLICK({
            if (state == CHOOSING)
            {
                SDL_Rect bar_track;
                SDL_Rect bar_thumb;
                const bool bar_shown = seed_chooser_scrollbar_geom(*this, bar_track, bar_thumb);
                if (EYE_RECT.is_mouse_in(mouse_x, mouse_y))
                {
                    play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                    state = VIEW_LAWN; // BACK returns to the chooser
                }
                else if (seed_chooser_menu_rect().is_mouse_in(mouse_x, mouse_y))
                {
                    play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                    back_to_menu = true;
                    state = DONE;
                }
                else if (player.unlocked_level >= ALMANAC_UNLOCK_LEVEL &&
                         seed_chooser_almanac_rect().is_mouse_in(mouse_x, mouse_y))
                {
                    play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                    run_almanac(); // blocking scene; CLOSE lands back here
                }
                else if (rock_armed(*this) &&
                         seed_chooser_rock_rect().is_mouse_in(mouse_x, mouse_y))
                {
                    play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                    state = SLIDE_OUT;
                }
                else if (seed_chooser_scroll_down_rect().is_mouse_in(mouse_x, mouse_y))
                {
                    if (scroll_row < max_scroll_row())
                    {
                        scroll_row++;
                        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                    }
                }
                else if (seed_chooser_scroll_up_rect().is_mouse_in(mouse_x, mouse_y))
                {
                    if (scroll_row > 0)
                    {
                        scroll_row--;
                        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                    }
                }
                else if (unpick_chosen_at(mouse_x, mouse_y))
                {
                    // click on a picked packet in the bank strip: send the
                    // seed back (repvz SeedChooserScreen removes it from
                    // mSeedsInBothBank, SeedChooserScreen.cpp MouseDown)
                }
                else if (bar_shown && mouse_x >= bar_track.x &&
                         mouse_x < bar_track.x + bar_track.w &&
                         mouse_y >= bar_track.y &&
                         mouse_y < bar_track.y + bar_track.h)
                {
                    // press on the slim rail: grab the thumb (a track click
                    // jumps the thumb under the cursor), then drag follows
                    bar_drag = true;
                    bar_grab_dy = (mouse_y >= bar_thumb.y &&
                                   mouse_y < bar_thumb.y + bar_thumb.h)
                                      ? mouse_y - bar_thumb.y
                                      : BAR_THUMB / 2;
                    scroll_row = scroll_row_for_thumb(*this, mouse_y - bar_grab_dy);
                }
                else if (mouse_x >= PANEL_X0 + GRID_X0 &&
                         mouse_x < PANEL_X0 + GRID_X0 + GRID_COLS * CELL_DX &&
                         mouse_y >= GRID_Y0 &&
                         mouse_y < GRID_Y0 + VISIBLE_ROWS * CELL_DY)
                {
                    // press inside the grid view: a drag scrolls, a tap picks
                    press_active = true;
                    press_dragging = false;
                    press_y = mouse_y;
                    press_scroll_row = scroll_row;
                }
            }
            else if (state == VIEW_LAWN)
            {
                if (EYE_RECT.is_mouse_in(mouse_x, mouse_y))
                {
                    play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                    state = CHOOSING; // BACK from the lawn view
                }
            }
        });
        LRELEASE({
            bar_drag = false;
            if (press_active)
            {
                press_active = false;
                if (!press_dragging && state == CHOOSING)
                {
                    int idx = cell_at(mouse_x, mouse_y);
                    if (idx >= 0)
                        toggle_pick(idx);
                }
                press_dragging = false;
            }
        });
        if (bar_drag && e.type == SDL_MOUSEMOTION &&
            (e.motion.state & SDL_BUTTON_LMASK))
        {
            scroll_row = scroll_row_for_thumb(*this, e.motion.y - bar_grab_dy);
        }
        if (press_active && e.type == SDL_MOUSEMOTION &&
            (e.motion.state & SDL_BUTTON_LMASK))
        {
            int dy = e.motion.y - press_y;
            if (!press_dragging && (dy > DRAG_THRESHOLD || dy < -DRAG_THRESHOLD))
                press_dragging = true;
            if (press_dragging)
                scroll_row = clampi(press_scroll_row - dy / CELL_DY, 0, max_scroll_row());
        }
        if (e.type == SDL_MOUSEWHEEL && state == CHOOSING)
        {
            scroll_row = clampi(scroll_row - e.wheel.y, 0, max_scroll_row());
        });

    // camera easing
    int target = camera_target(state);
    if (camera < target)
        camera = min(target, camera + PAN_STEP);
    else if (camera > target)
        camera = max(target, camera - PAN_STEP);

    // The panel shows itself once the pan reaches the street (repvz: the
    // chooser slides on right after TimePanRightEnd, no click gate).
    if (state == CREDIT_PAN && camera == PAN_MAX)
    {
        state = SLIDE_IN;
        panel_x = -PANEL_TRAVEL; // fully off-screen left of the dock
    }
    else if (state == CREDIT_OUT && camera == 0)
    {
        rocked = true;
        state = DONE;
    }
    else if (state == SLIDE_IN)
    {
        panel_x += PANEL_TRAVEL / SLIDE_IN_TICKS;
        if (panel_x >= 0)
        {
            panel_x = 0;
            state = CHOOSING;
        }
    }
    else if (state == SLIDE_OUT)
    {
        panel_x -= PANEL_TRAVEL / SLIDE_OUT_TICKS;
        if (panel_x <= -PANEL_TRAVEL)
            state = CREDIT_OUT;
    }
    return state != DONE;
}

void SeedChooser::finish()
{
    for (auto zombie : roster)
        delete zombie;
    roster.clear();
    if (rocked)
    {
        set_level_seeds(chosen);
        // The chooser already ran the credit pan (and its rewind); the
        // in-game credit goes straight to the sod roll + ready-set-plant.
        credit_mark_chooser_pan();
    }
}

bool level_needs_chooser()
{
    if (level.seed_mode != "all_unlocked")
        return false;
    return !level.is_special(LevelSpecial::Vasebreaker) && !level.is_special(LevelSpecial::Boss);
}

SeedChooserExit run_seed_chooser()
{
    // Crazy Dave intro (2-1 night tips etc., first playthrough only) runs
    // before the credit pan; display_credit skips it when the chooser ran.
    dave::play_intro();
    play_music(ChooseYourSeeds_DIRECTORY); // carries over into the credit pan
    win.show_announcer_text(player.name + "'S TRIP TO PLANTS VS. ZOMBIES", 300);
    SeedChooser chooser;
    chooser.init();
    while (!quit && chooser.tick())
        ;
    chooser.finish();
    return chooser.back_to_menu ? SeedChooserExit::BACK_TO_MENU
                                : SeedChooserExit::ROCKED;
}
