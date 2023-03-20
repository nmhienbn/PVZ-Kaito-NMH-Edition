#include "main.h"

int main(int argc, char *args[])
{
    // Start up SDL and create window
    if (!init())
    {
        printf("Failed to initialize!\n");
        exit(-1);
    }
    // Load media
    if (!loadMedia() || !loadIcon())
    {
        printf("Failed to load media!\n");
        exit(-1);
    }

    // Load icon
    if (!loadIcon())
    {
        printf("Failed to load Icon!\n");
        exit(-1);
    }

    backgroundTilesX[0] = 32;
    backgroundTilesY[0] = 83;

    for (int i = 1; i < 9; i++)
        backgroundTilesX[i] = backgroundTilesX[i - 1] + 82;
    for (int i = 1; i < 5; i++)
        backgroundTilesY[i] = backgroundTilesY[i - 1] + 97;

    // Main loop flag
    bool quit = false;

    // Event handler
    SDL_Event e;

    PeaShooter *pea1[10];
    for (int i = 0; i < 10; i++)
    {
        pea1[i] = new PeaShooter(i / 5 + 1, i % 5 + 1);
        pea1[i]->setAttack();
        pea1[i]->loadPlantMedia();
    }

    std::unordered_set<PeaShooter *> alivePeaShooter;
    for (int i = 0; i < 10; i++)
        alivePeaShooter.insert(pea1[i]);

    loadBackground();
    // The frames per second cap timer
    LTimer capTimer;

    int frame = 0;
    // While application is running
    while (!quit)
    {
        // Start cap timer
        capTimer.start();

        // Handle events on queue
        while (SDL_PollEvent(&e) != 0)
        {
            // User requests quit
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(gRenderer);

        // Render wall
        SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);

        renderGameplayBackground();
        for (auto PS : alivePeaShooter)
        {
            PS->drawPlant();
        }

        for (auto PS : alivePeaShooter)
        {
            PS->drawBullet();
        }

        // Update screen
        SDL_RenderPresent(gRenderer);

        // Update frame
        if (frame <= 300)
            frame++;
        else
        {
            // if (alivePeaShooter.find(pea1) != alivePeaShooter.end())
            // {
            //     alivePeaShooter.erase(pea1);
            //     delete pea1;
            // }
        }

        // If frame finished early
        int frameTicks = capTimer.getTicks();
        if (frameTicks < SCREEN_TICK_PER_FRAME)
        {
            // Wait remaining time
            SDL_Delay(SCREEN_TICK_PER_FRAME - frameTicks);
        }
    }

    // Free resources and close SDL
    close();

    return 0;
}