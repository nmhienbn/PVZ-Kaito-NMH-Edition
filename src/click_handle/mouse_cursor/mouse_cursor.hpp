#pragma once
#include "draw/rsdl.hpp"

/*
Set game mouse cursor
*/
class Mouse
{
    SDL_Cursor *Cursor;

public:
    Mouse();

    ~Mouse();

    void set_cursor();
};