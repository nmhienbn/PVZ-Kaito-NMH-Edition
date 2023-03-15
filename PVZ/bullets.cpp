#include "bullets.h"

const std::string bulletName[] = {
    "Apple.png",
    "Basketball.png",
    "Coconut.png",
    "Fire_Pea.png",
    "Heart.png",
    "Melon.png",
    "Cracked_Melon.png",
    "Pea.png",
    "Plasma_Pea.png",
    "Primal_Pea.png",
    "Snowz_Pea.png"

};

const std::string bulletPath = "images/Bullets/";

bool isLoadedBullet[NUMBER_OF_BULLET];

Bullet::Bullet()
{
}
Bullet::Bullet(bulletType typ)
{
    type = typ;
    std::string path = bulletPath + bulletName[type];
    printf("%s\n", path);
    // Load Bullet texture
    if (gBulletTexture.loadMedia(path.c_str()))
    {
        gBulletTexture.makeSprite(&gBulletClip);
        mHeight = 25;
        mWidth = 1LL * mHeight * gBulletTexture.getWidth() / gBulletTexture.getHeight();
        gBulletTexture.makeSprite(&gBulletClip);
    }
}
Bullet::~Bullet()
{
}
void Bullet::render(int x, int y)
{
    gBulletTexture.render(x, y, mWidth, mHeight, &gBulletClip);
}