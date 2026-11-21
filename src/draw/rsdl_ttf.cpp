#include "rsdl.hpp"
#include <iostream>
#include <sstream>
using namespace std;

/*
Get the TTF_Font pointer of a font.
If font has not loaded, create new TTF_Font.
*/
TTF_Font *Window::get_font(string font_addr, const int &size)
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
Render `input` with `font` and draw it at (x, y) through the sprite batch.
The rendered texture is cached in text_cache (keyed by font, size, style,
outline, text and color) and evicted when unused for
TEXT_CACHE_EVICT_FRAMES frames (see update_screen()).
`utf8` selects TTF_RenderUTF8_Blended over TTF_RenderText_Blended.
Note: like the old SDL code, the surface alpha mod of `alpha` does not reach
the texture (SDL_CreateTextureFromSurface ignores it), so it only feeds the
cache key.
*/
void Window::show_text_cached(TTF_Font *font, const string &input, const int &x,
                              const int &y, const RGB &color, const int &alpha, bool utf8)
{
    if (font == NULL)
        return;
    stringstream key;
    key << (utf8 ? "u8" : "t8") << '|' << (size_t)font << '|'
        << TTF_GetFontStyle(font) << '|' << TTF_GetFontOutline(font) << '|'
        << color.red << ',' << color.green << ',' << color.blue << ','
        << alpha << '|' << input;

    TextCacheEntry &entry = text_cache[key.str()];
    entry.last_used_frame = frame_counter;
    if (entry.tex.id == 0)
    {
        SDL_Color textColor = {(Uint8)color.red, (Uint8)color.green, (Uint8)color.blue, 0};
        SDL_Surface *textSurface =
            utf8 ? TTF_RenderUTF8_Blended(font, input.c_str(), textColor)
                 : TTF_RenderText_Blended(font, input.c_str(), textColor);
        if (textSurface == NULL)
            return;
        entry.tex = batch.upload(textSurface);
        SDL_FreeSurface(textSurface);
        if (entry.tex.id == 0)
            return;
    }
    batch.drawQuad(entry.tex, 0, 0, (float)entry.tex.w, (float)entry.tex.h,
                   (float)x, (float)y, (float)entry.tex.w, (float)entry.tex.h);
}

/* Show the text at (x; y)
    Load: Font, Surface -> Texture (cached)
Fixed: cannot show many texts.
Runtime: Solid < Shaded < Blended
*/
void Window::show_text(const string &input, const int &x, const int &y,
                       const RGB &color, string font_addr, const int &size, const int &alpha)
{
    if (input == "")
        return;
    show_text_cached(get_font(font_addr, size), input, x, y, color, alpha, false);
}

/*
Set game announce and remaining time of announce
*/
void Window::show_announcer_text(const string &announcer, const int &time_a)
{
    announce = announcer;
    time_announce = time_a;
}

/*
Draw announcer.
*/
void Window::show_announcer_text()
{
    if (time_announce > 0)
    {
        // Auto-FPS: the countdown tracks LOGIC ticks (the gameplay frame
        // driver in level.cpp), not the uncapped render rate -- at 144 fps
        // the old per-call decrement erased the text 2.4x too fast. The
        // time_announce > 0 half of the guard keeps the dip at 0.
        for (int _lt = logic_ticked; _lt > 0 && time_announce > 0; _lt--)
            time_announce--;
        // set_texture_alpha(BLACK_SCREEN_DIRECTORY, 100);
        draw_png(BLACK_SCREEN_DIRECTORY, 0, 540, WINDOW_WIDTH, 100);
        // set_texture_alpha(BLACK_SCREEN_DIRECTORY, 150);
        int w = 0, h = 0;
        TTF_SizeText(get_font(HOUSEOFTERROR_TTF, 40), announce.c_str(), &w, &h);
        show_text_outlined(announce, (WINDOW_WIDTH - w) / 2, 540 + (60 - h) / 2, WHITE, HOUSEOFTERROR_TTF, 40);
    }
}

/*
Set a font style.
*/
void Window::set_style(const string &font_addr, const int &size, const int &style)
{
    TTF_Font *font = get_font(font_addr, size);
    TTF_SetFontStyle(font, style);
}

/*
Set a font outline - make "net chu" bigger
*/
void Window::set_outline(const string &font_addr, const int &size, const int &outline)
{
    TTF_Font *font = get_font(font_addr, size);
    TTF_SetFontOutline(font, outline);
}

/*
Show text shadowed by render it's outline (set font outline).
Then render it.
*/
void Window::show_text_outlined(const string &input, const int &x, const int &y,
                                const RGB &color, string font_addr,
                                const int &size, const int &outline_weight, const RGB &outline_color)
{
    set_outline(font_addr, size, outline_weight);
    show_text(input, x - outline_weight, y - outline_weight, outline_color, font_addr, size);
    set_outline(font_addr, size, 0);
    show_text(input, x, y, color, font_addr, size);
}

void Window::show_text_shadowed(const string &input, const int &x, const int &y,
                                const RGB &color, string font_addr,
                                const int &size, const int &x_shadow, const int &y_shadow,
                                const RGB &shadow_color)
{
    show_text(input, x + x_shadow, y + y_shadow, shadow_color, font_addr, size);
    show_text(input, x, y, color, font_addr, size);
}

/*
If x = -1, show text at center by x (by width)
If y = -1, show text at center by y (by height)
*/
void Window::show_text_center(const string &input, int x, int y,
                              const RGB &color, string font_addr,
                              const int &size, const int &style,
                              const int &outline_weight, const RGB &outline_color)
{
    int w = 0, h = 0;
    TTF_SizeText(get_font(font_addr, size), input.c_str(), &w, &h);
    if (x == -1)
        x = (WINDOW_WIDTH - w) / 2;
    if (y == -1)
        y = (WINDOW_HEIGHT - h) / 2;

    set_style(font_addr, size, style);
    show_text_outlined(input, x, y, color, font_addr, size, outline_weight, outline_color);
}

/* Show the utf 8 text at (x; y)
    Load: Font, Surface -> Texture (cached)
Fixed: cannot show many texts.
*/
void Window::show_text_utf8(const string &input, const int &x, const int &y,
                            const RGB &color, string font_addr, const int &size)
{
    if (input == "")
        return;
    show_text_cached(get_font(font_addr, size), input, x, y, color, 255, true);
}
