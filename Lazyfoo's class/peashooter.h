#ifndef PEASHOOTER_H
#define PEASHOOTER_H

// Using SDL, SDL_image, SDL_ttf, SDL_mixer, standard IO, math, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <string>
#include "LTexture.h"
#include "bullets.h"

const int PEASHOOTER_ANIMATION_FRAMES = 60;
const int INF = 1e9;

extern int backgroundTilesX[9];
extern int backgroundTilesY[5];

class PeaShooter
{
public:
    PeaShooter();
    PeaShooter(int, int);
    ~PeaShooter();
    bool loadPlantMedia();

    bool IsDead();
    void closePlant();
    void attack(int frame, int x, int y);
    void setAttack();

    // Draw functions
    void drawPlant();
    void renderPlantAtFrame(int);
    void drawBullet();

    // When zombies eating
    void blinkPlant();

private:
    SDL_Rect gPeaShooterClips[PEASHOOTER_ANIMATION_FRAMES];
    LTexture gPeaShooterTexture;
    bool isDead = false;
    bool beginAttack = 0;
    bool isAttack = 0;
    int nowFrame = 0;
    double widthHeightRate;
    int xTile, yTile;
    int x, y;

    Bullet Pea = Bullet(PEA);
};

#endif // PEASHOOTER_H