#include "win.hpp"
#include "elements/Level/Level.hpp"
#include "elements/elements.hpp"

extern int game_state;
extern bool quit;
extern Level level;
extern Elements game_characters;
extern Window win;

/*On-lawn win presentation (repvz: the last zombie's death drops the level
award where it died and the board stays on screen under the prize --
Zombie::TrySpawnLevelAward, .tmp_research/repvz/Zombie.cpp:7194-7230; the win
jingle FOLEY_WINMUSIC replaces the music while the board fades to black --
Board::FadeOutLevel, .tmp_research/repvz/Board.cpp:1903-1990). The clone has
no award-coin objects yet, so the prize drop itself is skipped: the sparkle
phase plays over the frozen lawn, then the flow hands off to the award screen
(if one is pending) and on to the selector/main menu.*/

// Effect length at 60 fps: ~2.5 s of sparkle, then the shared black fade.
static const int WIN_FX_CLKS = 150;

// Glow/star pendant of the dropped award, parked at lawn center (the dead
// zombie's position is gone by the time has_player_won fires).
static const int WIN_FX_CX = WINDOW_WIDTH / 2;
static const int WIN_FX_CY = 300;

struct Twinkle
{
    int dx, dy, phase;
};

// Deterministic star offsets around the glow (stable for framebuffer probes).
static const Twinkle WIN_TWINKLES[] = {{-170, -90, 0},  {150, -110, 18}, {190, 40, 36},
                                       {-190, 70, 12},  {60, -150, 44},  {-40, 150, 28}};

static void draw_win_sparkle(int f)
{
    // Radial glow: radius eases in over the first second, then breathes.
    float t = f < 60 ? f / 60.0f : 1.0f;
    t = 1.0f - (1.0f - t) * (1.0f - t); // ease-out quad
    int r = 90 + (int)(100 * t);
    int pulse = ((f / 8) & 1) ? 30 : 0;
    win.set_texture_alpha(COIN_GLOW_DIRECTORY, 140 + pulse);
    win.draw_png(COIN_GLOW_DIRECTORY, WIN_FX_CX - r, WIN_FX_CY - r, 2 * r, 2 * r);
    win.set_texture_alpha(COIN_GLOW_DIRECTORY, all_img[COIN_GLOW_DIRECTORY].alpha);

    // Star twinkles: triangle-wave alpha over a 48-frame cycle each.
    for (const Twinkle &tw : WIN_TWINKLES)
    {
        int cyc = (f + tw.phase) % 48;
        if (cyc >= 24)
            continue;
        int a = cyc < 12 ? cyc * 21 : 255 - (cyc - 12) * 21;
        int s = 22 + a / 24;
        win.set_texture_alpha(DIAMOND_DIRECTORY, a);
        win.draw_png(DIAMOND_DIRECTORY, WIN_FX_CX + tw.dx - s / 2,
                     WIN_FX_CY + tw.dy - s / 2, s, s);
    }
    win.set_texture_alpha(DIAMOND_DIRECTORY, all_img[DIAMOND_DIRECTORY].alpha);

    // White pop as the jingle starts, decaying over 40 frames (the retail
    // award screen fades in from white, AwardScreen::Draw
    // .tmp_research/repvz/AwardScreen.cpp:407-409).
    if (f < 40)
    {
        win.set_texture_alpha(WHITE_SCREEN_DIRECTORY, 200 * (40 - f) / 40);
        win.draw_png(WHITE_SCREEN_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        win.set_texture_alpha(WHITE_SCREEN_DIRECTORY, all_img[WHITE_SCREEN_DIRECTORY].alpha);
    }
}

/*
One frozen-lawn frame of the win effect (f = effect clock). Kept separate
from the blocking loop so the headless win/award check can probe pixels.
*/
void draw_win_frame(int f)
{
    // Blocking win-effect loop: the gameplay frame driver (level.cpp) is not
    // running, so the auto-FPS gate would hold a stale value -- force it on
    // so lawn anims keep animating per frame like the old 60 fps loop did.
    logic_ticked = 1;
    win.clear_renderer();
    display_game_layout(); // full scene, simulation frozen while we block
    draw_win_sparkle(f);
}

/*
Check if player won: no wave or zombie left.
*/
bool has_player_won()
{
    if (level.waves_finished && game_characters.zombies.empty() && game_characters.zombie_parts.empty())
        return true;
    return false;
}

/*
Handle when player win the game:
    Win jingle + sparkle on the lawn (no separate backdrop).
    Bookkeep the cleared level, hand off to the pending award screen if any,
    and land the player back on the selector/main menu.
*/
bool display_win()
{
    play_music(WIN_MUSIC_DIRECTORY, 0); // FOLEY_WINMUSIC, repvz Board.cpp:1922-1933
    for (int f = 0; f < WIN_FX_CLKS; f++)
    {
        draw_win_frame(f);
        win.update_screen();
        HANDLE_SDL_EVENT(QUIT(););
    }
    // Level-complete bookkeeping (used to sit behind the win screen's
    // Continue click).
    update_unlocked_level();
    level.waves_finished = false;
    set_status(game_state, IS_LEVEL_CHOSEN, false);
    set_status(game_state, IS_PAUSED, false);
    // level.cpp flags the pending award via IS_UNLOCKING_PLANT next; either
    // way the main loop then lands on the selector screen.
    set_status(game_state, IS_SELECTOR_SCREEN, true);
    win.fade_out();
    return true;
}
