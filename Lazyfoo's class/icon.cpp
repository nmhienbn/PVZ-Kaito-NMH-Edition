#include "icon.h"

bool loadIcon()
{
    std::string path = "pvz.ico";
    // Load image at specified path
    SDL_Surface *iconSurface = IMG_Load(path.c_str());
    if (iconSurface == NULL)
    {
        printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
        return false;
    }
    // Color key image
    SDL_SetColorKey(iconSurface, SDL_TRUE, SDL_MapRGB(iconSurface->format, 0, 0xFF, 0xFF));

    // Set Application Icon
    SDL_SetWindowIcon(gWindow, iconSurface);

    printf("Icon have set!");

    // Get rid of old loaded surface
    SDL_FreeSurface(iconSurface);

    return true;
}