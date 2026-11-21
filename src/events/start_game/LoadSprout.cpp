#include "LoadSprout.hpp"
#include "draw/rsdl.hpp"
#include <cstdlib>

extern Window win;

// Debug pop logs: only with PVZ_LOADING_DEBUG=1.
static bool loading_debug()
{
    static int enabled = -1;
    if (enabled < 0)
    {
        const char *v = getenv("PVZ_LOADING_DEBUG");
        enabled = (v != NULL && v[0] == '1') ? 1 : 0;
    }
    return enabled == 1;
}

// LoadBar_sprout.png is a 486x40 sheet = 18 frames of 27px (the pop-up bounce
// art from the original game). ~36 ms per frame -> a full pop in ~0.65 s of
// real time, independent per sprout.
const Uint32 SPROUT_FRAME_TICKS = 36;
const int SPROUT_LAST_FRAME = 17;

LoadSprout::LoadSprout(int x_pos, SDL_RendererFlip flip, int height, int width)
{
    this->x_pos = x_pos;
    this->flip = flip;
    this->height = height;
    this->width = width;
    this->has_raised = false;
    this->raised_at = 0;
}

void LoadSprout::display(int pos)
{
    if (!has_raised && pos >= x_pos)
    {
        has_raised = true;
        raised_at = SDL_GetTicks();
        if (loading_debug())
            SDL_Log("[loading-debug] pop sprout x=%d tick=%u bar_pos=%d", x_pos, raised_at, pos);
        play_sound_effect(LOADINGBAR_FLOWER_DIRECTORY);
    }
    if (has_raised)
    {
        int frame = (int)((SDL_GetTicks() - raised_at) / SPROUT_FRAME_TICKS);
        if (frame > SPROUT_LAST_FRAME)
            frame = SPROUT_LAST_FRAME;
        win.draw_png(LOAD_BAR_SPROUT_DIRECTORY, 27 * frame, 0, 27, 40, x_pos, 510 - height, width, height, flip);
    }
}
