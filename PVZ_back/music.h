#pragma once
#include "SDL.h"
#include "SDL_mixer.h"
#include "stdio.h"

#define BACKGROUND_MUSIC_DIRECTORY "audio/Brainiac_Maniac.mp3"

bool init_music();
void play_in_game_music();
void close_music();
