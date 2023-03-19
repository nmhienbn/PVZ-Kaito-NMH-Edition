#include "music.h"

// The music that will be played
static Mix_Music *gMusic = NULL;

bool init_music()
{
    // Initialization flag
    bool success = true;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        // Initialize SDL_mixer
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
        {
            printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
            success = false;
        }
        else
        {
            // Load music
            gMusic = Mix_LoadMUS(BACKGROUND_MUSIC_DIRECTORY);
            if (gMusic == NULL)
            {
                printf("Failed to load %s! SDL_mixer Error: %s\n", BACKGROUND_MUSIC_DIRECTORY, Mix_GetError());
                success = false;
            }
        }
    }
    return success;
}

void play_in_game_music()
{ // If there is no music playing
    if (Mix_PlayingMusic() == 0)
    {
        // Play the music
        Mix_PlayMusic(gMusic, -1);
    }
}

void close_music()
{
    // Free the music
    Mix_FreeMusic(gMusic);
    gMusic = NULL;

    // Quit SDL subsystems
    Mix_Quit();
}
