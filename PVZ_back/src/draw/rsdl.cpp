#include <iostream>
#include <sstream>
#include "draw/rsdl.hpp"

using namespace std;

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
    else if (file_num == ZOMBIE_WALK1_BLINK_DIRECTORY ||
             file_num == ZOMBIE_WALK2_BLINK_DIRECTORY ||
             file_num == ZOMBIE_WALK3_BLINK_DIRECTORY ||
             file_num == ZOMBIE_EATING_BLINK_DIRECTORY ||
             file_num == FLAG_ZOMBIE_WALK_BLINK_DIRECTORY ||
             file_num == FLAG_ZOMBIE_EATING_BLINK_DIRECTORY ||
             file_num == CONE_ZOMBIE_WALK_1_BLINK_DIRECTORY ||
             file_num == CONE_ZOMBIE_WALK_2_BLINK_DIRECTORY ||
             file_num == CONE_ZOMBIE_WALK_3_BLINK_DIRECTORY ||
             file_num == CONE_ZOMBIE_EATING_1_BLINK_DIRECTORY ||
             file_num == CONE_ZOMBIE_EATING_2_BLINK_DIRECTORY ||
             file_num == CONE_ZOMBIE_EATING_3_BLINK_DIRECTORY ||
             file_num == BUCKET_ZOMBIE_WALK_1_BLINK_DIRECTORY ||
             file_num == BUCKET_ZOMBIE_WALK_2_BLINK_DIRECTORY ||
             file_num == BUCKET_ZOMBIE_WALK_3_BLINK_DIRECTORY ||
             file_num == BUCKET_ZOMBIE_EATING_1_BLINK_DIRECTORY ||
             file_num == BUCKET_ZOMBIE_EATING_2_BLINK_DIRECTORY ||
             file_num == BUCKET_ZOMBIE_EATING_3_BLINK_DIRECTORY ||

             file_num == SUNFLOWER_SHEET_BLINK_DIRECTORY ||
             file_num == WALNUT_1_BLINK_DIRECTORY ||
             file_num == WALNUT_2_BLINK_DIRECTORY ||
             file_num == WALNUT_3_BLINK_DIRECTORY ||
             file_num == WALNUT_4_BLINK_DIRECTORY ||
             file_num == PEASHOOTER_SHEET_BLINK_DIRECTORY ||
             file_num == PEASHOOTER_ATTACK_BLINK_DIRECTORY ||
             file_num == CHERRYBOMB_SHEET_BLINK_DIRECTORY)
    {
        SDL_SetTextureAlphaMod(res, 70);
    }
    else if (
        file_num == ZOMBIE_WALK1_COLD_DIRECTORY ||
        file_num == ZOMBIE_WALK2_COLD_DIRECTORY ||
        file_num == ZOMBIE_WALK3_COLD_DIRECTORY ||
        file_num == ZOMBIE_EATING_COLD_DIRECTORY ||
        file_num == FLAG_ZOMBIE_WALK_COLD_DIRECTORY ||
        file_num == FLAG_ZOMBIE_EATING_COLD_DIRECTORY ||
        file_num == ZOMBIE_HEAD_COLD_DIRECTORY ||
        file_num == ZOMBIE_DIE_COLD_DIRECTORY ||
        file_num == CONE_ZOMBIE_WALK_1_COLD_DIRECTORY ||
        file_num == CONE_ZOMBIE_WALK_2_COLD_DIRECTORY ||
        file_num == CONE_ZOMBIE_WALK_3_COLD_DIRECTORY ||
        file_num == CONE_ZOMBIE_EATING_1_COLD_DIRECTORY ||
        file_num == CONE_ZOMBIE_EATING_2_COLD_DIRECTORY ||
        file_num == CONE_ZOMBIE_EATING_3_COLD_DIRECTORY ||
        file_num == BUCKET_ZOMBIE_WALK_1_COLD_DIRECTORY ||
        file_num == BUCKET_ZOMBIE_WALK_2_COLD_DIRECTORY ||
        file_num == BUCKET_ZOMBIE_WALK_3_COLD_DIRECTORY ||
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

window::~window()
{
    SDL_DestroyWindow(win);
    if (TTF_WasInit())
        TTF_Quit();
    SDL_Quit();
}

TTF_Font *window::get_font(string font_addr, RGB color, int size)
{
    font_addr = FONTS_DIRECTORY + font_addr;
    stringstream ss;
    ss << size;
    TTF_Font *font = fonts_cache[font_addr + ":" + ss.str()];
    if (font == NULL)
    {
        font = TTF_OpenFont(font_addr.c_str(), size);
        fonts_cache[font_addr + ":" + ss.str()] = font;
    }
    return font;
}

/*
Fixed: cannot show many texts.
*/
void window::show_text(string input, int x, int y, RGB color, string font_addr, int size)
{
    font_addr = FONTS_DIRECTORY + font_addr;
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
    SDL_Rect renderQuad = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, text, NULL, &renderQuad);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(text);
}

void window::set_color(RGB color)
{
    SDL_SetRenderDrawColor(renderer, color.red, color.green, color.blue, 255);
}

