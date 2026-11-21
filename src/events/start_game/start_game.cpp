#include "start_game.hpp"
#include "LoadSprout.hpp"
#include "LoadZomHead.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/Player/Player.hpp"
#include "elements/button/button.hpp"
#include "elements/elements.hpp"
#include "level/level_data.hpp"
#include "reanim/reanim_plant.hpp"
#include "reanim/reanim_zombie.hpp"
#include <algorithm>
#include <filesystem>
#include <functional>

extern bool quit;
extern Level level;
extern Elements game_characters;
extern Player player;
extern Map cells;
extern Window win;

const int DISPLAY_LOGO_TIME = 200;

// Loading screen pacing: each rendered frame spends at most this long doing
// real load work (so the bar + pop animations keep redrawing at display
// rate), and the screen stays up at least this long so a fast disk still
// shows every marker pop to full.
const Uint32 LOAD_WORK_BUDGET_TICKS = 12;
const Uint32 MIN_LOADING_TICKS = 1300;

// Displayed bar tip: it chases the real progress but with a speed cap, so a
// heavy burst of loaded files never shoves the bar past several pop markers
// in one frame; markers still pop strictly in rolling order. 1200 px/s makes
// the smallest inter-marker gap (~60 px) take ~50 ms = ~3 frames at 60 fps,
// and a full 628 px catch-up (work finished early) ~0.53 s.
const double BAR_TIP_MAX_PX_PER_SEC = 1200.0;
// Per-frame delta-time clamp: with the speed cap this bounds each rendered
// step to 40 px < the ~57 px minimum gap between marker trigger positions, so
// no single frame can ever roll past two markers at once (also prevents the
// tip teleporting after a single task blocked for a long while).
const double BAR_TIP_MAX_DT_SEC = 33.0 / 1000.0;

vector<LoadSprout> sprouts{
    LoadSprout(240, SDL_FLIP_NONE, 80, 54),
    LoadSprout(300, SDL_FLIP_HORIZONTAL, 104, 70),
    LoadSprout(410, SDL_FLIP_NONE, 67, 45),
    LoadSprout(530, SDL_FLIP_HORIZONTAL, 89, 60)};
LoadZomHead zom_head(650, SDL_FLIP_NONE, 122, 104);

// PvZ1 title-screen start label (repvz TitleScreen.cpp: the [LOADING] /
// [CLICK_TO_START] HyperlinkWidget in Brianne-tod): mustard until hovered,
// then hot orange; drawn FLAT (no shadow), centered on the dirt bar.
const RGB UNTAP(218, 184, 33);
const RGB TAPPED(250, 90, 15);
const Button TAP_TO_START(204, 846, 490, 596);

/*Initialize game:
    Display loading and starting game.
    Read player data
    Create block on frontyard (playground)
*/
void init_game()
{
    play_music(OPENING_MUSIC_DIRECTORY);
    display_loading_screen();
    display_starting_screen();
    read_savedata();
    cells = create_a_collection_of_blocks();
}

/*
One starting-screen frame (split out of display_starting_screen so the
headless start-screen dump can render both tap states): the title art, the
dirt bar with its popped sprouts + zombie head, and the start label - PvZ1's
"Click to Start!" (repvz [CLICK_TO_START]) in flat Brianne-tod, centered on
the bar, UNTAP until the finger hovers, then TAPPED.
*/
void draw_starting_screen_frame(const bool &tap_hovered)
{
    win.draw_png_height_scaled(STARTING_SCREEN_DIRECTORY, 0, 0, WINDOW_WIDTH);
    win.draw_png(LOAD_BAR_DIRT_DIRECTORY, 204, 490, 642, 106);
    win.draw_png(LOAD_BAR_GRASS_DIRECTORY, 200, 455, 628, 66);
    for (auto &sprout : sprouts)
        sprout.display(INF);
    zom_head.display(INF);
    string loading = "Click to Start!";
    int w = 0, h = 0;
    TTF_SizeText(win.get_font(BRIANNE_TTF, 40), loading.c_str(), &w, &h);
    win.show_text(loading, 204 + (642 - w) / 2, 490 + (106 - h) / 2,
                  tap_hovered ? TAPPED : UNTAP, BRIANNE_TTF, 40);
}

