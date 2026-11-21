#include "LoadZomHead.hpp"
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

// LoadBar_zombiehead.png is a 936x61 sheet = 18 frames of 52px.
const Uint32 ZOMHEAD_FRAME_TICKS = 36;
const int ZOMHEAD_LAST_FRAME = 17;

LoadZomHead::LoadZomHead(int x_pos, SDL_RendererFlip flip, int height, int width)
{
    this->x_pos = x_pos;
    this->flip = flip;
    this->height = height;
    this->width = width;
    this->has_raised = false;
    this->raised_at = 0;
}

void LoadZomHead::display(int pos)
{
    if (!has_raised && pos >= x_pos)
    {
        has_raised = true;
        raised_at = SDL_GetTicks();
        if (loading_debug())
            SDL_Log("[loading-debug] pop zomhead x=%d tick=%u bar_pos=%d", x_pos, raised_at, pos);
        play_sound_effect(LOADINGBAR_ZOMBIE_DIRECTORY);
    }
    if (has_raised)
    {
        int frame = (int)((SDL_GetTicks() - raised_at) / ZOMHEAD_FRAME_TICKS);
        if (frame > ZOMHEAD_LAST_FRAME)
            frame = ZOMHEAD_LAST_FRAME;
        win.draw_png(LOAD_BAR_ZOMBIEHEAD_DIRECTORY, 52 * frame, 0, 52, 61, x_pos, 510 - height, width, height);
    }
}
