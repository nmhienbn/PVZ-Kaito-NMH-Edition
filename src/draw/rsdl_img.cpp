#include <iostream>
#include "rsdl.hpp"
using namespace std;

/*
Set default alpha blender for image in first loaded
*/
void set_default_alpha(int file_num, SDL_Texture *res)
{
    if (file_num == SHOVEL_DIRECTORY ||
        file_num == PEASHOOTER_DIRECTORY ||
        file_num == SUNFLOWER_DIRECTORY ||
        file_num == WALNUT_DIRECTORY ||
        file_num == SNOWPEA_DIRECTORY ||
        file_num == CHERRYBOMB_DIRECTORY)
    {
        SDL_SetTextureAlphaMod(res, 200);
    }
    else if (file_num == BLACK_SCREEN_DIRECTORY)
    {
        SDL_SetTextureAlphaMod(res, 150);
    }
    else if (
        // ZOMBIES
        file_num == ZOMBIE_WALK1_BLINK_DIRECTORY ||
        file_num == ZOMBIE_WALK2_BLINK_DIRECTORY ||
        file_num == ZOMBIE_EATING_BLINK_DIRECTORY ||

        file_num == ARMLESS_ZOMBIE_WALK1_BLINK_DIRECTORY ||
        file_num == ARMLESS_ZOMBIE_WALK2_BLINK_DIRECTORY ||
        file_num == ARMLESS_ZOMBIE_EATING_BLINK_DIRECTORY ||

        file_num == FLAG_ZOMBIE_WALK_BLINK_DIRECTORY ||
        file_num == FLAG_ZOMBIE_EATING_BLINK_DIRECTORY ||

        file_num == CONE_ZOMBIE_WALK1_1_BLINK_DIRECTORY ||
        file_num == CONE_ZOMBIE_WALK2_1_BLINK_DIRECTORY ||
        file_num == CONE_ZOMBIE_WALK1_2_BLINK_DIRECTORY ||
        file_num == CONE_ZOMBIE_WALK2_2_BLINK_DIRECTORY ||
        file_num == CONE_ZOMBIE_WALK1_3_BLINK_DIRECTORY ||
        file_num == CONE_ZOMBIE_WALK2_3_BLINK_DIRECTORY ||
        file_num == CONE_ZOMBIE_EATING_1_BLINK_DIRECTORY ||
        file_num == CONE_ZOMBIE_EATING_2_BLINK_DIRECTORY ||
        file_num == CONE_ZOMBIE_EATING_3_BLINK_DIRECTORY ||

        file_num == BUCKET_ZOMBIE_WALK1_1_BLINK_DIRECTORY ||
        file_num == BUCKET_ZOMBIE_WALK2_1_BLINK_DIRECTORY ||
        file_num == BUCKET_ZOMBIE_WALK1_2_BLINK_DIRECTORY ||
        file_num == BUCKET_ZOMBIE_WALK2_2_BLINK_DIRECTORY ||
        file_num == BUCKET_ZOMBIE_WALK1_3_BLINK_DIRECTORY ||
        file_num == BUCKET_ZOMBIE_WALK2_3_BLINK_DIRECTORY ||
        file_num == BUCKET_ZOMBIE_EATING_1_BLINK_DIRECTORY ||
        file_num == BUCKET_ZOMBIE_EATING_2_BLINK_DIRECTORY ||
        file_num == BUCKET_ZOMBIE_EATING_3_BLINK_DIRECTORY ||

        // PLANTS
        file_num == PEASHOOTER_SHEET_BLINK_DIRECTORY ||
        file_num == PEASHOOTER_ATTACK_BLINK_DIRECTORY ||

        file_num == SUNFLOWER_SHEET_BLINK_DIRECTORY ||
        file_num == SUNFLOWER_HAPPY_BLINK_DIRECTORY ||

        file_num == WALNUT_1_BLINK_DIRECTORY ||
        file_num == WALNUT_2_BLINK_DIRECTORY ||
        file_num == WALNUT_3_BLINK_DIRECTORY ||
        file_num == WALNUT_4_BLINK_DIRECTORY ||
        file_num == WALNUT_5_BLINK_DIRECTORY ||

        file_num == SNOWPEA_SHEET_BLINK_DIRECTORY ||
        file_num == SNOWPEA_ATTACK_BLINK_DIRECTORY ||

        file_num == CHERRYBOMB_SHEET_BLINK_DIRECTORY)
    {
        SDL_SetTextureAlphaMod(res, 70);
    }
    else if (
        file_num == ZOMBIE_WALK1_COLD_DIRECTORY ||
        file_num == ZOMBIE_WALK2_COLD_DIRECTORY ||
        file_num == ZOMBIE_EATING_COLD_DIRECTORY ||

        file_num == ARMLESS_ZOMBIE_WALK1_COLD_DIRECTORY ||
        file_num == ARMLESS_ZOMBIE_WALK2_COLD_DIRECTORY ||
        file_num == ARMLESS_ZOMBIE_EATING_COLD_DIRECTORY ||

        file_num == FLAG_ZOMBIE_WALK_COLD_DIRECTORY ||
        file_num == FLAG_ZOMBIE_EATING_COLD_DIRECTORY ||

        file_num == ZOMBIE_HEAD_COLD_DIRECTORY ||
        file_num == ZOMBIE_DIE1_COLD_DIRECTORY ||
        file_num == ZOMBIE_DIE2_COLD_DIRECTORY ||

        file_num == CONE_ZOMBIE_WALK1_1_COLD_DIRECTORY ||
        file_num == CONE_ZOMBIE_WALK2_1_COLD_DIRECTORY ||
        file_num == CONE_ZOMBIE_WALK1_2_COLD_DIRECTORY ||
        file_num == CONE_ZOMBIE_WALK2_2_COLD_DIRECTORY ||
        file_num == CONE_ZOMBIE_WALK1_3_COLD_DIRECTORY ||
        file_num == CONE_ZOMBIE_WALK2_3_COLD_DIRECTORY ||
        file_num == CONE_ZOMBIE_EATING_1_COLD_DIRECTORY ||
        file_num == CONE_ZOMBIE_EATING_2_COLD_DIRECTORY ||
        file_num == CONE_ZOMBIE_EATING_3_COLD_DIRECTORY ||

        file_num == BUCKET_ZOMBIE_WALK1_1_COLD_DIRECTORY ||
        file_num == BUCKET_ZOMBIE_WALK2_1_COLD_DIRECTORY ||
        file_num == BUCKET_ZOMBIE_WALK1_2_COLD_DIRECTORY ||
        file_num == BUCKET_ZOMBIE_WALK2_2_COLD_DIRECTORY ||
        file_num == BUCKET_ZOMBIE_WALK1_3_COLD_DIRECTORY ||
        file_num == BUCKET_ZOMBIE_WALK2_3_COLD_DIRECTORY ||
        file_num == BUCKET_ZOMBIE_EATING_1_COLD_DIRECTORY ||
        file_num == BUCKET_ZOMBIE_EATING_2_COLD_DIRECTORY ||
        file_num == BUCKET_ZOMBIE_EATING_3_COLD_DIRECTORY)
    {
        SDL_SetTextureAlphaMod(res, 120);
    }
    else if (file_num == SUN2_DIRECTORY)
    {
        SDL_SetTextureAlphaMod(res, 200);
    }
    else if (file_num == LEVEL_BLINK_DIRECTORY ||
             file_num == WHITE_SCREEN_DIRECTORY)
    {
        SDL_SetTextureAlphaMod(res, 70);
    }
    else if (file_num == FRONTYARD_C1 ||
             file_num == FRONTYARD_C2 ||
             file_num == FRONTYARD_C3 ||
             file_num == FRONTYARD_C4 ||
             file_num == FRONTYARD_C5 ||
             file_num == FRONTYARD_C6 ||
             file_num == FRONTYARD_C7 ||
             file_num == FRONTYARD_C8 ||
             file_num == FRONTYARD_C9 ||
             file_num == FRONTYARD_R1 ||
             file_num == FRONTYARD_R2 ||
             file_num == FRONTYARD_R3 ||
             file_num == FRONTYARD_R4 ||
             file_num == FRONTYARD_R5)
    {
        SDL_SetTextureAlphaMod(res, 120);
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

SDL_Texture *window::load_texture(int file_num)
{
    SDL_Texture *res = texture_cache[file_num];
    if (res == NULL)
    {
        res = IMG_LoadTexture(renderer, image_directory[file_num].c_str());
        print_error(res, image_directory[file_num]);
        set_default_alpha(file_num, res);
        texture_cache[file_num] = res;
    }
    return res;
}

/*
Load & draw bmp:
    Load: Surface -> Texture
    Draw: SDL_RenderCopy
*/
void window::draw_bmp(int file_num, int x, int y, int width, int height)
{
    if (file_num == NULL_DIRECTORY || width <= 0 || height <= 0)
        return;
    SDL_Texture *res = texture_cache[file_num];
    if (res == NULL)
    {
        SDL_Surface *surface = SDL_LoadBMP(image_directory[file_num].c_str());
        res = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        print_error(res, image_directory[file_num]);
        set_default_alpha(file_num, res);
        texture_cache[file_num] = res;
    }
    SDL_Rect r = {x, y, width, height};
    SDL_RenderCopy(renderer, res, NULL, &r);
}

/*
Load & draw with scale: height = width * mHeight / mWidth
    Load: Texture
    Draw: SDL_RenderCopy
    mHeight, mWidth: SDL_QueryTexture
*/
void window::draw_png_scale(int file_num, int x, int y, int width, int height)
{
    if (file_num == NULL_DIRECTORY || width <= 0)
        return;
    SDL_Texture *res = load_texture(file_num);
    int mWidth = 0, mHeight = 0;
    SDL_QueryTexture(res, NULL, NULL, &mWidth, &mHeight);
    SDL_Rect r = {x, y, width, width * mHeight / mWidth};
    SDL_RenderCopy(renderer, res, NULL, &r);
}

/*
Load & draw in the center of window
    Load: Texture
    Draw: SDL_RenderCopy
    mHeight, mWidth: SDL_QueryTexture
*/
void window::draw_png_center(int file_num)
{
    if (file_num == NULL_DIRECTORY)
        return;
    SDL_Texture *res = load_texture(file_num);
    int mWidth = 0, mHeight = 0;
    SDL_QueryTexture(res, NULL, NULL, &mWidth, &mHeight);
    SDL_Rect r = {(WINDOW_WIDTH - mWidth) >> 1, (WINDOW_HEIGHT - mHeight) >> 1, mWidth, mHeight};
    SDL_RenderCopy(renderer, res, NULL, &r);
}

/*
Load & draw in the SDL_Rect(x, y, width, height)
    Load: Texture
    Draw: SDL_RenderCopy
*/
void window::draw_png(int file_num, int x, int y, int width, int height)
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
void window::draw_png(int file_num, int x, int y, int width, int height, int angle)
{
    if (file_num == NULL_DIRECTORY || width <= 0 || height <= 0)
        return;
    SDL_Texture *res = load_texture(file_num);
    SDL_Rect r = {x, y, width, height};
    SDL_RenderCopyEx(renderer, res, NULL, &r, angle, NULL, SDL_FLIP_NONE);
}

/*
Load & draw with scale: from source SDL_Rect(sx, sy, sw, sh) to destination SDL_Rect(dx, dy, dw, dh)
    Load: Texture
    Draw: SDL_RenderCopy
*/
void window::draw_png(int file_num, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh)
{
    if (file_num == NULL_DIRECTORY || sw <= 0 || sh <= 0 || dw <= 0 || dh <= 0)
        return;
    SDL_Texture *res = load_texture(file_num);
    SDL_Rect src = {sx, sy, sw, sh};
    SDL_Rect dst = {dx, dy, dw, sh * dw / sw};
    SDL_RenderCopy(renderer, res, &src, &dst);
}

/*
Load & draw image to all window:
    Load: Texture
    Draw: SDL_RenderCopy
*/
void window::draw_bg(int file_num, int x, int y)
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
void window::set_texture_alpha(int file_num, int a)
{
    if (file_num != NULL_DIRECTORY && texture_cache[file_num] != NULL)
    {
        SDL_SetTextureAlphaMod(texture_cache[file_num], a);
    }
}

extern bool quit;
/*
Fade out effect
*/
void window::fade_out()
{
    for (int i = 0; i <= 64; i++)
    {
        draw_png(BLACK_SCREEN_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        set_texture_alpha(BLACK_SCREEN_DIRECTORY, (i));
        update_screen();
        HANDLE(QUIT(quit = true; return;))
    }
    set_texture_alpha(BLACK_SCREEN_DIRECTORY, 150);
    clear_renderer();
}