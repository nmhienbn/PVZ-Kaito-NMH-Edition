#include "LoadZomHead.hpp"

extern Window win;

LoadZomHead::LoadZomHead(int x_pos, SDL_RendererFlip flip, int height, int width)
{
    this->x_pos = x_pos;
    this->flip = flip;
    this->height = height;
    this->width = width;
    this->clk = 0;
    this->frame = 0;
    this->has_raised = false;
}

void LoadZomHead::display(int pos)
{
    if (pos >= x_pos && !has_raised)
    {
        has_raised = true;
        play_sound_effect(LOADINGBAR_ZOMBIE_DIRECTORY);
    }
    if (has_raised)
    {
        win.draw_png(LOAD_BAR_ZOMBIEHEAD_DIRECTORY, 52 * frame, 0, 52, 61, x_pos, 510 - height, width, height);
        clk++;
        if (frame < 17 && (clk % 4 == 0))
            frame++;
    }
}