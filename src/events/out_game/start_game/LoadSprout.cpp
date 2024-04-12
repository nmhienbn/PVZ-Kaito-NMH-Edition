#include "LoadSprout.hpp"

extern Window win;

LoadSprout::LoadSprout(int x_pos, SDL_RendererFlip flip, int height, int width)
{
    this->x_pos = x_pos;
    this->flip = flip;
    this->clk = 0;
    this->frame = 0;
    this->height = height;
    this->width = width;
    this->has_raised = false;
}

void LoadSprout::display(int pos)
{
    if (pos >= x_pos)
    {
        if (!has_raised)
        {
            has_raised = true;
            play_sound_effect(LOADINGBAR_FLOWER_DIRECTORY);
        }
        win.draw_png(LOAD_BAR_SPROUT_DIRECTORY, 27 * frame, 0, 27, 40, x_pos, 510 - height, width, height, flip);
        clk++;
        if (frame < 17 && (clk % 4 == 0))
            frame++;
    }
}
