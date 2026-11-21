#include "credit.hpp"
#include "draw/render_elements.hpp"
#include "draw/render_message.hpp"
#include "draw/rsdl.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/Player/Player.hpp"
#include "elements/elements_actions.hpp" // tick_grave_grows
#include "elements/zombies/zombie_type.hpp"
#include "events/dave_scene/dave_scene.hpp"
#include "game_param.hpp"
#include "level/level_data.hpp"
#include "reanim/one_shot_fx.hpp"
#include "reanim/pam_game_hooks.hpp"
#include <algorithm>

extern int game_state;
extern bool quit;
extern Level level;
extern Player player;
extern Window win;
extern Map cells;

/*
Display zombies outside playground
*/
void review_playground(vector<Zombie *> &zombies, const int &i)
{
    // Blocking scene pacing itself at the 60 fps cap: force the auto-FPS
    // gate so the gated callees below (show_announcer_text's countdown)
    // keep advancing per frame -- logic_ticked would otherwise hold a
    // stale value from the last gameplay frame.
    logic_ticked = 1;
    win.clear_renderer();
    // Background: when this level's sod roll is about to play, the pan still
    // shows the previous stage's lawn (the roll replaces it at reveal time).
    win.draw_bg(sod_prev_background(), i, 0);
    // Night gravestones rise during the pan itself (repvz sprouts the initial
    // stones in the level-intro cutscene): one grow tick per pan frame, drawn
    // panned with the camera.
    tick_grave_grows();
    display_credit_graves(i);
    // Zombies
    for (auto &zombie : zombies)
    {
        zombie->display_credited(i);
    }
    // Welcome text
    win.show_announcer_text();
    win.update_screen();
    HANDLE_SDL_EVENT(
        QUIT();
        LOST_FOCUS(
            set_status(game_state, IS_PAUSED, true);
            Mix_PauseMusic();
            Mix_Pause(-1););)
}

// One-shot flag: the seed chooser ran the credit pan (its CREDIT_PAN /
// CREDIT_OUT states); display_credit consumes it and skips its own pan
// (rework 2 flow: pan lives in the chooser for chooser levels).
static bool chooser_pan_pending = false;

void credit_mark_chooser_pan()
{
    chooser_pan_pending = true;
}

bool credit_chooser_pan_pending()
{
    return chooser_pan_pending;
}

/*
The classic credit pan: Dave intro, welcome announcer, zombie crowd from the
final waves, pan house -> street -> stop -> back, then free the textures.
*/
static void credit_pan_roll()
{
    // Crazy Dave intro (1-5 digging / 2-1 night tips, first playthrough only)
    // runs before the credits pan; no-op on other levels.
    dave::play_intro();
    play_music(ChooseYourSeeds_DIRECTORY);
    // set welcome announcer
    win.show_announcer_text(player.name + "\'S TRIP TO PLANTS VS. ZOMBIES", 300);
    // random credit zombies from final waves
    vector<Zombie *> tmp;

    for (int i = NORMAL_TYPE; i < COUNT_ZOMBIE_TYPE; i++)
    {
        int cnt = min(level.count_zombie_type[i] / 2, 7);
        for (int j = 0; j < cnt; j++)
        {
            auto t = init_zombie(i, level.level_num);
            t->make_credit();
            tmp.push_back(t);
        }
    }
    sort(tmp.begin(), tmp.end(), [](const Zombie *x, const Zombie *y) -> bool
         { return x->y_location < y->y_location; });

    // The stones rise over the coming pan (review_playground ticks them):
    // herald with the sprout rumble (FOLEY_GRAVESTONE_RUMBLE, repvz
    // Zombie.cpp:8341).
    if (!level.grave_grows.empty())
        play_sound_effect(GRAVESTONE_RUMBLE_MUSIC_DIRECTORY);

    // review playground
    for (int i = 0; i <= 60; i++)
    {
        review_playground(tmp, 0);
    }
    // house to outside
    for (int i = 0; i <= 1400 - WINDOW_WIDTH; i += 10)
    {
        review_playground(tmp, i);
    }
    // stop
    for (int i = 0; i <= 180; i++)
    {
        review_playground(tmp, 1400 - WINDOW_WIDTH);
    }
    // outside back to house
    for (int i = 1400 - WINDOW_WIDTH; i >= 0; i -= 10)
    {
        review_playground(tmp, i);
    }
    // remove credit zombies
    for (auto &zombie : tmp)
    {
        delete zombie;
    }
    tmp.clear();
}

