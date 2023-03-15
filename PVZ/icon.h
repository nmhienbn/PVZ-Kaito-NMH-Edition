#ifndef icon_h
#define icon_h

#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>

// The window we'll be rendering to
extern SDL_Window *gWindow;

bool loadIcon();

#endif // icon_h