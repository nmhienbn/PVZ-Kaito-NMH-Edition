#include "rsdl.hpp"

#include <map>

namespace
{
std::map<const void *, float> g_debug_scales;
}

void rsdl_set_debug_scale(const void *key, float scale)
{
    if (key != NULL && scale != 1.0f)
        g_debug_scales[key] = scale;
    else
        g_debug_scales.erase(key);
}

void rsdl_clear_debug_scale(const void *key)
{
    g_debug_scales.erase(key);
}

void rsdl_clear_debug_scales()
{
    g_debug_scales.clear();
}

float rsdl_debug_scale(const void *key)
{
    auto it = g_debug_scales.find(key);
    return it == g_debug_scales.end() ? 1.0f : it->second;
}

/*
Set the current draw color (also the color clear_renderer() uses, like the
SDL renderer draw color did)
*/
void Window::set_color(RGB color)
{
    clear_r = color.red;
    clear_g = color.green;
    clear_b = color.blue;
}

/*
Draw filled rectangle (1x1 white texture, tinted)
*/
void Window::fill_rect(int x, int y, int width, int height, RGB color)
{
    set_color(color);
    batch.fillRect((float)x, (float)y, (float)width, (float)height,
                   (Uint8)color.red, (Uint8)color.green, (Uint8)color.blue);
}

/*
Draw line (1px thick quad)
*/
void Window::draw_line(int x1, int y1, int x2, int y2, RGB color)
{
    set_color(color);
    batch.drawLine(x1, y1, x2, y2, (Uint8)color.red, (Uint8)color.green, (Uint8)color.blue);
}

/*
Draw point (1x1 rect)
*/
void Window::draw_point(int x, int y, RGB color)
{
    set_color(color);
    batch.drawPoint(x, y, (Uint8)color.red, (Uint8)color.green, (Uint8)color.blue);
}

/*
Draw unfilled rectangle
*/
void Window::draw_rect(int x, int y, int width, int height, RGB color)
{
    draw_line(x, y, x + width, y, color);
    draw_line(x, y, x, y + height, color);
    draw_line(x, y + height, x + width, y + height, color);
    draw_line(x + width, y, x + width, y + height, color);
}