/*
Reset speed to normal, and no paused.
Display welcome player
Display ready-set-plant
Now game is really start
 */
void display_credit()
{
    // reset some status
    set_status(game_state, IS_FAST, false);
    set_status(game_state, IS_PAUSED, false);
    if (chooser_pan_pending)
    {
        // The chooser already played Dave + the pan and ended rewound to the
        // lawn; pick up at the sod roll.
        chooser_pan_pending = false;
    }
    else
    {
        credit_pan_roll();
    }

    for (int i = ZOMBIE_CREDIT1_DIRECTORY; i <= DOOR_ZOMBIE_EATING_3_BLINK_DIRECTORY; i++)
        if (i != MOUSE_CURSOR_DIRECTORY)
        {
            win.delete_texture(i);
        }
    display_sod_roll();
    SDL_Log("credit: sod roll done, entering ready-set-plant");
    // ready - set - plant
    play_sound_effect(R_S_P_MUSIC_DIRECTORY);
    for (int clk = 0; clk <= 180; clk++)
    {
        win.clear_renderer();
        if (clk < 30)
        {
            display_ready_set_plant(START_READY_DIRECTORY);
        }
        else if (clk < 60)
        {
            display_ready_set_plant(START_SET_DIRECTORY);
        }
        else if (clk < 180)
        {
            display_ready_set_plant(START_PLANT_DIRECTORY);
        }
        else
        {
            display_ready_set_plant(START_PLANT_DIRECTORY);
            set_status(game_state, IS_GAME_STARTED, true);
        }
        win.update_screen();
        HANDLE_SDL_EVENT(
            QUIT();
            LOST_FOCUS(
                set_status(game_state, IS_PAUSED, true);
                Mix_PauseMusic();
                Mix_Pause(-1););)
    }
    SDL_Log("credit: ready-set-plant done");
}

/*
Day frontyard sod stage image for a row count: none -> LV0 (unsodded),
1 lane -> LV1, 3 lanes -> LV2, 5 lanes -> full lawn.
*/
static int sod_stage_dir(int rows)
{
    if (rows <= 0)
        return BACKGROUND_LV0_DIRECTORY;
    if (rows <= 1)
        return BACKGROUND_LV1_DIRECTORY;
    if (rows <= 3)
        return BACKGROUND_LV2_DIRECTORY;
    return BACKGROUND_DIRECTORY;
}

// Cached per chosen level: the previous level's row count when it was also a
// day frontyard stage, -1 when unknown/not applicable. Reviewed every intro
// frame, so keep the JSON read off the frame path. Level 1-1 never had a
// previous level: the original (repvz CutScene.cpp:692-699) treats it as
// rolling the middle lane onto the bare dirt, i.e. prev rows = 0.
static int sod_prev_rows_cache()
{
    static int cached_level = -1;
    static int cached_rows = -2;
    if (cached_level != level.level_num)
    {
        cached_level = level.level_num;
        if (level.level_num == 1)
            cached_rows = 0;
        else if (level.level_num > 1)
        {
            auto jprev = get_lv_json(level.level_num - 1);
            string pbg = jprev.value("background", string());
            if (pbg == "frontyard_1lane" || pbg == "frontyard_3lane" ||
                pbg == "frontyard_full")
                cached_rows = jprev.value("rows", GRID_ROWS);
            else
                cached_rows = -1;
        }
        else
            cached_rows = -1;
    }
    return cached_rows;
}

// Roll plays when the lawn gains sod rows vs. the previous day level:
// 1-1: nothing -> 1 row, 1-2: 1 -> 3 rows, 1-4: 3 -> 5 rows. Matches repvz
// CutScene.cpp:692 ("levels 1, 2 and 4" of adventure get mSodTime > 0).
bool sod_lawn_expands()
{
    // Only the day frontyard stages have sod composites (map_type 0..2).
    if (level.map_type > 2 || level.level_num <= 0)
        return false;
    int prev_rows = sod_prev_rows_cache();
    return prev_rows >= 0 && prev_rows < (int)level.playable_rows.size();
}

int sod_prev_background()
{
    int prev_rows = sod_lawn_expands() ? sod_prev_rows_cache() : -1;
    return prev_rows >= 0 ? sod_stage_dir(prev_rows) : level.background_directory;
}

