#include "elements/button/button.hpp"
#include "button.hpp"

extern Window win;

/*Button constructor*/
Button::Button()
{
}

/*Button constructor with rectangle coordination.*/
Button::Button(int _x1, int _x2, int _y1, int _y2)
{
    x1 = _x1;
    x2 = _x2;
    y1 = _y1;
    y2 = _y2;
}

/*Button destructor*/
Button::~Button()
{
}

/*
@return true if current mouse is over the button
*/
bool Button::is_mouse_in(int mouse_x, int mouse_y) const
{
    if (x1 < mouse_x && mouse_x < x2 &&
        y1 < mouse_y && mouse_y < y2)
        return true;
    return false;
}

/*
Display blink effect when mouse move over the button.
*/
void Button::blink() const
{
    int _x, _y;
    SDL_GetMouseState(&_x, &_y);
    if (is_mouse_in(_x, _y))
    {
        win.draw_png(WHITE_SCREEN_DIRECTORY, x1, y1, x2 - x1, y2 - y1);
    }
}

void Button::show_text(const string &text, const int &x_bias, const int &y_bias,
                       const int &font_size, const RGB &color, const RGB &shadow_color) const
{
    int w, h;
    TTF_SizeText(win.get_font(DWARVESC_TTF, font_size), text.c_str(), &w, &h);
    int x = (x2 + x1 - w) / 2 + x_bias;
    int y = (y2 + y1 - h) / 2 + y_bias;
    win.show_text_shadowed(text, x, y, color, DWARVESC_TTF, font_size, 0, 1, shadow_color);
}
/*
Display Game Buttons
*/
void display_button(const Button &button, const int &button_directory, int angle, SDL_RendererFlip flip)
{
    win.draw_png(button_directory, button.x1, button.y1, button.x2 - button.x1, button.y2 - button.y1, angle, flip);
}

/*
Display transparent black when level is locked
*/
void display_level_is_locked(const Button &button)
{
    win.draw_png_height_scaled(BLACK_SCREEN_DIRECTORY, button.x1, button.y1, button.x2 - button.x1);
}
