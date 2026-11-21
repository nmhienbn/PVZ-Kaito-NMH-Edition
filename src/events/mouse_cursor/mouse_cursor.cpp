#include "mouse_cursor.hpp"

// Whack mallet cursor frames (idle + swing). The mallet renders at 2x its
// source art (136x142, upscaled from the 68x71 extraction) so the head is
// about as wide as the original's whack hit circle (radius 45 at the mouse,
// repvz Challenge.cpp MouseDownWhackAZombie) -- the original draws the
// hammer as a full-size in-game reanim (REANIM_HAMMER, ~140 px tall at
// retail scale), not a small OS cursor. The shared hotspot is the mallet
// head's impact point, so a click lands where the head visually strikes
// (the extraction pinned it at 12,60 on the 68x71 art; 2x keeps it there).
static const char *HAMMER_CURSOR_PATHS[4] = {
    "./resources/images/Cursor/hammer_idle.png",
    "./resources/images/Cursor/hammer_hit1.png", // impact
    "./resources/images/Cursor/hammer_hit2.png", // rising
    "./resources/images/Cursor/hammer_hit3.png"  // almost raised
};
static const int HAMMER_HOTSPOT_X = 24, HAMMER_HOTSPOT_Y = 120;

// Swing playback: impact (2 ticks) -> two recovery frames -> idle; ~100 ms
// at 60 tps, matching the original's reanim-rate swing snapback.
static const int SWING_SEQUENCE[] = {1, 1, 2, 2, 3, 3};
static const int SWING_LEN = sizeof(SWING_SEQUENCE) / sizeof(SWING_SEQUENCE[0]);

static SDL_Cursor *load_cursor(const string &path, const int &hot_x, const int &hot_y)
{
    SDL_Surface *tmp = IMG_Load(path.c_str());
    if (tmp == NULL)
        return NULL;
    SDL_Cursor *cursor = SDL_CreateColorCursor(tmp, hot_x, hot_y);
    SDL_FreeSurface(tmp);
    return cursor;
}

/*
Load game mouse cursor
*/
Mouse::Mouse()
{
    hammer_frame = -1;
    hammer_active = false;
    Cursor = load_cursor(all_img[MOUSE_CURSOR_DIRECTORY].img_dir, 0, 0);
    // The mallet set is optional: without the art the whack level simply
    // keeps the normal cursor.
    bool have_all = true;
    for (int i = 0; i < 4; i++)
    {
        hammer_cursor[i] = load_cursor(HAMMER_CURSOR_PATHS[i],
                                       HAMMER_HOTSPOT_X, HAMMER_HOTSPOT_Y);
        if (hammer_cursor[i] == NULL)
            have_all = false;
    }
    if (!have_all)
        for (int i = 0; i < 4; i++)
        {
            SDL_FreeCursor(hammer_cursor[i]); // safe on NULL
            hammer_cursor[i] = NULL;
        }
}

/*
Free game mouse cursor
*/
Mouse::~Mouse()
{
    SDL_FreeCursor(Cursor);
    for (int i = 0; i < 4; i++)
        SDL_FreeCursor(hammer_cursor[i]);
    Cursor = NULL;
}

/*Push the cursor matching the current state onto SDL.*/
void Mouse::apply()
{
    if (hammer_active && hammer_cursor[0] != NULL)
        SDL_SetCursor(hammer_cursor[hammer_frame < 0 ? 0 : SWING_SEQUENCE[hammer_frame]]);
    else
        SDL_SetCursor(Cursor);
}

/*
Set game mouse cursor
*/
void Mouse::set_cursor()
{
    hammer_active = false;
    hammer_frame = -1;
    apply();
}

void Mouse::update(const bool &whack_level, const int &advance_ticks)
{
    const bool want_hammer = whack_level && hammer_cursor[0] != NULL;
    if (want_hammer != hammer_active)
    {
        hammer_active = want_hammer;
        hammer_frame = -1;
        apply();
    }
    // One swing step per logic tick (auto-FPS); once hammer_frame hits -1
    // (idle) the loop condition stops any further steps this frame.
    for (int _lt = advance_ticks;
         _lt > 0 && hammer_active && hammer_frame >= 0; _lt--)
    {
        if (++hammer_frame >= SWING_LEN)
            hammer_frame = -1;
        apply();
    }
}

void Mouse::start_swing()
{
    if (!hammer_active)
        return;
    hammer_frame = 0;
    apply();
}

Mouse &mouse_cursor()
{
    static Mouse inst;
    return inst;
}
