#include "volume.hpp"
#include "draw/rsdl.hpp"
#include "elements/button/button.hpp"
#include "music/music.hpp"

extern Window win;
extern int music_volume;
extern int sfx_volume;
extern Mix_Music *gMusic[COUNT_MUSIC_DIRECTORY];
extern Mix_Chunk *gChunk[COUNT_MUSIC_DIRECTORY];
extern int now_music;

// Music volume slider
int music_x, music_y;

// SFX volume slider
int sfx_x, sfx_y;

void init_volume()
{
    // Music volume slider
    music_x = (MUSIC_VOLUME.x1 + MUSIC_VOLUME.x2) / 2 - 11;
    music_y = MUSIC_VOLUME.y1;

    // SFX volume slider
    sfx_x = (SFX_VOLUME.x1 + SFX_VOLUME.x2) / 2 - 11;
    sfx_y = SFX_VOLUME.y1;
}

/*
Handle player mouse down on slide to change the volume.
Update all available music and sound effect.
*/
void handle_volume_change()
{
    int mouse_x, mouse_y;
    auto mouse_state = SDL_GetMouseState(&mouse_x, &mouse_y);
    if (mouse_state & SDL_BUTTON_LMASK)
    {
        if (MUSIC_VOLUME.is_mouse_in(mouse_x, mouse_y))
        {
            music_volume = (mouse_x - MUSIC_VOLUME.x1) * MIX_MAX_VOLUME / (MUSIC_VOLUME.x2 - MUSIC_VOLUME.x1);
            music_x = mouse_x - 11;
            Mix_VolumeMusic(music_volume);
        }
        if (SFX_VOLUME.is_mouse_in(mouse_x, mouse_y))
        {
            sfx_volume = (mouse_x - SFX_VOLUME.x1) * MIX_MAX_VOLUME / (SFX_VOLUME.x2 - SFX_VOLUME.x1);
            sfx_x = mouse_x - 11;
            for (int i = 0; i < COUNT_MUSIC_DIRECTORY; i++)
            {
                if (gChunk[i] != NULL)
                {
                    Mix_VolumeChunk(gChunk[i], sfx_volume);
                }
            }
        }
    }
}

/*
Display the slider.
*/
void display_slider()
{
    win.draw_png(SLIDER_DIRECTORY, music_x, music_y, 22, 29);
    win.draw_png(SLIDER_DIRECTORY, sfx_x, sfx_y, 22, 29);
}
