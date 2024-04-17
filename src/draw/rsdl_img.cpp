#include "rsdl.hpp"
#include <iostream>
using namespace std;

/*
Set default alpha blender for image in first loaded
*/
void set_default_alpha(int file_num, SDL_Texture *res)
{
    if (all_img[file_num].alpha != 255)
    {
        SDL_SetTextureAlphaMod(res, all_img[file_num].alpha);
    }
}

/*
Print error when loading image
*/
void print_error(SDL_Texture *res, const string &img)
{
    if (res == NULL)
    {
        printf("Cannot load image %s! Error: %s\n", img.c_str(), SDL_GetError());
        exit(-123);
    }
}

/*
Get loaded texture.
If has not loaded, create new texture.
*/
double total_mem = 0;
SDL_Texture *Window::load_texture(int file_num)
{
    SDL_Texture *res = texture_cache[file_num];
    if (res == NULL)
    {
        res = IMG_LoadTexture(renderer, all_img[file_num].img_dir.c_str());
        print_error(res, all_img[file_num].img_dir);
        set_default_alpha(file_num, res);
        texture_cache[file_num] = res;

        // query size of texture
        // int texture_width, texture_height, texture_access;
        // Uint32 texture_format;
        // SDL_QueryTexture(res, &texture_format, &texture_access, &texture_width, &texture_height);
        // int texture_bytes_per_pixel = SDL_BYTESPERPIXEL(texture_format);
        // int texture_size_in_bytes = texture_width * texture_height * texture_bytes_per_pixel;
        // total_mem += texture_size_in_bytes * 1e-6;
        // cout << all_img[file_num].img_dir << ' ' << texture_size_in_bytes * 1e-6 << ' ' << total_mem << '\n';
    }
    return res;
}

/*
Load & draw bmp:
    Load: Surface -> Texture
    Draw: SDL_RenderCopy
*/
void Window::draw_bmp(int file_num, int x, int y, int width, int height)
{
    if (file_num == NULL_DIRECTORY || width <= 0 || height <= 0)
        return;
    SDL_Texture *res = texture_cache[file_num];
    if (res == NULL)
    {
        SDL_Surface *surface = SDL_LoadBMP(all_img[file_num].img_dir.c_str());
        res = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        print_error(res, all_img[file_num].img_dir);
        set_default_alpha(file_num, res);
        texture_cache[file_num] = res;
    }
    SDL_Rect r = {x, y, width, height};
    SDL_RenderCopy(renderer, res, NULL, &r);
}

/*
Load & draw with height scaled by width: height = width * mHeight / mWidth
    Load: Texture
    Draw: SDL_RenderCopy
    mHeight, mWidth: SDL_QueryTexture
*/
void Window::draw_png_height_scaled(int file_num, int x, int y, int width, int angle)
{
    if (file_num == NULL_DIRECTORY || width <= 0)
        return;
    SDL_Texture *res = load_texture(file_num);
    int mWidth = 0, mHeight = 0;
    // get width, height
    SDL_QueryTexture(res, NULL, NULL, &mWidth, &mHeight);
    SDL_Rect r = {x, y, width, width * mHeight / mWidth};
    SDL_RenderCopyEx(renderer, res, NULL, &r, angle, NULL, SDL_FLIP_NONE);
}

/*
Load & draw with width scaled by height: width = height * mWidth / mHeight
    Load: Texture
    Draw: SDL_RenderCopy
    mHeight, mWidth: SDL_QueryTexture
*/
void Window::draw_png_width_scaled(int file_num, int x, int y, int height, int angle)
{
    if (file_num == NULL_DIRECTORY || height <= 0)
        return;
    SDL_Texture *res = load_texture(file_num);
    int mWidth = 0, mHeight = 0;
    // get width, height
    SDL_QueryTexture(res, NULL, NULL, &mWidth, &mHeight);
    SDL_Rect r = {x, y, height * mWidth / mHeight, height};
    SDL_RenderCopyEx(renderer, res, NULL, &r, angle, NULL, SDL_FLIP_NONE);
}

