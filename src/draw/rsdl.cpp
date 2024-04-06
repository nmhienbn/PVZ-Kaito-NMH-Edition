#include "rsdl.hpp"
#include <iostream>

using namespace std;

/*
Init window
*/
void window::init()
{
    if (SDL_Init(0) < 0)
        printf("SDL Init Fail\n");
    int flags = (SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    if (SDL_WasInit(flags) != 0)
        printf("SDL_WasInit Failed! Error: %s\n", SDL_GetError());
    if (SDL_InitSubSystem(flags) < 0)
        printf("SDL_InitSubSystem Failed Error: %s\n", SDL_GetError());
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
        printf("IMG_Init Fail\n");
    if (TTF_Init() == -1)
        printf("TTF_Init Fail\n");
}

/*
Create window and renderer with width, height and title
*/
window::window(int width, int height, string title)
{
    init();
    win = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (win == NULL)
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL)
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    SDL_SetWindowTitle(win, title.c_str());
    // set_color(BLACK);
    fill_rect(0, 0, width, height);
    update_screen();
}

/*
Destroy window
*/
window::~window()
{
    SDL_DestroyWindow(win);
    if (TTF_WasInit())
        TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

/*
Show everything on renderer to screen.
*/
void window::update_screen()
{
    SDL_RenderPresent(renderer);
    int now_time = SDL_GetTicks() - previous_time;
    int delay_time = std::max(5, TICKS_PER_FRAME - now_time);
    DELAY(delay_time);
    previous_time = SDL_GetTicks();
}

/*
Clear renderer
*/
void window::clear_renderer()
{
    SDL_RenderClear(renderer);
}
