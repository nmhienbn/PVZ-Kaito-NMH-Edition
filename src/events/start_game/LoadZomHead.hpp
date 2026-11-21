#pragma once
#include "draw/render_elements.hpp"
#include "music/music.hpp"

/*
The zombie-head marker at the end of the loading bar. Same real-time pop
model as LoadSprout: triggered the moment the displayed bar tip (the
speed-capped one) rolls over x_pos, flipbook paced by SDL_GetTicks so it
plays out smoothly on its own timeline.
*/
class LoadZomHead
{
public:
    int x_pos;
    SDL_RendererFlip flip;
    int height, width;
    bool has_raised;
    Uint32 raised_at; // SDL ticks when the bar tip rolled over the head

    LoadZomHead(int x_pos, SDL_RendererFlip flip, int height, int width);
    void display(int pos);
};