/*
Horizontal span of the unrolling strip in screen pixels (the background is
drawn at pan offset 0 at roll time, so screen x == image x). Values from
repvz Board::DrawBackdrop (Board.cpp:6111-6134), the strip clips the original
uses per level: 1-1 sod1row at (239, 265) width 771; 1-2 sod3row at (235, 149)
width 771; 1-4 background1 clipped from src x=232, width 773.
*/
SodStrip sod_strip_geom()
{
    switch (sod_lawn_expands() ? sod_prev_rows_cache() : -1)
    {
    case 0:
        return {239, 771}; // 1-1: the middle lane rolls onto bare dirt
    case 1:
        return {235, 771}; // 1-2: 3 lanes
    case 3:
        return {232, 773}; // 1-4: the rest of the full lawn
    default:
        return {239, int(WINDOW_WIDTH) - 239};
    }
}

/*
Lanes the sod unrolls on: the current playable rows minus the previous day
stage's lanes (same centered layout as init_playable_rows): 1-1 {2}, 1-2
{1,3}, 1-4 {0,4}. Empty when nothing is gained. repvz spawns the SODROLL
rollers on exactly the gained lanes -- level 1 one roller, levels 2 and 4 two
(CutScene.cpp:1216-1234), not one per playable row.
*/
vector<int> sod_roll_rows()
{
    vector<int> rows;
    if (!sod_lawn_expands())
        return rows;
    int prev_rows = sod_prev_rows_cache();
    int prev0 = (GRID_ROWS - prev_rows) / 2;
    for (int row : level.playable_rows)
        if (row < prev0 || row >= prev0 + prev_rows)
            rows.push_back(row);
    return rows;
}

/*
Draw sod roll and sod roll cap riding the reveal edge of one lane. The strip
is exactly the lane band (cells[row][0].y1..y2, 100 px day lanes from the
live grid -- repvz fits the SOD1ROW/SOD3ROW strips band-tight to the lanes,
Board.cpp:6111-6134).
*/
void sod_roll(int row, int pos, int clk)
{
    int sod_width = 10 + 45 * (180 - clk) / 180;
    int cap_width = sod_width + 2;
    int y1 = cells[row][0].y1;
    int row_h = cells[row][0].y2 - y1;
    win.draw_png(SOD_ROLL, pos - sod_width / 2, y1, sod_width, row_h);
    win.draw_png(SOD_ROLL_CAP, pos - cap_width / 2, y1 + row_h - cap_width / 2, cap_width, cap_width, 8 * clk);
}

/*
One classic roll frame: the gained lanes reveal progressively over the base
background, left to right along the strip (sod_strip_geom) -- one band per
gained row, band height = the row's own height, so a 2-row expansion unrolls
two strips at once -- with the roll/cap sprites riding the reveal edge. Caller
clears the renderer and draws the base (previous-stage) background. At
prog == 1 the plain new background is drawn so the rolled lawn hands over to
gameplay without a seam.
*/
void draw_sod_roll_frame(float prog, int clk)
{
    if (prog >= 1.0f)
    {
        win.draw_bg(level.background_directory, 0, 0);
        return;
    }
    SodStrip strip = sod_strip_geom();
    int pos = strip.x0 + (int)(strip.w * prog);
    for (int row : sod_roll_rows())
    {
        int y1 = cells[row][0].y1;
        int row_h = cells[row][0].y2 - y1;
        if (pos > 0)
            win.draw_png(level.background_directory, 0, y1, pos, row_h,
                         0, y1, pos, row_h);
        sod_roll(row, pos, clk);
    }
    reanimfx::sod_dust(pos); // dust puff period at the roll's front edge + scene FX pump
}

/*
Display sod roll when this level's lawn gained sod rows (1-1: bare dirt -> 1
row, 1-2: 1->3 rows, 1-4: 3->5): the new lawn unrolls over the previous
stage's lawn.
*/
void display_sod_roll()
{
    if (!sod_lawn_expands())
        return;
    // PvZ2 PAM sod-roll intro when the pam assets are present.
    if (pamsod::display_sod_roll_intro())
        return;
    int prev_bg = sod_prev_background();
    for (int clk = 0; clk <= 180; clk++)
    {
        win.clear_renderer();
        win.draw_bg(prev_bg, 0, 0);
        draw_sod_roll_frame(clk / 180.0f, clk);
        win.update_screen();
        HANDLE_SDL_EVENT(
            QUIT();
            LOST_FOCUS(
                set_status(game_state, IS_PAUSED, true);
                Mix_PauseMusic();
                Mix_Pause(-1););)
    }
}