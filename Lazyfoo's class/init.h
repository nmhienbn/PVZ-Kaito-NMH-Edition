#ifndef INIT_H
#define INIT_H

// Using SDL, SDL_image, standard IO, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include "LTexture.h"

// Starts up SDL and creates window
bool init();

// Loads media
bool loadMedia();

// Frees media and shuts down SDL
void close();

// Box collision detector
bool checkCollision(SDL_Rect a, SDL_Rect b);

#endif // INIT_H