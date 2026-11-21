#include "music.hpp"
#include "events/in_game/volume/volume.hpp"
#include <fstream>
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
    if (now_music == num_path)
    {
        return;
    }
    // Stop the currently playing track.
    Mix_HaltMusic();
    // Free the music (Mix_FreeMusic(NULL) is a no-op).
    Mix_FreeMusic(gMusic);
    gMusic = NULL;
    // Update current music
    now_music = num_path;
    // Set volume
    Mix_VolumeMusic(music_volume);
    // Load music
    gMusic = Mix_LoadMUS(music_directory[num_path].c_str());
    if (gMusic == NULL)
    {
        printf("Failed to load %s! SDL_mixer Error: %s\n", music_directory[num_path].c_str(), Mix_GetError());
        return;
    }
    // Play the music
    Mix_PlayMusic(gMusic, repeat_times);
}

/*Warm up sound effects: decode into gChunk now (loading screen)*/
void warm_sound_effects(int begin, int end)
{
    if (begin < 0)
        begin = 0;
    if (end > COUNT_MUSIC_DIRECTORY)
        end = COUNT_MUSIC_DIRECTORY;
    for (int i = begin; i < end; i++)
    {
        if (gChunk[i] == NULL)
        {
            gChunk[i] = Mix_LoadWAV(music_directory[i].c_str());
            if (gChunk[i] != NULL)
                Mix_VolumeChunk(gChunk[i], sfx_volume);
        }
    }
}

/*Pull a music file through the page cache (Mix_LoadMUS streams later)*/
void prefetch_music_file(int num_path)
{
    if (num_path < 0 || num_path >= COUNT_MUSIC_DIRECTORY)
        return;
    ifstream f(music_directory[num_path], ios::binary);
    if (!f)
        return;
    static char buf[65536];
    volatile size_t eaten = 0;
    while (f.read(buf, sizeof buf) || f.gcount() > 0)
        eaten += f.gcount();
}

/*Play sound effect*/
void play_sound_effect(int num_path)
{
    if (num_path == NULL_MUSIC_DIRECTORY)
    {
        return;
    }
    if (gChunk[num_path] == NULL)
    {
        gChunk[num_path] = Mix_LoadWAV(music_directory[num_path].c_str());
    }
    if (gChunk[num_path] == NULL)
    {
        printf("Failed to load %s sound effect! SDL_mixer Error: %s\n", music_directory[num_path].c_str(), Mix_GetError());
        return;
    }
    Mix_VolumeChunk(gChunk[num_path], sfx_volume);
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
    Mix_CloseAudio();
    Mix_Quit();
}

void delete_all_sound_effect()
{
    for (int i = 0; i < COUNT_MUSIC_DIRECTORY; i++)
    {
        Mix_FreeChunk(gChunk[i]);
        gChunk[i] = NULL;
    }
}
