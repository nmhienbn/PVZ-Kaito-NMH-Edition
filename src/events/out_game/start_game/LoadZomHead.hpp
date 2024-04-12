#pragma once
#include "draw/render_elements.hpp"
#include "music/music.hpp"

class LoadZomHead
{
public:
    int x_pos;
    SDL_RendererFlip flip;
    int clk, frame;
    int height, width;
    bool has_raised;

    LoadZomHead(int x_pos, SDL_RendererFlip flip, int height, int width);
    void display(int pos);
};