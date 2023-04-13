#include "mouse_cursor.hpp"

/*
Load game mouse cursor
*/
Mouse::Mouse()
{
    SDL_Surface *tmp = IMG_Load(image_directory[MOUSE_CURSOR_DIRECTORY].c_str());
    Cursor = SDL_CreateColorCursor(tmp, 0, 0);
    SDL_FreeSurface(tmp);
}

/*
Free game mouse cursor
*/
Mouse::~Mouse()
{
    SDL_FreeCursor(Cursor);
    Cursor = NULL;
}

/*
Set game mouse cursor
*/
void Mouse::set_cursor()
{
    SDL_SetCursor(Cursor);
}