/*
Display starting screen.
Updated: Now player can exit game from here.
Handle player click on TAP_TO_START
*/
void display_starting_screen()
{
    play_music(OPENING_MUSIC_DIRECTORY);
    bool game_started = false;
    bool quit = false;
    while (!quit && !game_started)
    {
        // Change color when mouse over TAP_TO_START
        int _x = 0, _y = 0;
        SDL_GetMouseState(&_x, &_y);
        draw_starting_screen_frame(TAP_TO_START.is_mouse_in(_x, _y));
        HANDLE_SDL_EVENT(
            QUIT();
            // Handle player click on TAP_TO_START
            LCLICK({
                if (TAP_TO_START.is_mouse_in(mouse_x, mouse_y))
                    game_started = true;
            });

        );

        win.update_screen();
    }
    win.fade_out();
    win.clear_renderer();
}

void display_logo()
{
    for (int clk = 0; clk < DISPLAY_LOGO_TIME; clk++)
    {
        int alpha = 510 * (clk <= (DISPLAY_LOGO_TIME / 2) ? clk : DISPLAY_LOGO_TIME - clk) / DISPLAY_LOGO_TIME;
        win.draw_bg(BLACK_SCREEN_DIRECTORY);
        // EA logo
        win.set_texture_alpha(EA_LOGO_DIRECTORY, alpha);
        win.draw_png_center(EA_LOGO_DIRECTORY, 1, 150);
        // Popcap logo
        win.set_texture_alpha(POPCAP_LOGO_DIRECTORY, alpha);
        win.draw_png_center(POPCAP_LOGO_DIRECTORY, 1, 600);
        // Nguyen Minh Hien Edition
        int w = 0, h = 0;
        auto edition = "NGUYEN MINH HIEN Edition";
        TTF_SizeText(win.get_font(HOUSEOFTERROR_TTF, 60), edition, &w, &h);
        win.show_text(edition, 204 + (642 - w) / 2, 490 + (106 - h) / 2, UNTAP, HOUSEOFTERROR_TTF, 60, alpha);
        // Update screen
        HANDLE_SDL_EVENT(
            QUIT(););
        win.update_screen();
    }
    const int FADE_IN_TIME = 30;
    for (int clk = 0; clk <= FADE_IN_TIME; clk++)
    {
        // Starting screen
        win.draw_bg(STARTING_SCREEN_DIRECTORY);
        // Fly in loading bar
        int dy = 490 + (WINDOW_HEIGHT - 490) * (FADE_IN_TIME - clk) / FADE_IN_TIME;
        win.draw_png(LOAD_BAR_DIRT_DIRECTORY, 204, dy, 642, 106);
        // Update screen
        HANDLE_SDL_EVENT(
            QUIT(););
        win.update_screen();
    }
}

