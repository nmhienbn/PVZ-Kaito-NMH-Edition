#include "background.h"

// The window we'll be rendering to
extern SDL_Window *gWindow;

// The window renderer
extern SDL_Renderer *gRenderer;

// Background textures
LTexture gBackgroundTexture;

// Shop textures
LTexture gShopTexture;

bool loadBackgroundMedia()
{
    // Loading success flag
    bool success = true;

    // Load background texture
    if (!gBackgroundTexture.loadMedia("images/Background.jpg"))
    {
        success = false;
    }
    // Load shop texture
    if (!gShopTexture.loadMedia("images/ChooserBackground.png"))
    {
        success = false;
    }

    return success;
}

void loadBackground()
{
    for (int x = 0, _ = gBackgroundTexture.getWidth() - 800; x <= _; x += 10)
    {
        // Clear screen
        SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(gRenderer);

        // Render background
        SDL_Rect gBackgroundRect = {x, 0, 800, gBackgroundTexture.getHeight()};
        gBackgroundTexture.render(0, 0, &gBackgroundRect);

        // Update screen
        SDL_RenderPresent(gRenderer);
    }
    // SDL_Delay(5000);
    // Select plants
    for (int x = gBackgroundTexture.getWidth() - 800; x >= 220; x -= 10)
    {
        // Clear screen
        SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(gRenderer);

        // Render background
        SDL_Rect gBackgroundRect = {x, 0, 800, gBackgroundTexture.getHeight()};
        gBackgroundTexture.render(0, 0, &gBackgroundRect);

        // Update screen
        SDL_RenderPresent(gRenderer);
    }
    // Loading plant seeds
}

void renderGameplayBackground()
{
    SDL_Rect gBackgroundRect = {220, 0, 800, gBackgroundTexture.getHeight()};
    gBackgroundTexture.render(0, 0, &gBackgroundRect);

    SDL_Rect gShopRect = {0, 0, gShopTexture.getWidth(), gShopTexture.getHeight()};
    gShopTexture.render(100, 0, &gShopRect);
}

void initTiles()
{
}