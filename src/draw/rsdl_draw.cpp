#include "rsdl.hpp"

/*
SDL set color
*/
void window::set_color(RGB color)
{
    SDL_SetRenderDrawColor(renderer, color.red, color.green, color.blue, 255);
}

/*
SDL draw filled rectangle
*/
void window::fill_rect(int x, int y, int width, int height, RGB color)
{
    set_color(color);
    SDL_Rect r;
    r.x = x;
    r.y = y;
    r.w = width;
    r.h = height;
    SDL_RenderFillRect(renderer, &r);
}

/*
SDL draw line
*/
void window::draw_line(int x1, int y1, int x2, int y2, RGB color)
{
    set_color(color);
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

/*
SDL draw point
*/
void window::draw_point(int x, int y, RGB color)
{
    set_color(color);
    SDL_RenderDrawPoint(renderer, x, y);
}

/*
SDL draw unfilled rectangle
*/
void window::draw_rect(int x, int y, int width, int height, RGB color)
{
    draw_line(x, y, x + width, y, color);
    draw_line(x, y, x, y + height, color);
    draw_line(x, y + height, x + width, y + height, color);
    draw_line(x + width, y, x + width, y + height, color);
}