/*
Real loading work. The task list is built up front so the total is the real
number of files/assets that will be loaded at boot; every task is one file
(or a warm step weighted by the exact number of files it reads), so progress
advances by true file count, never by a fake curve.
*/
namespace loading {

struct Task
{
    const char *name;
    int files; // real files/units this task completes
    function<void()> run;
};

/*
Decode + upload one game image slot into Window's texture cache. (fade_out
still frees the cache when the starting screen closes, as before; the win is
the one-time decode/validation and a warmed page cache.)
*/
void warm_one_texture(int i)
{
    if (i < COUNT_USED_DIRECTORY)
        win.load_texture(i);
}

void warm_reanim_zombie() { reanimz::init(win.sprite_renderer()); }
void warm_reanim_plants() { reanimplant::init(win.sprite_renderer()); }

/*Decode one reanim part PNG now (page cache + codec warm-up; the sprite
renderer uploads its own GL copy on first in-game use).*/
void warm_one_reanim_png(string path)
{
    SDL_Surface *surf = IMG_Load(path.c_str());
    if (surf != NULL)
        SDL_FreeSurface(surf);
}

/*Parse a level file now instead of at level start (warm-up only: the real
read still happens on level load and will report errors there).*/
void warm_level_json(int lv)
{
    try
    {
        (void)get_lv_json(lv);
    }
    catch (...)
    {
    }
}

void warm_font(string font_addr, int size) { (void)win.get_font(font_addr, size); }

vector<Task> build()
{
    vector<Task> tasks;

    // Game images: COUNT_USED_DIRECTORY is the real length of all_img
    // (static_asserted in image_directory.cpp); one texture slot per task.
    for (int i = 0; i < COUNT_USED_DIRECTORY; i++)
        tasks.push_back({"texture", 1, [i]() { warm_one_texture(i); }});

    // Reanim part sprites: enumerate the real PNG files, one decode per task.
    // Also count the .reanim scripts: reanimz/reanimplant init monolithically
    // parses them, so those two tasks are weighted by that real file count.
    int reanim_scripts = 0;
    {
        vector<string> files;
        try
        {
            for (const auto &entry : filesystem::directory_iterator("./resources/reanim"))
            {
                if (entry.path().extension() == ".png")
                    files.push_back(entry.path().string());
                else if (entry.path().extension() == ".reanim")
                    reanim_scripts++;
            }
        }
        catch (...)
        {
        }
        for (const auto &f : files)
            tasks.push_back({"reanim part", 1, [f]() { warm_one_reanim_png(f); }});
    }
    if (reanim_scripts == 0)
        reanim_scripts = 2; // fallback if the directory could not be listed
    tasks.push_back({"reanim zombie", reanim_scripts / 2 + reanim_scripts % 2, warm_reanim_zombie});
    tasks.push_back({"reanim plants", reanim_scripts / 2, warm_reanim_plants});

    // Level JSON definitions, one file per task.
    for (int lv = 1; lv <= LEVEL_COUNT; lv++)
        tasks.push_back({"level json", 1, [lv]() { warm_level_json(lv); }});

    // Fonts used by the early screens, one file/size per task.
    static const pair<string, int> fonts[] = {
        {BRIANNE_TTF, 40}, {HOUSEOFTERROR_TTF, 60}, {HOUSEOFTERROR_TTF, 40},
        {FREESANS_TTF, 24}, {FREESANS_TTF, 16}, {PVZUI_TTF, 24}, {CONTM_TTF, 24},
    };
    for (const auto &f : fonts)
        tasks.push_back({"font", 1, [f]() { warm_font(f.first, f.second); }});

    // Sound effects (persistent chunk cache) + streamed music files (page
    // cache), one file per task. Index 0..ChooseYourSeeds are the streamed
    // tracks; the rest are effects.
    for (int i = ChooseYourSeeds_DIRECTORY; i < COUNT_MUSIC_DIRECTORY; i++)
        tasks.push_back({"sfx", 1, [i]() { warm_sound_effects(i, i + 1); }});
    for (int i = 0; i <= ChooseYourSeeds_DIRECTORY; i++)
        tasks.push_back({"music", 1, [i]() { prefetch_music_file(i); }});

    return tasks;
}

} // namespace loading

