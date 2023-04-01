#include "button.h"

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

void Button::blink(window &win) const
{
    int _x, _y;
    SDL_GetMouseState(&_x, &_y);
    if (is_mouse_in(_x, _y))
    {
        win.draw_png(WHITE_SCREEN_DIRECTORY, x1, y1, x2 - x1, y2 - y1);
    }
}