/*
Load & draw in the center of window:
    If x1 = -1, center by x (width)
    If y1 = -1, center by y (height)
    Load: Texture
    Draw: SDL_RenderCopy
    mHeight, mWidth: SDL_QueryTexture
*/
void Window::draw_png_center(int file_num, double ratio, int x1, int y1)
{
    if (file_num == NULL_DIRECTORY)
        return;
    SDL_Texture *res = load_texture(file_num);
    int mWidth = 0, mHeight = 0;
    // get width, height
    SDL_QueryTexture(res, NULL, NULL, &mWidth, &mHeight);
    mWidth = mWidth * ratio;
    mHeight = mHeight * ratio;
    if (x1 == -1)
        x1 = (WINDOW_WIDTH - mWidth) >> 1;
    if (y1 == -1)
        y1 = (WINDOW_HEIGHT - mHeight) >> 1;
    SDL_Rect r = {x1, y1, mWidth, mHeight};
    SDL_RenderCopy(renderer, res, NULL, &r);
}

/*
Load & draw in the SDL_Rect(x, y, width, height)
    Load: Texture
    Draw: SDL_RenderCopy
*/
void Window::draw_png(int file_num, int x, int y, int width, int height)
{
    if (file_num == NULL_DIRECTORY || width <= 0 || height <= 0)
        return;
    SDL_Texture *res = load_texture(file_num);
    SDL_Rect r = {x, y, width, height};
    SDL_RenderCopy(renderer, res, NULL, &r);
}

/*
Load & draw in the SDL_Rect(x, y, width, height) and rotation of angle
    Load: Texture
    Draw: SDL_RenderCopyEx
*/
void Window::draw_png(int file_num, int x, int y, int width, int height, int angle, SDL_RendererFlip flip)
{
    if (file_num == NULL_DIRECTORY || width <= 0 || height <= 0)
        return;
    SDL_Texture *res = load_texture(file_num);
    SDL_Rect r = {x, y, width, height};
    SDL_RenderCopyEx(renderer, res, NULL, &r, angle, NULL, flip);
}

/*
Load & draw with scale: from source SDL_Rect(sx, sy, sw, sh) to destination SDL_Rect(dx, dy, dw, dh)
    Load: Texture
    Draw: SDL_RenderCopy
*/
void Window::draw_png(int file_num, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, SDL_RendererFlip flip)
{
    if (file_num == NULL_DIRECTORY || sw <= 0 || sh <= 0 || dw <= 0 || dh <= 0)
        return;
    SDL_Texture *res = load_texture(file_num);
    SDL_Rect src = {sx, sy, sw, sh};
    SDL_Rect dst = {dx, dy, dw, sh * dw / sw};
    SDL_RenderCopyEx(renderer, res, &src, &dst, 0, NULL, flip);
}

/*
Load & draw image to all window:
    Load: Texture
    Draw: SDL_RenderCopy
*/
void Window::draw_bg(int file_num, int x, int y)
{
    if (file_num == NULL_DIRECTORY)
        return;
    SDL_Texture *res = load_texture(file_num);
    SDL_Rect src = {x, y, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_Rect dst = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderCopy(renderer, res, &src, &dst);
}

/*
Set texture alpha mod
*/
void Window::set_texture_alpha(int file_num, int a)
{
    if (file_num != NULL_DIRECTORY && texture_cache[file_num] != NULL)
    {
        SDL_SetTextureAlphaMod(texture_cache[file_num], a);
    }
}

/*
Set texture RGB mod
*/
void Window::set_texture_color(int file_num, int r, int g, int b)
{
    if (file_num != NULL_DIRECTORY && texture_cache[file_num] != NULL)
    {
        SDL_SetTextureColorMod(texture_cache[file_num], r, g, b);
    }
}

extern bool quit;
/*
Fade out effect
*/
void Window::fade_out()
{
    for (int i = 0; i <= 64; i++)
    {
        draw_png(BLACK_SCREEN_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        set_texture_alpha(BLACK_SCREEN_DIRECTORY, (i));
        update_screen();
        HANDLE_SDL_EVENT(QUIT())
    }
    set_texture_alpha(BLACK_SCREEN_DIRECTORY, 150);
    clear_renderer();
    delete_all_texture();
}

/*
Delete a game texture
*/
void Window::delete_texture(const int &file_num)
{
    if (texture_cache[file_num] != NULL)
    {
        SDL_DestroyTexture(texture_cache[file_num]);
        texture_cache[file_num] = NULL;
    }
}

/*
Delete all game textures except the mouse cursor
*/
void Window::delete_all_texture()
{
    for (int i = 0; i < COUNT_USED_DIRECTORY; i++)
        if (i != MOUSE_CURSOR_DIRECTORY)
        {
            delete_texture(i);
        }
}