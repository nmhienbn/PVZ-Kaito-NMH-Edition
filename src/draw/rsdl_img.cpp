#include "rsdl.hpp"
#include <iostream>
using namespace std;

int last_render[COUNT_USED_DIRECTORY];

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
Print error when loading image
*/
void print_error(SDL_Surface *res, const string &img)
{
    if (res == NULL)
    {
        printf("Cannot load image %s! Error: %s\n", img.c_str(), SDL_GetError());
        exit(-123);
    }
}

SDL_Texture *Window::load_texture_with_blink(int file_num)
{
    string img_dir = all_img[file_num].img_dir;
    SDL_Surface *loadedSurface = IMG_Load(img_dir.c_str());
    print_error(loadedSurface, img_dir);
    int height = loadedSurface->h;

    Uint32 *pixels = (Uint32 *)loadedSurface->pixels;
    int pitch = loadedSurface->pitch;
    int pixel_count = pitch / sizeof(Uint32) * height;

    Uint32 white_color = SDL_MapRGBA(loadedSurface->format, 0xff, 0xff, 0xff, 0xff);
    Uint32 transparent = SDL_MapRGBA(loadedSurface->format, 0x00, 0x00, 0x00, 0x00);

    for (int i = 0; i < pixel_count; ++i)

    {
        Uint32 pixel = pixels[i];
        Uint8 r, g, b, a;
        SDL_GetRGBA(pixel, loadedSurface->format, &r, &g, &b, &a);
        pixels[i] = (a > 127 ? white_color : transparent);
    }

    SDL_Texture *blink = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);
    texture_cache[blink_of(file_num)] = blink;
    return blink;
}

/*
Get loaded texture.
If has not loaded, create new texture.
*/
double total_mem = 0;
SDL_Texture *Window::load_texture(int file_num)
{
    SDL_Texture *res = texture_cache[file_num];
    last_render[file_num] = 0;
    if (res == NULL)
    {
        int blink = blink_ori(file_num);
        if (blink != NULL_DIRECTORY)
        {
            res = load_texture_with_blink(blink);
            set_default_alpha(file_num, res);
            texture_cache[file_num] = res;
        }
        else
        {
            res = IMG_LoadTexture(renderer, all_img[file_num].img_dir.c_str());
            print_error(res, all_img[file_num].img_dir);
            set_default_alpha(file_num, res);
            texture_cache[file_num] = res;
        }

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

void Window::draw_nth_frame(int file_num, int sframe, int ori_w, int ori_h,
                            int game_x, int game_y, int game_w, int game_h, int angle)
{
    if (file_num == NULL_DIRECTORY || game_w <= 0 || game_h <= 0 || ori_w <= 0 || ori_h <= 0)
        return;
    if (all_img[file_num].c_sheet == 0)
    {
        cout << "NOT A SHEET IMAGE\n";
        exit(-1234567);
    }
    // current col in source image
    int scol = sframe % all_img[file_num].c_sheet;
    // current row in source image
    int srow = sframe / all_img[file_num].c_sheet;

    draw_png(file_num, ori_w * scol, ori_h * srow, ori_w, ori_h,
             game_x, game_y, game_w, game_h);
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

void Window::delete_all_texture_no_use()
{
    for (int i = 0; i < COUNT_USED_DIRECTORY; i++)
        if (i != MOUSE_CURSOR_DIRECTORY &&
            i != CHERRYBOMB_EXPLOSION_DIRECTORY &&
            i != POTATOMINE_EXPLOSION_DIRECTORY &&
            ++last_render[i] > 300)
        {
            delete_texture(i);
        }
}