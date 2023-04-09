#include <iostream>
#include <sstream>
#include "rsdl.hpp"
using namespace std;

/*
Get the TTF_Font pointer of a font
*/
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

/* Show the text at (x; y)
    Load: Font, Surface -> Texture
Fixed: cannot show many texts.
*/
void window::show_text(string input, int x, int y, RGB color, string font_addr, int size)
{
    if (input == "")
        return;
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

/*
Set game announce and remaining time of announce
*/
void window::show_announcer_text(const string &announcer, const int &time_a)
{
    announce = announcer;
    time_announce = time_a;
}

/*
Draw announcer.
*/
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