/*
Display loading screen.
The bar is real: total = the actual number of files/assets to load, and each
finished file advances the real progress by exactly its share. Work runs in
~12 ms slices per rendered frame so the screen keeps drawing at display rate.

The DISPLAYED bar tip is decoupled from that real progress: it chases the
real fraction with a speed cap (delta-time based, per-frame step clamped
below the smallest marker gap). Heavy bursts of loaded files therefore never
pop several markers on one frame - the sod roll visibly rolls over each
sprout / the zombie head one after another, each marker then animates to full
on its own wall-clock timeline. The tip never passes the real progress and
only reaches 100% once loading is truly done.
*/
void display_loading_screen()
{
    play_music(OPENING_MUSIC_DIRECTORY);
    display_logo();

    vector<loading::Task> tasks = loading::build();
    int total_files = 0;
    for (const auto &t : tasks)
        total_files += t.files;
    SDL_Log("Loading screen: %d real asset files to load", total_files);

    size_t next_task = 0;
    int done_files = 0;
    double shown_pos = 0.0; // displayed bar length, px over the 628 px track
    int last_logged_mark = -1;
    Uint32 started = SDL_GetTicks();
    Uint32 prev_ticks = started;
    string loading = "LOADING.";

    for (int clk = 1;; clk++)
    {
        // Do a slice of real loading work this frame; keep rendering at
        // display rate instead of blocking on the whole queue.
        Uint32 frame_begin = SDL_GetTicks();
        while (next_task < tasks.size() && SDL_GetTicks() - frame_begin <= LOAD_WORK_BUDGET_TICKS)
        {
            tasks[next_task].run();
            done_files += tasks[next_task].files;
            next_task++;
        }
        bool work_done = next_task >= tasks.size();

        // Move the displayed tip toward the real target, speed-capped and
        // delta-time based; never past the real progress.
        Uint32 now = SDL_GetTicks();
        double dt = (now - prev_ticks) / 1000.0;
        prev_ticks = now;
        if (dt > BAR_TIP_MAX_DT_SEC)
            dt = BAR_TIP_MAX_DT_SEC;
        double real_p = total_files > 0 ? done_files / (double)total_files : 1.0;
        double target_pos = real_p * 628.0;
        if (shown_pos < target_pos)
        {
            shown_pos += BAR_TIP_MAX_PX_PER_SEC * dt;
            if (shown_pos > target_pos)
                shown_pos = target_pos;
        }

        // Done only when the work is done, the minimum screen time passed,
        // and the displayed tip actually rolled to the end (chasing at the
        // capped speed, so the last markers still pop in order).
        bool finished = work_done &&
                        SDL_GetTicks() - started >= MIN_LOADING_TICKS &&
                        shown_pos >= 628.0 - 0.01;
        if (finished)
            shown_pos = 628.0;
        double shown = shown_pos / 628.0;

        // Debug log of the real file progress (bar itself is shown_pos).
        int real_pct = (int)(real_p * 100.0);
        int mark = real_pct / 25;
        if (mark != last_logged_mark)
        {
            last_logged_mark = mark;
            SDL_Log("Loading: %d%% (%d/%d files)", mark * 25, done_files, total_files);
        }

        if (clk % 45 == 15)
            loading = "LOADING..";
        else if (clk % 45 == 30)
            loading = "LOADING...";
        else if (clk % 45 == 0)
            loading = "LOADING.";

        // Loading screen
        win.draw_png_height_scaled(STARTING_SCREEN_DIRECTORY, 0, 0, WINDOW_WIDTH);
        // Loading dirt
        win.draw_png(LOAD_BAR_DIRT_DIRECTORY, 204, 490, 642, 106);
        // Loading text (repvz [LOADING] label - same flat Brianne-tod in the
        // bar center; pulsing ellipsis only - no percentage readout).
        int w = 0, h = 0;
        TTF_SizeText(win.get_font(BRIANNE_TTF, 40), loading.c_str(), &w, &h);
        win.show_text(loading, 204 + (642 - w) / 2, 490 + (106 - h) / 2, UNTAP, BRIANNE_TTF, 40);
        // Loading grass
        int dx = (int)(628 * shown);
        win.draw_png(LOAD_BAR_GRASS_DIRECTORY, 0, 0, dx / 2, 33, 200, 455, dx, 66);
        // Loading roll cap
        int pos = dx + 200;
        int cap_width = 75 + (int)(50 * (1.0 - shown));
        win.draw_png(SOD_ROLL_CAP, pos - cap_width / 2, 515 - cap_width, cap_width, cap_width, 4 * clk);
        // Loading sprouts & zombie head pop the moment the tip rolls past
        // their position; each animates on its own real-time timeline.
        for (auto &sprout : sprouts)
            sprout.display(pos - cap_width / 2);
        zom_head.display(pos);

        HANDLE_SDL_EVENT(
            QUIT(););

        win.update_screen();

        if (finished)
            break;
    }
}
