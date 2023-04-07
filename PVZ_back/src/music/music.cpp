#include "music/music.hpp"

// The music that will be played
static Mix_Music *gMusic[COUNT_MUSIC_DIRECTORY];
static Mix_Chunk *gChunk[COUNT_MUSIC_DIRECTORY];
static int now_music = -1;

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
    }
    return success;
}

void play_music(int num_path, int repeat_times)
{
    if (now_music != num_path)
    {
        Mix_HaltMusic();
        now_music = num_path;
        if (num_path == URF_DIRECTORY)
        {
            Mix_VolumeMusic(50);
        }
        else
        {
            Mix_VolumeMusic(128);
        }
        // Load music
        if (gMusic[num_path] == NULL)
        {
            gMusic[num_path] = Mix_LoadMUS(music_directory[num_path].c_str());
        }
        if (gMusic[num_path] == NULL)
        {
            printf("Failed to load %s! SDL_mixer Error: %s\n", music_directory[num_path].c_str(), Mix_GetError());
        }
        // If there is no music playing
        if (Mix_PlayingMusic() == 0)
        {
            // Play the music
            Mix_PlayMusic(gMusic[num_path], repeat_times);
        }
    }
}

void play_sound_effect(int num_path)
{
    if (gChunk[num_path] == NULL)
        gChunk[num_path] = Mix_LoadWAV(music_directory[num_path].c_str());
    if (gChunk[num_path] == NULL)
    {
        printf("Failed to load %s sound effect! SDL_mixer Error: %s\n", music_directory[num_path].c_str(), Mix_GetError());
    }
    Mix_PlayChannel(-1, gChunk[num_path], 0);
}

void close_music()
{
    // Free the music
    for (int i = 0; i < COUNT_MUSIC_DIRECTORY; i++)
    {
        Mix_FreeMusic(gMusic[i]);
        gMusic[i] = NULL;
        Mix_FreeChunk(gChunk[i]);
        gChunk[i] = NULL;
    }

    // Quit SDL subsystems
    Mix_Quit();
}
