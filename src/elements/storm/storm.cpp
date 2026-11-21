#include "storm.hpp"
#include "draw/rsdl.hpp"
#include "draw/render_elements.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Player/Player.hpp"
#include "elements/button/button.hpp"
#include "elements/conveyor/conveyor.hpp"
#include "elements/elements.hpp"
#include "elements/progress_bar/progress_bar.hpp"
#include "elements/sun/sun.hpp"
#include "music/music.hpp"
#include "support.hpp"
#include <algorithm>

extern Level level;
extern Player player;
extern Elements game_characters;
extern Window win;

namespace storm {

namespace {

// repvz (Challenge::UpdateStormyNight / DrawStormNight) ticks at 100 fps; this
// clone runs 60 fps, so every original count is scaled by 0.6.

// Darkness alpha ramps (255 - max) -> 255 while a flash's fade time counts
// down from FLASH_FADE_TICKS to 0 (repvz STORM_FLASH_TIME = 150).
const int FLASH_FADE_TICKS = 90;
// Flash styles (repvz STATECHALLENGE_STORM_FLASH_1/2/3):
//   1 = bright strike fading out, then a dimmer second pulse (300 ticks);
//   2 = one long fade at half clock (300 ticks);
//   3 = one short fade (150 ticks).
const int FLASH_LONG_TICKS = 180;
const int FLASH_SHORT_TICKS = 90;
// The dim second pulse of style 1 stays far-from-black (repvz max 92).
const int FLASH_DIM_MAX = 92;
// Darkness between flashes: repvz holds 1.5-6 s (150 + Rand(300, 400|750) at
// 100 fps) -> 90..360 ticks here. The level opens with a short 1 s of dark
// then the big flash (repvz StartLevel: FLASH_1, 100-tick lead-in).
const int STORM_START_DARK_TICKS = 60;
const int DARK_MIN_TICKS = 90;
const int DARK_SPAN_TICKS = 270;
// Darkness flickers +/-32 around the ramp, re-rolled every ~4 ticks (repvz
// re-rolls every 6 ticks at 100 fps). Stateless hash: drawing must never
// consume the gameplay rand() stream.
const int FLICKER_GROUP_TICKS = 4;
const int FLICKER_SPAN = 32;
// rain.ogg is 4.34 s; re-arm the loop a touch early so it never gaps.
const int RAIN_LOOP_CLK = 250;

int g_phase_ticks = 0; // countdown: dark ticks, or ticks left in the flash
int g_flash_style = 0; // 0 = dark; 1/2/3 = repvz FLASH_1/2/3
int g_anim_ticks = 0;  // storm-local clock driving the flicker
int g_rain_clk = 0;
int g_storm_level = -1; // current level instance the clock belongs to

int flash_window(int style)
{
    return style == 3 ? FLASH_SHORT_TICKS : FLASH_LONG_TICKS;
}

int flicker()
{
    unsigned h = (unsigned)(g_anim_ticks / FLICKER_GROUP_TICKS) * 2654435761u;
    return (h >> 13) % (2 * FLICKER_SPAN + 1) - FLICKER_SPAN;
}

// repvz DrawStormNight state mapping: pure view of the current tick.
int darkness_alpha()
{
    if (g_flash_style == 0)
        return 255;
    int time, max;
    if (g_flash_style == 1 && g_phase_ticks <= FLASH_SHORT_TICKS)
    {
        time = g_phase_ticks;
        max = FLASH_DIM_MAX;
    }
    else if (g_flash_style == 1)
    {
        time = g_phase_ticks - FLASH_SHORT_TICKS;
        max = 255;
    }
    else if (g_flash_style == 2)
    {
        time = g_phase_ticks / 2;
        max = 255;
    }
    else
    {
        time = g_phase_ticks;
        max = 255;
    }
    int base = 255 - max * time / FLASH_FADE_TICKS;
    return std::clamp(base + flicker(), 0, 255);
}

void reset_clock()
{
    g_flash_style = 0;
    g_phase_ticks = STORM_START_DARK_TICKS;
    g_anim_ticks = 0;
    g_rain_clk = 0;
    g_storm_level = level.level_num;
}

// repvz redraws the board UI (DrawUIBottom / DrawTopRightUI; coins sit on a
// render layer above the storm) after the darkness fill, so packets, shovel,
// suns and counters stay readable while the lawn is pitch black.
void redraw_ui_on_top()
{
    display_suns(game_characters.suns);
    if (!belt.active)
    {
        win.draw_png_height_scaled(SUN_BAR_DIRECTORY, 5, 5, 180);
        win.set_style(PVZUI_TTF, 26, TTF_STYLE_BOLD);
        win.show_text(std::to_string(player.sun_count), 100, 17,
                      (player.sun_count_change_color_times & 1 ? RED : BLACK),
                      PVZUI_TTF, 30);
    }
    if (level.has_shovel)
    {
        Shovel_bar.display(SHOVEL_BAR_DIRECTORY);
        Shovel_bar.blink();
        win.show_text_outlined("S", Shovel_bar.x2 - 13, Shovel_bar.y1, WHITE, BRIANNE_TTF);
    }
    display_seed_packets_bar();
    PAUSE_ICON.display(PAUSE_ICON_DIRECTORY);
    TURBO_ICON.display(TURBO_ICON_DIRECTORY);
    // Storm redraw double-call: the storm sheet covers the first progress
    // bar draw, so the bar is redrawn here on top -- a SECOND display call
    // in the same frame. advance = false keeps the auto-FPS logic-tick gate
    // from advancing the chase/raise state twice per frame (2x speed).
    display_progress_bar(false);
}

} // namespace

void update()
{
    if (!level.is_special(LevelSpecial::Storm))
        return;
    if (g_storm_level != level.level_num)
        reset_clock();
    ++g_anim_ticks;
    if (--g_rain_clk <= 0)
    {
        play_sound_effect(RAIN_MUSIC_DIRECTORY);
        g_rain_clk = RAIN_LOOP_CLK;
    }
    if (g_flash_style == 0)
    {
        if (--g_phase_ticks <= 0)
        {
            g_flash_style = rand(1, 3);
            g_phase_ticks = flash_window(g_flash_style);
            play_sound_effect(THUNDER_MUSIC_DIRECTORY);
        }
        return;
    }
    if (--g_phase_ticks <= 0)
    {
        g_flash_style = 0;
        g_phase_ticks = DARK_MIN_TICKS + rand(0, DARK_SPAN_TICKS);
    }
}

// Darkness alpha drawn over the lawn right now: 255 = pitch black, ~0 = the
// lawn is fully lit by lightning. Pure (no RNG side effects) so tests and the
// render pass can both consult it.
int overlay_alpha()
{
    if (!level.is_special(LevelSpecial::Storm))
        return 0;
    return darkness_alpha();
}

void draw_overlay()
{
    if (!level.is_special(LevelSpecial::Storm))
        return;
    int alpha = darkness_alpha();
    win.set_texture_alpha(BLACK_SCREEN_DIRECTORY, alpha);
    win.draw_bg(BLACK_SCREEN_DIRECTORY);
    win.set_texture_alpha(BLACK_SCREEN_DIRECTORY, 255);
    redraw_ui_on_top();
}

} // namespace storm
