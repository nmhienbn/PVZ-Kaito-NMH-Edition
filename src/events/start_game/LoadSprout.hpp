#pragma once
#include "draw/render_elements.hpp"
#include "music/music.hpp"

/*
One pop-up flower marker on the loading bar. When the displayed bar tip (the
speed-capped one, see display_loading_screen) rolls over x_pos the sprout
pops up through its flipbook, paced by wall clock (SDL_GetTicks) instead of
the loading loop's frame counter, so each marker animates to full on its own
timeline.
*/
class LoadSprout
{
public:
    int x_pos;
    SDL_RendererFlip flip;
    int height, width;
    bool has_raised;
    Uint32 raised_at; // SDL ticks when the bar tip rolled over this sprout

    LoadSprout(int x_pos, SDL_RendererFlip flip, int height, int width);
    void display(int pos);
};
