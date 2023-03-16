#include <iostream>
#include <sstream>
#include "rsdl.hpp"

using namespace std;

void window::init()
{
    if (SDL_Init(0) < 0)
        throw "SDL Init Fail";
    int flags = (SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    if (SDL_WasInit(flags) != 0)
        throw string("SDL_WasInit Failed ") + SDL_GetError();
    if (SDL_InitSubSystem(flags) < 0)
        throw string("SDL_InitSubSystem Failed ") + SDL_GetError();
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
        throw "IMG_Init Fail";
    if (TTF_Init() == -1)
        throw "TTF_Init Fail";
}

window::window(int width, int height, string title)
{
    init();
    SDL_CreateWindowAndRenderer(width, height, 0, &win, &renderer);
    if (win == NULL || renderer == NULL)
        throw string("Window could not be created! SDL_Error: ") + SDL_GetError();
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    SDL_SetWindowTitle(win, title.c_str());
    // set_color(BLACK);
    fill_rect(0, 0, width, height);
    update_screen();
}

window::~window()
{
    SDL_DestroyWindow(win);
    if (TTF_WasInit())
        TTF_Quit();
    SDL_Quit();
}

void window::show_text(string input, int x, int y, RGB color, string font_addr, int size)
{
    SDL_Color textColor = {(Uint8)color.red, (Uint8)color.green, (Uint8)color.blue, 0};
    stringstream ss;
    ss << size;
    TTF_Font *font = fonts_cache[font_addr + ":" + ss.str()];
    if (font == NULL)
    {
        font = TTF_OpenFont(font_addr.c_str(), size);
        fonts_cache[font_addr + ":" + ss.str()] = font;
    }
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, input.c_str(), textColor);
    SDL_Texture *text = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
    SDL_Rect renderQuad = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, text, NULL, &renderQuad);
    SDL_DestroyTexture(text);
}

void window::set_color(RGB color)
{
    SDL_SetRenderDrawColor(renderer, color.red, color.green, color.blue, 255);
}

void window::draw_bmp(int file_num, int x, int y, int width, int height)
{
    SDL_Texture *res = texture_cache[file_num];
    if (res == NULL)
    {
        SDL_Surface *surface = SDL_LoadBMP(image_directory[file_num].c_str());
        res = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        texture_cache[file_num] = res;
    }
    SDL_Rect r = {x, y, width, height};
    SDL_RenderCopy(renderer, res, NULL, &r);
}

void window::draw_png_scale(int file_num, int x, int y, int width, int height)
{
    SDL_Texture *res = texture_cache[file_num];
    int mWidth = 0, mHeight = 0;
    if (res == NULL)
    {
        res = IMG_LoadTexture(renderer, image_directory[file_num].c_str());
        texture_cache[file_num] = res;
    }
    SDL_QueryTexture(res, NULL, NULL, &mWidth, &mHeight);
    SDL_Rect r = {x, y, width, width * mHeight / mWidth};
    SDL_RenderCopy(renderer, res, NULL, &r);
}

void window::draw_png(int file_num, int x, int y, int width, int height)
{
    SDL_Texture *res = texture_cache[file_num];
    if (res == NULL)
    {
        res = IMG_LoadTexture(renderer, image_directory[file_num].c_str());
        texture_cache[file_num] = res;
    }
    SDL_Rect r = {x, y, width, height};
    SDL_RenderCopy(renderer, res, NULL, &r);
}

void window::draw_png(int file_num, int x, int y, int width, int height, int angle)
{
    SDL_Texture *res = texture_cache[file_num];
    if (res == NULL)
    {
        res = IMG_LoadTexture(renderer, image_directory[file_num].c_str());
        texture_cache[file_num] = res;
    }
    SDL_Rect r = {x, y, width, height};
    SDL_RenderCopyEx(renderer, res, NULL, &r, angle, NULL, SDL_FLIP_NONE);
}

void window::draw_png(int file_num, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh)
{
    SDL_Texture *res = texture_cache[file_num];
    if (res == NULL)
    {
        res = IMG_LoadTexture(renderer, image_directory[file_num].c_str());
        // Color key image
        // SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));
        if (file_num == ZOMBIE_BLINK_SHEET_DIRECTORY || file_num == SUNFLOWER_SHEET_BLINK_DIRECTORY)
        {
            SDL_SetTextureAlphaMod(res, 100);
        }
        texture_cache[file_num] = res;
    }
    SDL_Rect src = {sx, sy, sw, sh};
    SDL_Rect dst = {dx, dy, dw, sh * dw / sw};
    SDL_RenderCopy(renderer, res, &src, &dst);
}

void window::draw_bg(int file_num, int x, int y)
{
    SDL_Texture *res = texture_cache[file_num];
    if (res == NULL)
    {
        // SDL_Surface *loadedSurface = IMG_Load(image_directory[file_num].c_str());
        // if (loadedSurface == NULL)
        // {
        //     printf("Unable to load image %s! SDL_image Error: %s\n", image_directory[file_num].c_str(), IMG_GetError());
        // }
        // else
        // {
        //     // Color key image
        //     SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

        //     // Create texture from surface pixels
        //     res = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        //     if (res == NULL)
        //     {
        //         printf("Unable to create texture from %s! SDL Error: %s\n", image_directory[file_num].c_str(), SDL_GetError());
        //     }
        // }
        res = IMG_LoadTexture(renderer, image_directory[file_num].c_str());
        if (file_num == BLACK_SCREEN_DIRECTORY)
        {
            SDL_SetTextureAlphaMod(res, 150);
        }
        texture_cache[file_num] = res;
    }
    SDL_Rect src = {x, y, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_Rect dst = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderCopy(renderer, res, &src, &dst);
}

void window::update_screen()
{
    SDL_RenderPresent(renderer);
}

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

void window::draw_line(int x1, int y1, int x2, int y2, RGB color)
{
    set_color(color);
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

void window::draw_point(int x, int y, RGB color)
{
    set_color(color);
    SDL_RenderDrawPoint(renderer, x, y);
}

void window::draw_rect(int x, int y, int width, int height, RGB color)
{
    draw_line(x, y, x + width, y, color);
    draw_line(x, y, x, y + height, color);
    draw_line(x, y + height, x + width, y + height, color);
    draw_line(x + width, y, x + width, y + height, color);
}
