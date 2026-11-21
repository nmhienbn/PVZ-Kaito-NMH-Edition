#include "rsdl.hpp"
#include "gl/gl_loader.hpp"
#include <iostream>

using namespace std;

/*
Init window
*/
void Window::init()
{
    if (SDL_Init(0) < 0)
        printf("SDL Init Fail\n");
    int flags = (SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    if (SDL_WasInit(flags) != 0)
        printf("SDL_WasInit Failed! Error: %s\n", SDL_GetError());
    if (SDL_InitSubSystem(flags) < 0)
        printf("SDL_InitSubSystem Failed Error: %s\n", SDL_GetError());
    if (!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & IMG_INIT_PNG))
        printf("IMG_Init Fail\n");
    // Reanim .jpg part images need the JPG loader (Frontyard_LV0 is a PNG
    // again since the 2Pak rebuild); warn (don't abort) if this SDL_image
    // build lacks it.
    if (!(IMG_Init(0) & IMG_INIT_JPG))
        printf("IMG_Init: JPG support unavailable\n");
    if (TTF_Init() == -1)
        printf("TTF_Init Fail\n");
}

/*
Create window with an OpenGL 2.1 context; all sprite drawing is batched by
`batch`. GL functions are loaded dynamically (no -lGL link dependency) so a
GL context must exist before pvzgl::init() and batch.init() run.
*/
Window::Window(int width, int height, string title)
{
    init();
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    win = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                           width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (win == NULL)
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    else
    {
        gl_context = SDL_GL_CreateContext(win);
        if (gl_context == NULL)
            printf("GL context could not be created! SDL_Error: %s\n", SDL_GetError());
        else
        {
            SDL_GL_SetSwapInterval(1); // vsync; the frame cap below still applies
            if (!pvzgl::init())
                printf("GL function loading failed! SDL_Error: %s\n", SDL_GetError());
            else if (!batch.init(win, width, height))
                printf("Sprite batch could not be created! SDL_Error: %s\n", SDL_GetError());
        }
    }
    SDL_SetWindowTitle(win, title.c_str());
    fill_rect(0, 0, width, height);
    update_screen();
}

/*
Destroy window
*/
Window::~Window()
{
    for (int i = 0; i < COUNT_USED_DIRECTORY; i++)
        delete_texture(i);
    for (auto &entry : text_cache)
        batch.free(entry.second.tex);
    text_cache.clear();
    // GlSpriteRenderer frees the reanim/PAM textures it uploaded.
    sprite_rend.freeAll();
    batch.shutdown();
    if (gl_context != nullptr)
    {
        SDL_GL_DeleteContext(gl_context);
        gl_context = nullptr;
    }
    SDL_DestroyWindow(win);
    if (TTF_WasInit())
        TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

/*
Show everything on screen: flush the sprite batch, swap buffers, keep the
frame cap, and evict unused cached text textures.

Frame cap: settable per scene via set_frame_cap -- the global FPS (60) by
default for menus and blocking scenes that pace themselves through
update_screen, 0 (uncapped) for the gameplay present in the auto-FPS
fixed-timestep design, where logic ticks are driven by the accumulator in
level.cpp and no longer ride the presentation cadence. When a cap is
active, the pacer holds the presentation cadence at exactly that many
frames/s. The pacer deadline lives in performance-counter units
(SDL_GetPerformanceCounter behaves the same on mingw/Windows); SDL_Delay
granularity varies by platform (~1-15 ms), so it takes the whole-ms part
minus a margin and a short counter spin covers the sub-ms tail.
*/
void Window::update_screen()
{
    batch.present();
    frame_counter++;
    for (auto it = text_cache.begin(); it != text_cache.end();)
    {
        if (frame_counter - it->second.last_used_frame > TEXT_CACHE_EVICT_FRAMES)
        {
            batch.free(it->second.tex);
            it = text_cache.erase(it);
        }
        else
            ++it;
    }
    // -1 (unset) falls back to the global FPS default; 0 skips pacing
    // entirely and presents as fast as the GPU/vsync allows.
    const int cap = frame_cap >= 0 ? frame_cap : FPS;
    if (cap <= 0)
        return;
    const long long freq = (long long)SDL_GetPerformanceFrequency();
    const long long deadline =
        frame_pacer.advance((long long)SDL_GetPerformanceCounter(), freq, cap);
    for (;;)
    {
        const long long remain = deadline - (long long)SDL_GetPerformanceCounter();
        if (remain <= 0)
            break;
        if (remain * 1000 / freq >= 2)
            DELAY((Uint32)(remain * 1000 / freq - 1));
        // <2 ms to go: spin on the performance counter (the tail SDL_Delay
        // cannot sleep precisely).
    }
}

/*
Switch the presentation cap used by update_screen. Setting the value that
is already active is a no-op (the running pacer schedule stays untouched);
an actual change disarms the FramePacer (next_deadline = -1) so its
deadline re-arms at the next advance() call measured from the current
instant, instead of snapping the sleep to a schedule built for the old cap.
*/
void Window::set_frame_cap(int tps)
{
    if (tps == frame_cap)
        return;
    frame_cap = tps;
    frame_pacer.next_deadline = -1; // disarm: next advance re-arms the schedule
}

/*
Clear the frame (uses the color of the last set_color, like SDL_RenderClear
with the current draw color)
*/
void Window::clear_renderer()
{
    batch.clear(clear_r / 255.0f, clear_g / 255.0f, clear_b / 255.0f, 1.0f);
}