void window::draw_bmp(int file_num, int x, int y, int width, int height)
{
    if (file_num == NULL_DIRECTORY)
        return;
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
    if (file_num == NULL_DIRECTORY)
        return;
    SDL_Texture *res = texture_cache[file_num];
    int mWidth = 0, mHeight = 0;
    if (res == NULL)
    {
        res = IMG_LoadTexture(renderer, image_directory[file_num].c_str());
        print_error(res, image_directory[file_num]);
        set_default_alpha(file_num, res);
        texture_cache[file_num] = res;
    }
    SDL_QueryTexture(res, NULL, NULL, &mWidth, &mHeight);
    SDL_Rect r = {x, y, width, width * mHeight / mWidth};
    SDL_RenderCopy(renderer, res, NULL, &r);
}

void window::draw_png_center(int file_num)
{
    if (file_num == NULL_DIRECTORY)
        return;
    SDL_Texture *res = texture_cache[file_num];
    int mWidth = 0, mHeight = 0;
    if (res == NULL)
    {
        res = IMG_LoadTexture(renderer, image_directory[file_num].c_str());
        print_error(res, image_directory[file_num]);
        set_default_alpha(file_num, res);
        texture_cache[file_num] = res;
    }
    SDL_QueryTexture(res, NULL, NULL, &mWidth, &mHeight);
    SDL_Rect r = {(WINDOW_WIDTH - mWidth) >> 1, (WINDOW_HEIGHT - mHeight) >> 1, mWidth, mHeight};
    SDL_RenderCopy(renderer, res, NULL, &r);
}

void window::draw_png(int file_num, int x, int y, int width, int height)
{
    if (file_num == NULL_DIRECTORY)
        return;
    SDL_Texture *res = texture_cache[file_num];
    if (res == NULL)
    {
        res = IMG_LoadTexture(renderer, image_directory[file_num].c_str());
        print_error(res, image_directory[file_num]);
        set_default_alpha(file_num, res);
        texture_cache[file_num] = res;
    }
    SDL_Rect r = {x, y, width, height};
    SDL_RenderCopy(renderer, res, NULL, &r);
}

void window::draw_png(int file_num, int x, int y, int width, int height, int angle)
{
    if (file_num == NULL_DIRECTORY)
        return;
    SDL_Texture *res = texture_cache[file_num];
    if (res == NULL)
    {
        res = IMG_LoadTexture(renderer, image_directory[file_num].c_str());
        print_error(res, image_directory[file_num]);
        texture_cache[file_num] = res;
    }
    SDL_Rect r = {x, y, width, height};
    SDL_RenderCopyEx(renderer, res, NULL, &r, angle, NULL, SDL_FLIP_NONE);
}

void window::draw_png(int file_num, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh)
{
    if (file_num == NULL_DIRECTORY)
        return;
    SDL_Texture *res = texture_cache[file_num];
    if (res == NULL)
    {
        res = IMG_LoadTexture(renderer, image_directory[file_num].c_str());
        print_error(res, image_directory[file_num]);
        set_default_alpha(file_num, res);
        texture_cache[file_num] = res;
    }
    SDL_Rect src = {sx, sy, sw, sh};
    SDL_Rect dst = {dx, dy, dw, sh * dw / sw};
    SDL_RenderCopy(renderer, res, &src, &dst);
}

void window::draw_bg(int file_num, int x, int y)
{
    if (file_num == NULL_DIRECTORY)
        return;
    SDL_Texture *res = texture_cache[file_num];
    if (res == NULL)
    {
        res = IMG_LoadTexture(renderer, image_directory[file_num].c_str());
        print_error(res, image_directory[file_num]);
        set_default_alpha(file_num, res);
        texture_cache[file_num] = res;
    }
    SDL_Rect src = {x, y, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_Rect dst = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderCopy(renderer, res, &src, &dst);
}

void window::update_screen()
{
    SDL_RenderPresent(renderer);
    int now_time = SDL_GetTicks() - previous_time;
    int delay_time = std::max(5, ticks_per_frame - now_time);
    DELAY(delay_time);
    previous_time = SDL_GetTicks();
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
void window::clear_renderer()
{
    SDL_RenderClear(renderer);
}
void window::set_texture_alpha(int file_num, int a)
{
    if (texture_cache[file_num] != NULL)
    {
        SDL_SetTextureAlphaMod(texture_cache[file_num], a);
    }
}
void print_error(SDL_Texture *res, const string &img)
{
    if (res == NULL)
    {
        printf("Cannot load image %s! Error: %s\n", img.c_str(), SDL_GetError());
        exit(-1);
    }
}
void window::fade_out()
{
    for (int i = 0; i <= 255; i++)
    {
        draw_png(BLACK_SCREEN_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        set_texture_alpha(BLACK_SCREEN_DIRECTORY, (i >> 3));
        update_screen();
    }
    set_texture_alpha(BLACK_SCREEN_DIRECTORY, 150);
    clear_renderer();
}

void window::show_announcer_text(const string &announcer)
{
    announce = announcer;
    time_announce = MAX_TIME_ANNOUNCE;
}

void window::show_announcer_text()
{
    if (time_announce > 0)
    {
        time_announce--;
        draw_png(BLACK_SCREEN_DIRECTORY, 0, 500, WINDOW_WIDTH, 100);
        int w = 0, h = 0;
        TTF_SizeText(get_font("HouseofTerror.ttf", WHITE, 50), announce.c_str(), &w, &h);
        show_text(announce, (WINDOW_WIDTH - w) / 2, 500 + (100 - h) / 2, WHITE, "HouseofTerror.ttf", 50);
    }
}
