#include "peashooter.h"

extern SDL_Renderer *gRenderer;
bool isLoaded = false;

PeaShooter::PeaShooter()
{
}

PeaShooter::PeaShooter(int _xTile, int _yTile)
{
    if (_xTile < 1 || _xTile > 9 || _yTile < 1 || _yTile > 5)
    {
        printf("Invalid Tile coordinates!\n0 < xTile < 10\n0 < yTile < 6");
        x = INF;
        y = INF;
    }
    else
    {
        xTile = _xTile;
        yTile = _yTile;
        x = backgroundTilesX[xTile - 1];
        y = backgroundTilesY[yTile - 1];
    }
}

PeaShooter::~PeaShooter()
{
}

bool PeaShooter::IsDead()
{
    return isDead;
}

bool PeaShooter::loadPlantMedia()
{
    // if (isLoaded)
    //     return true;
    // Load sprite sheet texture
    if (gPeaShooterTexture.loadMedia("images/Plants/Snow-pea-attacking.png"))
    {
        gPeaShooterTexture.makeSprites(gPeaShooterClips, 8, 8, 60);
        widthHeightRate = (double)gPeaShooterTexture.getHeight() / gPeaShooterTexture.getWidth();
        // gPeaShooterTexture.setColor(255, 128, 128);

        isLoaded = true;
    }
    return true;
}

void PeaShooter::drawPlant()
{
    nowFrame++;
    if (nowFrame >= PEASHOOTER_ANIMATION_FRAMES)
    {
        nowFrame -= PEASHOOTER_ANIMATION_FRAMES;
    }
    renderPlantAtFrame(nowFrame);
}

void PeaShooter::renderPlantAtFrame(int frame)
{
    // If plant is not dead
    if (!isDead)
    {
        // Render current frame
        SDL_Rect *currentClip = &gPeaShooterClips[frame];

        // gPeaShooterTexture.setColor(225, 0, 225);
        gPeaShooterTexture.render(x, y, 90, (int)90 * widthHeightRate, currentClip);

        // Check whether plant is attacking
        if (nowFrame == 32)
            isAttack = 1;
    }
}

void PeaShooter::closePlant()
{
    // Set dead flag
    isDead = 0;
    // Free loaded images
    gPeaShooterTexture.free();
}

void PeaShooter::attack(int frame, int x, int y)
{
    if (!isAttack)
        return;
    // frame -= beginAttack;
    if (frame < 0)
        frame += PEASHOOTER_ANIMATION_FRAMES;
    Pea.render(x + frame * 20, y);
}

void PeaShooter::setAttack()
{
    beginAttack = 1;
}

void PeaShooter::drawBullet()
{
    // Make plant attack (Emiss Pea)
    attack(nowFrame - 32, x + 65, y + 15);
}