#ifndef bullets_h
#define bullets_h
#include <string>
#include "LTexture.h"

enum bulletType
{
    APPLE,
    BASKETBALL,
    COCONUT,
    FIRE_PEA,
    HEART,
    MELON,
    CRACKED_MELON,
    PEA,
    PLASMA_PEA,
    PRIMAL_PEA,
    SNOWZ_PEA,
    NUMBER_OF_BULLET
};

struct Bullet
{
public:
    Bullet();
    Bullet(bulletType);
    ~Bullet();
    void render(int x, int y);

private:
    bulletType type;
    LTexture gBulletTexture;
    SDL_Rect gBulletClip;
    int mWidth, mHeight;
};

#endif // bullets_h