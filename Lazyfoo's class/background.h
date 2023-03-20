#ifndef BACKGROUND_H
#define BACKGROUND_H
// Using SDL, SDL_image, standard IO, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include "LTexture.h"
// #include "mouseEvent.h"

// const int NUMBER_OF_TILES_ROW = 5;
// const int NUMBER_OF_TILE_COLUMN = 9;

// LButton tile[NUMBER_OF_TILE_COLUMN][NUMBER_OF_TILES_ROW];

bool loadBackgroundMedia();
void loadBackground();
void renderGameplayBackground();
void initTiles();

#endif // BACKGROUND_H