#include "music.hpp"
#include "events/in_game/volume/volume.hpp"
int music_volume = MIX_MAX_VOLUME / 2;
int sfx_volume = MIX_MAX_VOLUME / 2;

// The music that will be played
Mix_Music *gMusic;
Mix_Chunk *gChunk[COUNT_MUSIC_DIRECTORY];
int now_music = -1;

/*Init music*/
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
        // Change number of channels - number of sfx can be used at a time
        Mix_AllocateChannels(200);
    }
    init_volume();
    return success;
}

/*Play music*/
void play_music(int num_path, int repeat_times)
{
    if (num_path == NULL_MUSIC_DIRECTORY)
    {
        return;
    }
    // If other music is playing
    if (now_music != num_path)
    {
        // Stop playing music
        Mix_HaltMusic();
        // Free the music
        Mix_FreeMusic(gMusic);
        // Update current music
        now_music = num_path;
        // Set volume
        Mix_VolumeMusic(music_volume);
        // Load music
        gMusic = Mix_LoadMUS(music_directory[num_path].c_str());
        if (gMusic == NULL)
        {
            printf("Failed to load %s! SDL_mixer Error: %s\n", music_directory[num_path].c_str(), Mix_GetError());
        }
        // If there is no music playing
        if (Mix_PlayingMusic() == 0)
        {
            // Play the music
            Mix_PlayMusic(gMusic, repeat_times);
        }
    }
}

/*Play sound effect*/
void play_sound_effect(int num_path)
{
    if (num_path == NULL_MUSIC_DIRECTORY)
    {
        return;
    }
    Mix_VolumeChunk(gChunk[num_path], sfx_volume);
    if (gChunk[num_path] == NULL)
    {
        gChunk[num_path] = Mix_LoadWAV(music_directory[num_path].c_str());
        Mix_VolumeChunk(gChunk[num_path], sfx_volume);
    }
    if (gChunk[num_path] == NULL)
    {
        printf("Failed to load %s sound effect! SDL_mixer Error: %s\n", music_directory[num_path].c_str(), Mix_GetError());
    }
    Mix_PlayChannel(-1, gChunk[num_path], 0);
}

/*Close the music*/
void close_music()
{
    // Free the music
    Mix_FreeMusic(gMusic);
    gMusic = NULL;
    // Free the chunks
    for (int i = 0; i < COUNT_MUSIC_DIRECTORY; i++)
    {
        Mix_FreeChunk(gChunk[i]);
        gChunk[i] = NULL;
    }

    // Quit SDL subsystems
    Mix_Quit();
}
