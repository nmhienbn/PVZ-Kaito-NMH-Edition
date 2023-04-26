#include "volume.hpp"

extern window win;
extern int music_volume;
extern int sfx_volume;
extern Mix_Music *gMusic[COUNT_MUSIC_DIRECTORY];
extern Mix_Chunk *gChunk[COUNT_MUSIC_DIRECTORY];
extern int now_music;

Button MUSIC_VOLUME(MENU_X1 + 194, MENU_X1 + 334, MENU_Y1 + 139, MENU_Y1 + 168);
Button SFX_VOLUME(MENU_X1 + 194, MENU_X1 + 334, MENU_Y1 + 168, MENU_Y1 + 197);

int music_x = (MUSIC_VOLUME.x1 + MUSIC_VOLUME.x2) / 2 - 11;
int music_y = MUSIC_VOLUME.y1;
int sfx_x = (SFX_VOLUME.x1 + SFX_VOLUME.x2) / 2 - 11;
int sfx_y = SFX_VOLUME.y1;

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

void display_slider()
{
    win.draw_png(SLIDER_DIRECTORY, music_x, music_y, 22, 29);
    win.draw_png(SLIDER_DIRECTORY, sfx_x, sfx_y, 22, 29);
}
