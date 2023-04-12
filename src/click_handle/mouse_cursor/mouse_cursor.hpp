#pragma once
#include "draw/rsdl.hpp"

extern window win;

class Mouse
{
    SDL_Cursor *Cursor;

public:
    Mouse();

    ~Mouse();

    void set_cursor();
};