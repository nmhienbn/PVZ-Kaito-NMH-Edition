#include "mouse_cursor.hpp"

Mouse::Mouse()
{
    SDL_Surface *tmp = IMG_Load(image_directory[MOUSE_CURSOR_DIRECTORY].c_str());
    Cursor = SDL_CreateColorCursor(tmp, 0, 0);
    SDL_FreeSurface(tmp);
}

Mouse::~Mouse()
{
    SDL_FreeCursor(Cursor);
    Cursor = NULL;
}

void Mouse::set_cursor()
{
    SDL_SetCursor(Cursor);
}